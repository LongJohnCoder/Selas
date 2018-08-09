//=================================================================================================================================
// Joe Schutte
//=================================================================================================================================

#include "SurfaceParameters.h"
#include "IntegratorContexts.h"

#include "TextureLib/TextureFiltering.h"
#include "TextureLib/TextureResource.h"
#include "SceneLib/SceneResource.h"
#include "GeometryLib/Ray.h"
#include "GeometryLib/CoordinateSystem.h"
#include "MathLib/FloatFuncs.h"
#include "MathLib/ColorSpace.h"

#include <embree3/rtcore.h>
#include <embree3/rtcore_ray.h>
#include <ptex/Include/Ptexture.h>
#include <ptex/Include/PtexUtils.h>

#define ForceNoMips_ true
#define EnableEWA_ true

#define ReadUvAttribute(scene, uvs, primId, barycoords, attrib)                                                             \
                        SampleTextureFloat(scene, uvs, primId, barycoords, material->scalarAttributeTextureIndices[attrib], \
                                           false, material->scalarAttributeValues[attrib]);

namespace Selas
{
    //=============================================================================================================================
    static float3 SampleTextureNormal(const SceneResource* scene, float2 uvs, uint textureIndex)
    {
        if(textureIndex == InvalidIndex32)
            return float3::ZAxis_;

        TextureResource* textures = scene->textures;
        if(textures[textureIndex].data->format != TextureResourceData::Float3) {
            Assert_(false);
            return float3::ZAxis_;
        }

        float3 sample;
        TextureFiltering::Triangle(textures[textureIndex].data, 0, uvs, sample);

        return 2.0f * sample - float3(1.0f);
    }

    //=============================================================================================================================
    static float SampleTextureOpacity(const SceneResource* scene, float2 uvs, uint textureIndex)
    {
        if(textureIndex == InvalidIndex32) {
            return 1.0f;
        }

        TextureResource* textures = scene->textures;
        if(textures[textureIndex].data->format != TextureResourceData::Float4) {
            return 1.0f;
        }

        float4 sample;
        TextureFiltering::Triangle(textures[textureIndex].data, 0, uvs, sample);

        return sample.w;
    }

    //=============================================================================================================================
    template <typename Type_>
    static Type_ SampleTexture(const SceneResource* scene, float2 uvs, int32 primId, float2 barycoords, uint textureIndex,
                               bool sRGB, Type_ defaultValue)
    {
        if(textureIndex == InvalidIndex32)
            return defaultValue;

        Type_ sample;

        TextureResource* texture = &scene->textures[textureIndex];
        if(texture->ptexFilter) {
            Ptex::PtexTexture* ptex = (Ptex::PtexTexture*)texture->ptex;
            int32 n = ptex->numChannels();
            Assert_(n == (sizeof(Type_) / sizeof(float)));

            Ptex::PtexFilter* filter = (Ptex::PtexFilter*)texture->ptexFilter;
            filter->eval((float*)(&sample), 0, n, primId, barycoords.x, barycoords.y, 0, 0, 0, 0);
        }
        else if(texture->data->mipCount > 0) {
            TextureFiltering::Triangle(texture->data, 0, uvs, sample);
        }
        else {
            return defaultValue;
        }

        if(sRGB) {
            sample = Math::SrgbToLinearPrecise(sample);
        }

        return sample;
    }

    //=============================================================================================================================
    static float SampleTextureFloat(const SceneResource* scene, float2 uvs, int32 primId, float2 barycoords, uint textureIndex,
                                    bool sRGB, float defaultValue)
    {
        if(textureIndex == InvalidIndex32)
            return defaultValue;

        TextureResource* textures = scene->textures;

        if(textures[textureIndex].data->format == TextureResourceData::Float) {
            return SampleTexture(scene, uvs, primId, barycoords, textureIndex, sRGB, defaultValue);
        }
        else if(textures[textureIndex].data->format == TextureResourceData::Float2) {
            return SampleTexture(scene, uvs, primId, barycoords, textureIndex, sRGB, float2(defaultValue, 0.0f)).x;
        }
        else if(textures[textureIndex].data->format == TextureResourceData::Float3) {
            return SampleTexture(scene, uvs, primId, barycoords, textureIndex, sRGB, float3(defaultValue, 0.0f, 0.0f)).x;
        }
        else if(textures[textureIndex].data->format == TextureResourceData::Float4) {
            return SampleTexture(scene, uvs, primId, barycoords, textureIndex, sRGB, float4(defaultValue, 0.0f, 0.0f, 0.0f)).x;
        }

        Assert_(false);
        return 0.0f;
    }

