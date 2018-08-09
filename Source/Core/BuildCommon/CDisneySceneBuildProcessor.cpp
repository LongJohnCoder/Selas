//=================================================================================================================================
// Joe Schutte
//=================================================================================================================================

#include "BuildCommon/CDisneySceneBuildProcessor.h"

#include "BuildCommon/BuildScene.h"
#include "BuildCommon/BakeScene.h"
#include "BuildCommon/ImportMaterial.h"
#include "BuildCore/BuildContext.h"
#include "SceneLib/SceneResource.h"
#include "UtilityLib/JsonUtilities.h"
#include "UtilityLib/QuickSort.h"
#include "Assets/AssetFileUtils.h"
#include "MathLib/Trigonometric.h"
#include "MathLib/FloatFuncs.h"
#include "SystemLib/MemoryAllocation.h"

namespace Selas
{
    //=============================================================================================================================
    struct SceneFileData
    {
        CArray<FixedString256> elements;
        // Camera
        // IBL?
    };

    //=============================================================================================================================
    struct MeshInstance
    {
        uint32 meshIndex;
        float4x4 transform;
    };

    //=============================================================================================================================
    struct MeshDescription
    {
        FilePathString materialFile;
        CArray<FilePathString> objFiles;
        CArray<MeshInstance> instances;
    };

    //=============================================================================================================================
    struct ElementData
    {
        CArray<MeshDescription*> meshes;

        // -- float4x4 is insufficient but will cover isHibiscus for now.
        CArray<float4x4> instancedCopies;
    };

    //=============================================================================================================================
    static FixedString256 ContentRoot(BuildProcessorContext* context)
    {
        FixedString256 root;
        root.Copy(context->source.name.Ascii());

        char* addr = StringUtil::FindLastChar(root.Ascii(), '~');
        if(addr == nullptr) {
            root.Clear();
        }
        else {
            *(addr + 1) = '\0';
        }
        
        return root;
    }

    //=============================================================================================================================
    static Error ParseSceneFile(BuildProcessorContext* context, SceneFileData& output)
    {
        FilePathString filepath;
        AssetFileUtils::ContentFilePath(context->source.name.Ascii(), filepath);
        ReturnError_(context->AddFileDependency(filepath.Ascii()));

        rapidjson::Document document;
        ReturnError_(Json::OpenJsonDocument(filepath.Ascii(), document));

        if(document.HasMember("elements") == false) {
            return Error_("'elements' member not found in Disney scene '%s'", filepath.Ascii());
        }

        for(const auto& element : document["elements"].GetArray()) {
            FixedString256& str = output.elements.Add();
            str.Copy(element.GetString());
        }

        return Success_;
    }

    //=============================================================================================================================
    static Error ParseInstancePrimitivesFile(BuildProcessorContext* context, const FixedString256& root,
                                             const FilePathString& sourceId, MeshDescription* mesh)
    {
        FilePathString filepath;
        AssetFileUtils::ContentFilePath(sourceId.Ascii(), filepath);
        ReturnError_(context->AddFileDependency(filepath.Ascii()));

        rapidjson::Document document;
        ReturnError_(Json::OpenJsonDocument(filepath.Ascii(), document));

        for(const auto& objFileKV : document.GetObject()) {
            cpointer objFile = objFileKV.name.GetString();

            FilePathString instanceObjFile;
            FixedStringSprintf(instanceObjFile, "%s%s", root.Ascii(), objFile);
            AssetFileUtils::IndependentPathSeperators(instanceObjFile);

            uint32 meshIndex = mesh->objFiles.Add(instanceObjFile);

            const auto& element = objFileKV.value;
            uint count = element.GetObject().MemberCount();
            
            uint32 instanceOffset = mesh->instances.Length();
            mesh->instances.Resize(instanceOffset + (uint32)count);

            uint index = instanceOffset;
            for(const auto& instanceKV : element.GetObject()) {
                cpointer instanceName = instanceKV.name.GetString();

                mesh->instances[index].meshIndex = meshIndex;
                if(Json::ReadMatrix4x4(instanceKV.value, mesh->instances[index].transform) == false) {
                    return Error_("Failed to parse transform from instance '%s' in primfile '%s'", instanceName, filepath.Ascii());
                }

                ++index;
            }
        }

        return Success_;
    }

