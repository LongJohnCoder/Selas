#pragma once

//=================================================================================================================================
// Joe Schutte
//=================================================================================================================================

#include "Shading/IntegratorContexts.h"
#include "SceneLib/EmbreeUtils.h"
#include "StringLib/FixedString.h"
#include "GeometryLib/AxisAlignedBox.h"
#include "GeometryLib/Camera.h"
#include "UtilityLib/MurmurHash.h"
#include "MathLib/FloatStructs.h"
#include "ContainersLib/CArray.h"
#include "SystemLib/Error.h"
#include "SystemLib/BasicTypes.h"

namespace Selas
{
    class TextureCache;
    struct ModelResource;
    struct ImageBasedLightResource;
    struct GeometryUserData;

    enum SceneLightType
    {
        QuadLight
    };

    struct SceneLight
    {
        uint32 type;
        float3 position;
        float3 direction;
        float3 x;
        float3 z;
        float3 radiance;
    };

    struct Instance
    {
        Instance()
        {
            localToWorld = Matrix4x4::Identity();
            worldToLocal = Matrix4x4::Identity();
        }

        uint64 index;
        float4x4 localToWorld;
        float4x4 worldToLocal;
    };

    //=============================================================================================================================
    struct SceneResourceData
    {
        FilePathString name;
        FilePathString iblName;
        float4 backgroundIntensity;
        CArray<FilePathString> sceneNames;
        CArray<FilePathString> modelNames;
        CArray<Instance> sceneInstances;
        CArray<Instance> modelInstances;
        CArray<SceneLight> lights;
        
        CArray<FixedString64> sceneMaterialNames;
        CArray<MaterialResourceData> sceneMaterials;
        CameraSettings camera;
    };

    //=============================================================================================================================
    struct SceneResource
    {
        static cpointer kDataType;
        static const uint64 kDataVersion;

        SceneResourceData* data;

        RTCScene rtcScene;

        AxisAlignedBox aaBox;
        float4 boundingSphere;

        GeometryUserData* geomInstanceData;
        SceneResource** scenes;
        ModelResource** models;
        ImageBasedLightResource* iblResource;

        SceneResource();
        ~SceneResource();
    };

    void Serialize(CSerializer* serializer, SceneResourceData& data);

    Error ReadSceneResource(cpointer filepath, SceneResource* scene);
    Error InitializeSceneResource(SceneResource* scene);
    void InitializeEmbreeScene(SceneResource* scene, TextureCache* cache, RTCDevice rtcDevice);
    void ShutdownSceneResource(SceneResource* scene, TextureCache* textureCache);

    void InitializeSceneCamera(const SceneResource* scene, uint width, uint height, RayCastCameraSettings& camera);

    void GeometryFromRayIds(const SceneResource* scene, const int32 instIds[MaxInstanceLevelCount_], int32 geomId,
                            float4x4& localToWorld, RTCGeometry& rtcGeometry);
}