    //=============================================================================================================================
    static float3 SampleTextureFloat3(const SceneResource* scene, float2 uvs, int32 primId, float2 barycoords, uint textureIndex,
                                      bool sRGB, float3 defaultValue)
    {
        if(textureIndex == InvalidIndex32)
            return defaultValue;

        TextureResource* textures = scene->textures;

        if(textures[textureIndex].data->format == TextureResourceData::Float) {
            float val;
            val = SampleTexture(scene, uvs, primId, barycoords, textureIndex, sRGB, 0.0f);
            return float3(val, val, val);
        }
        else if(textures[textureIndex].data->format == TextureResourceData::Float3) {
            return SampleTexture(scene, uvs, primId, barycoords, textureIndex, sRGB, defaultValue);
        }
        else if(textures[textureIndex].data->format == TextureResourceData::Float4) {
            float4 val = SampleTexture(scene, uvs, primId, barycoords, textureIndex, sRGB, float4(defaultValue, 1.0f));
            return val.XYZ();
        }

        Assert_(false);
        return float3(0.0f);
    }

    //=============================================================================================================================
    static float4 SampleTextureFloat4(const SceneResource* scene, float2 uvs, int32 primId, float2 barycoords, uint textureIndex,
                                      bool sRGB, float defaultValue)
    {
        if(textureIndex == InvalidIndex32)
            return float4(defaultValue, defaultValue, defaultValue, defaultValue);

        TextureResource* textures = scene->textures;

        if(textures[textureIndex].data->format == TextureResourceData::Float) {
            float val = SampleTexture(scene, uvs, primId, barycoords, textureIndex, sRGB, defaultValue);
            return float4(val, val, val, 1.0f);
        }
        else if(textures[textureIndex].data->format == TextureResourceData::Float3) {
            float3 value = SampleTexture(scene, uvs, primId, barycoords, textureIndex, sRGB,
                                         float3(defaultValue, defaultValue, defaultValue));
            return float4(value, 1.0f);
        }
        else if(textures[textureIndex].data->format == TextureResourceData::Float4) {
            return SampleTexture(scene, uvs, primId, barycoords, textureIndex, sRGB,
                                 float4(defaultValue, defaultValue, defaultValue, defaultValue));
        }

        Assert_(false);
        return float4(0.0f);
    }

    //=============================================================================================================================
    static const Material* GetSurfaceMaterial(const SceneResource* scene, uint32 geomId)
    {
        Assert_(geomId < scene->materialLookup.Length());
        return scene->materialLookup[geomId];
    }

    //=============================================================================================================================
    bool CalculateSurfaceParams(const GIIntegrationContext* context, const HitParameters* __restrict hit,
                                SurfaceParameters& surface)
    {
        const SceneResource* scene = context->sceneData->scene;
        
        const Material* material = GetSurfaceMaterial(scene, hit->geomId);

        Align_(16) float3 normal;
        rtcInterpolate0((RTCGeometry)scene->rtcGeometries[hit->geomId], hit->primId, hit->baryCoords.x, hit->baryCoords.y,
                        RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 0, &normal.x, 3);
        Align_(16) float4 tangent;
        rtcInterpolate0((RTCGeometry)scene->rtcGeometries[hit->geomId], hit->primId, hit->baryCoords.x, hit->baryCoords.y,
                        RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 1, &tangent.x, 4);

        float3 n = Normalize(normal);
        float3 t = tangent.XYZ();
        float3 b = Cross(normal, t) * tangent.w;

        // -- Calculate tangent space transforms
        float3x3 tangentToWorld = MakeFloat3x3(t, n, b);
        surface.worldToTangent = MatrixTranspose(tangentToWorld);

        surface.position        = hit->position;
        surface.error           = hit->error;
        surface.materialFlags   = material->flags;

        Align_(16) float2 uvs = float2::Zero_;

        if(material->flags & MaterialFlags::eUsesPtex) {

        }
        else {   
            rtcInterpolate0((RTCGeometry)scene->rtcGeometries[hit->geomId], hit->primId, hit->baryCoords.x, hit->baryCoords.y,
                            RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 2, &uvs.x, 2);

        }

        surface.baseColor = SampleTextureFloat3(scene, uvs, hit->primId, hit->baryCoords, material->baseColorTextureIndex,
                                                true, material->baseColor);
        surface.sheen          = ReadUvAttribute(scene, uvs, hit->primId, hit->baryCoords, eSheen);
        surface.sheenTint      = ReadUvAttribute(scene, uvs, hit->primId, hit->baryCoords, eSheenTint);
        surface.clearcoat      = ReadUvAttribute(scene, uvs, hit->primId, hit->baryCoords, eClearcoat);
        surface.clearcoatGloss = ReadUvAttribute(scene, uvs, hit->primId, hit->baryCoords, eClearcoatGloss);
        surface.specTrans      = ReadUvAttribute(scene, uvs, hit->primId, hit->baryCoords, eSpecTrans);
        surface.diffTrans      = ReadUvAttribute(scene, uvs, hit->primId, hit->baryCoords, eDiffuseTrans);
        surface.flatness       = ReadUvAttribute(scene, uvs, hit->primId, hit->baryCoords, eFlatness);
        surface.anisotropic    = ReadUvAttribute(scene, uvs, hit->primId, hit->baryCoords, eAnisotropic);
        surface.specularTint   = ReadUvAttribute(scene, uvs, hit->primId, hit->baryCoords, eSpecularTint);
        surface.roughness      = ReadUvAttribute(scene, uvs, hit->primId, hit->baryCoords, eRoughness);
        surface.metallic       = ReadUvAttribute(scene, uvs, hit->primId, hit->baryCoords, eMetallic);
        surface.ior            = ReadUvAttribute(scene, uvs, hit->primId, hit->baryCoords, eIor);

        surface.shader = material->shader;
        surface.view = hit->incDirection;

        // -- better way to handle this would be for the ray to know what IOR it is within
        surface.relativeIOR = ((material->flags & eTransparent) && Dot(hit->incDirection, n) < 0.0f) 
                            ? surface.ior : 1.0f / surface.ior;

        //float3x3 normalToWorld = MakeFloat3x3(t, -b, n);
        //float3 perturbNormal = SampleTextureNormal(scene, uvs, material->normalTextureIndex);
        surface.perturbedNormal = n;// Normalize(MatrixMultiply(perturbNormal, normalToWorld));

        return true;
    }