    //=============================================================================================================================
    static Error ParseInstancePrimitivesSection(BuildProcessorContext* context, const FixedString256& root,
                                                const rapidjson::Value& section, MeshDescription* mesh)
    {
        for(const auto& keyvalue : section.GetObject()) {
            const auto& element = keyvalue.value;

            FixedString32 type;
            if(Json::ReadFixedString(element, "type", type) == false) {
                return Error_("'type' parameter missing from instanced primitives section.");
            }

            if(StringUtil::Equals(type.Ascii(), "archive") == false) {
                // TODO
                continue;
            }

            FilePathString primFile;
            if(Json::ReadFixedString(element, "jsonFile", primFile) == false) {
                return Error_("`jsonFile ` parameter missing from instanced primitives section");
            }

            AssetFileUtils::IndependentPathSeperators(primFile);

            FilePathString sourceId;
            FixedStringSprintf(sourceId, "%s%s", root.Ascii(), primFile.Ascii());

            ReturnError_(ParseInstancePrimitivesFile(context, root, sourceId, mesh));
        }

        return Success_;
    }

    //=============================================================================================================================
    static Error ParseElementFile(BuildProcessorContext* context, const FixedString256& root, const FixedString256& path,
                                  ElementData& elementData)
    {
        FilePathString filepath;
        AssetFileUtils::ContentFilePath(path.Ascii(), filepath);
        ReturnError_(context->AddFileDependency(filepath.Ascii()));

        rapidjson::Document document;
        ReturnError_(Json::OpenJsonDocument(filepath.Ascii(), document));

        MeshDescription* mesh = New_(MeshDescription);

        // -- get the materials json file path
        FixedStringSprintf(mesh->materialFile, "%s%s", root.Ascii(), document["matFile"].GetString());

        // -- Each element file will have one root level obj file so we add that and create a single instance for it with
        // -- the identity transform
        FilePathString baseMesh;
        FixedStringSprintf(baseMesh, "%s%s", root.Ascii(), document["geomObjFile"].GetString());
        AssetFileUtils::IndependentPathSeperators(baseMesh);
        mesh->objFiles.Add(baseMesh);

        MeshInstance rootInstance;
        rootInstance.meshIndex = 0;
        rootInstance.transform = Matrix4x4::Identity();
        mesh->instances.Add(rootInstance);

        // -- read the instanced primitives file.
        ReturnError_(ParseInstancePrimitivesSection(context, root, document["instancedPrimitiveJsonFiles"], mesh));

        // -- Each element file will have a transform for the 'root level' object file...
        float4x4 rootTransform;
        if(Json::ReadMatrix4x4(document["transformMatrix"], rootTransform) == false) {
            return Error_("Failed to read root transform matrix");
        }
        elementData.instancedCopies.Add(rootTransform);

        // -- ... and each element file can contain additional instanced copies or the root level object file
        for(const auto& instancedCopyKV : document["instancedCopies"].GetObject()) {

            float4x4 instancexform;
            if(Json::ReadMatrix4x4(instancedCopyKV.value["transformMatrix"], instancexform) == false) {
                return Error_("Failed to read `transformMatrix` from instancedCopy '%s'", instancedCopyKV.name.GetString());
            }

            elementData.instancedCopies.Add(instancexform);
        }        

        elementData.meshes.Add(mesh);

        return Success_;
    }

    //=============================================================================================================================
    static Error ImportDisneyMaterials(BuildProcessorContext* context, const FilePathString& contentId, cpointer prefix,
                                       ImportedModel* imported)
    {
        FilePathString filepath;
        AssetFileUtils::ContentFilePath(contentId.Ascii(), filepath);
        ReturnError_(context->AddFileDependency(filepath.Ascii()));

        CArray<Hash32> materialHashes;
        CArray<ImportedMaterialData> importedMaterials;
        ReturnError_(ImportDisneyMaterials(filepath.Ascii(), prefix, materialHashes, importedMaterials));

        imported->loadedMaterialHashes.Append(materialHashes);
        imported->loadedMaterials.Append(importedMaterials);

        return Success_;
    }

