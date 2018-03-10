//==============================================================================
// Joe Schutte
//==============================================================================

#include <BuildCommon/BuildScene.h>
#include <BuildCommon/SceneBuildPipeline.h>
#include <BuildCommon/BuildMaterial.h>
#include <SystemLib/CheckedCast.h>
#include <SystemLib/MinMax.h>
#include <SystemLib/MemoryAllocation.h>
#include <MathLib/FloatFuncs.h>
#include <UtilityLib/Color.h>

namespace Shooty
{
    //==============================================================================
    static void AppendAndOffsetIndices(const CArray<uint32>& addend, uint32 offset, CArray <uint32>& indices)
    {
        uint addendCount = addend.Length();
        uint32 newTotal = (uint32)(indices.Length() + addendCount);

        indices.Reserve(newTotal);

        for(uint scan = 0; scan < addendCount; ++scan) {
            indices.Add(addend[scan] + offset);
        }
    }

    //==============================================================================
    static void BuildMeshes(ImportedScene* imported, BuiltScene* built)
    {
        uint32 totalIndexCount = 0;
        uint32 totalVertexCount = 0;

        for(uint scan = 0, count = imported->meshes.Length(); scan < count; ++scan) {

            ImportedMesh* mesh = imported->meshes[scan];

            BuiltMeshData meshData;
            meshData.indexCount = mesh->indices.Length();
            meshData.vertexCount = mesh->positions.Length();
            meshData.indexOffset = totalIndexCount;
            meshData.vertexOffset = totalVertexCount;

            built->meshes.Add(meshData);
            AppendAndOffsetIndices(mesh->indices, totalVertexCount, built->indices);
            built->positions.Append(mesh->positions);

            built->vertexData.Reserve(built->vertexData.Length() + meshData.vertexCount);
            for(uint i = 0; i < meshData.vertexCount; ++i) {
                VertexAuxiliaryData vertexData;
                vertexData.px            = mesh->positions[i].x;
                vertexData.py            = mesh->positions[i].y;
                vertexData.pz            = mesh->positions[i].z;
                vertexData.nx            = mesh->normals[i].x;
                vertexData.ny            = mesh->normals[i].y;
                vertexData.nz            = mesh->normals[i].z;
                vertexData.u             = mesh->uv0[i].x;
                vertexData.v             = mesh->uv0[i].y;
                vertexData.materialIndex = mesh->materialIndex;

                built->vertexData.Add(vertexData);
            }

            totalIndexCount += meshData.indexCount;
            totalVertexCount += meshData.vertexCount;
        }
    }

    //==============================================================================
    static bool BuildMaterials(ImportedScene* imported, BuiltScene* built)
    {
        built->materialData.Resize(imported->materials.Length());
        for(uint scan = 0, count = imported->materials.Length(); scan < count; ++scan) {
            ReturnFailure_(ImportMaterial(imported->materials[scan].Ascii(), &built->materialData[scan]));
        }

        return true;
    }

    //==============================================================================
    bool BuildScene(ImportedScene* imported, BuiltScene* built)
    {

        ReturnFailure_(BuildMaterials(imported, built));

        BuildMeshes(imported, built);

        built->camera = imported->camera;

        return true;
    }
}