    //=============================================================================================================================
    bool CalculatePassesAlphaTest(const SceneResource* scene, uint32 geomId, uint32 primId, float2 baryCoords)
    {
        const Material* material = GetSurfaceMaterial(scene, geomId);
        Assert_(material->flags & MaterialFlags::eAlphaTested);

        Align_(16) float2 uvs;
        rtcInterpolate0((RTCGeometry)scene->rtcGeometries[geomId], primId, baryCoords.x, baryCoords.y,
                        RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, 2, &uvs.x, 2);

        static const float kAlphaTestCutoff = 0.5f;
        return SampleTextureOpacity(scene, uvs, material->baseColorTextureIndex) > kAlphaTestCutoff;
    }

    //=============================================================================================================================
    float CalculateDisplacement(const SceneResource* scene, uint32 geomId, uint32 primId, float2 uvs)
    {
        // JSTODO - Need to fix this
        Assert_(0);

        const Material* material = GetSurfaceMaterial(scene, geomId);
        float displacement = SampleTextureFloat(scene, uvs, primId, uvs,
                                                material->scalarAttributeTextureIndices[eDisplacement], false, 0.0f);
        if(material->flags & eInvertDisplacement) {
            displacement = 1.0f - displacement;
        }

        return material->scalarAttributeValues[eDisplacement] * displacement;
    }

    //=============================================================================================================================
    float3 GeometricTangent(const SurfaceParameters& surface)
    {
        return float3(surface.worldToTangent.r0.x, surface.worldToTangent.r1.x, surface.worldToTangent.r2.x);
    }

    //=============================================================================================================================
    float3 GeometricNormal(const SurfaceParameters& surface)
    {
        return float3(surface.worldToTangent.r0.y, surface.worldToTangent.r1.y, surface.worldToTangent.r2.y);
    }

    //=============================================================================================================================
    float3 GeometricBitangent(const SurfaceParameters& surface)
    {
        return float3(surface.worldToTangent.r0.z, surface.worldToTangent.r1.z, surface.worldToTangent.r2.z);
    }

    //=============================================================================================================================
    float3 OffsetRayOrigin(const SurfaceParameters& surface, float3 direction, float biasScale)
    {
        float directionOffset = Dot(direction, GeometricNormal(surface)) < 0.0f ? -1.0f : 1.0f;
        float3 offset = directionOffset * surface.error * biasScale * GeometricNormal(surface);
        return surface.position + offset;
    }

    //=============================================================================================================================
    float3 OffsetRayOrigin(const SurfaceParameters& surface, float3 direction, float biasScale, float& signedBiasDistance)
    {
        float directionOffset = Dot(direction, GeometricNormal(surface)) < 0.0f ? -1.0f : 1.0f;
        signedBiasDistance = directionOffset * surface.error * biasScale;
        float3 offset = signedBiasDistance * GeometricNormal(surface);
        return surface.position + offset;
    }

    //=============================================================================================================================
    float ContinuationProbability(const SurfaceParameters& surface)
    {
        float3 value = surface.baseColor;
        return Saturate(Max(Max(value.x, value.y), value.z));
    }
}