    //=============================================================================================================================
    static Error AddElementToScene(BuildProcessorContext* context, const ElementData* element, cpointer prefix,
                                  ImportedModel* scene)
    {
        uint32 modelOffset = scene->modelFiles.Length();
        for(uint meshIndex = 0, meshCount = element->meshes.Length(); meshIndex < meshCount; ++meshIndex) {
            const MeshDescription* meshDesc = element->meshes[meshIndex];
            scene->modelFiles.Append(meshDesc->objFiles);

            ReturnError_(ImportDisneyMaterials(context, meshDesc->materialFile, prefix, scene));
        }

        for(uint elemInstIdx = 0, elemInstCount = element->instancedCopies.Length(); elemInstIdx < elemInstCount; ++elemInstIdx) {
            float4x4 elementTransform = element->instancedCopies[elemInstIdx];

            for(uint meshIndex = 0, meshCount = element->meshes.Length(); meshIndex < meshCount; ++meshIndex) {
                const MeshDescription* meshDesc = element->meshes[meshIndex];

                uint instanceCount = meshDesc->instances.Length();
                for(uint instIndex = 0; instIndex < instanceCount; ++instIndex) {

                    ModelInstance inst;
                    inst.transform    = MatrixMultiply(elementTransform, meshDesc->instances[instIndex].transform);
                    inst.meshIdx      = meshDesc->instances[instIndex].meshIndex + modelOffset;
                    inst.materialHash = 0;

                    scene->instances.Add(inst);
                }
            }
        }

        return Success_;
    }

    //=============================================================================================================================
    Error CDisneySceneBuildProcessor::Setup()
    {
        AssetFileUtils::EnsureAssetDirectory<SceneResource>();
        AssetFileUtils::EnsureAssetDirectory(SceneResource::kGeometryDataType, SceneResource::kDataVersion);

        return Success_;
    }

    //=============================================================================================================================
    cpointer CDisneySceneBuildProcessor::Type()
    {
        return "disney";
    }

    //=============================================================================================================================
    uint64 CDisneySceneBuildProcessor::Version()
    {
        return SceneResource::kDataVersion;
    }

    //=============================================================================================================================
    Error CDisneySceneBuildProcessor::Process(BuildProcessorContext* context)
    {
        FixedString256 contentRoot = ContentRoot(context);

        ImportedModel importedModel;

        importedModel.camera.position = float3(0.0f, 0.0f, 5.0f);
        importedModel.camera.lookAt   = float3(0.0f, 0.0f, 0.0f);
        importedModel.camera.up       = float3(0.0f, 1.0f, 0.0f);
        importedModel.camera.fov      = 45.0f * Math::DegreesToRadians_;
        importedModel.camera.znear    = 0.1f;
        importedModel.camera.zfar     = 500.0f;

        SceneFileData sceneFile;
        ReturnError_(ParseSceneFile(context, sceneFile));

        FilePathString contentPrefix;
        contentPrefix.Copy(contentRoot.Ascii());

        for(uint scan = 0, count = sceneFile.elements.Length(); scan < count; ++scan) {
            const FixedString256& elementName = sceneFile.elements[scan];

            ElementData elementData;
            ReturnError_(ParseElementFile(context, contentRoot, elementName, elementData));
            AddElementToScene(context, &elementData, contentPrefix.Ascii(), &importedModel);
        }

        BuiltScene builtScene;
        ReturnError_(BuildScene(context, contentPrefix.Ascii(), &importedModel, &builtScene));
        ReturnError_(BakeScene(context, builtScene));

        for(uint scan = 0, count = importedModel.modelFiles.Length(); scan < count; ++scan) {
            ReturnError_(context->AddProcessDependency("model", importedModel.modelFiles[scan].Ascii()));
        }
        for(uint scan = 0, count = builtScene.textures.Length(); scan < count; ++scan) {
            ReturnError_(context->AddProcessDependency("ptex", builtScene.textures[scan].Ascii()));
        }

        return Success_;
    }
}