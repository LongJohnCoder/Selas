#pragma once

//==============================================================================
// Joe Schutte
//==============================================================================

#include <MathLib/FloatStructs.h>
#include <MathLib/Trigonometric.h>
#include <SystemLib/BasicTypes.h>

namespace Shooty {

    ForceInline_ float2 operator+(float2 lhs, float2 rhs) {
        float2 result = {lhs.x + rhs.x, lhs.y + rhs.y};
        return result;
    }

    ForceInline_ float3 operator+(float3 lhs, float3 rhs) {
        float3 result = {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
        return result;
    }

    ForceInline_ float4 operator+(float4 lhs, float4 rhs) {
        float4 result = {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w};
        return result;
    }

    ForceInline_ float2 operator+(float2 lhs, float rhs) {
        float2 result = {lhs.x + rhs, lhs.y + rhs};
        return result;
    }

    ForceInline_ float3 operator+(float3 lhs, float rhs) {
        float3 result = {lhs.x + rhs, lhs.y + rhs, lhs.z + rhs};
        return result;
    }

    ForceInline_ float4 operator+(float4 lhs, float rhs) {
        float4 result = {lhs.x + rhs, lhs.y + rhs, lhs.z + rhs, lhs.w + rhs};
        return result;
    }

    ForceInline_ float2 operator-(float2 lhs, float2 rhs) {
        float2 result = {lhs.x - rhs.x, lhs.y - rhs.y};
        return result;
    }

    ForceInline_ float3 operator-(float3 lhs, float3 rhs) {
        float3 result = {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
        return result;
    }

    ForceInline_ float4 operator-(float4 lhs, float4 rhs) {
        float4 result = {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w};
        return result;
    }

    ForceInline_ float2 operator*(float2 lhs, float2 rhs) {
        float2 result = { lhs.x * rhs.x, lhs.y * rhs.y };
        return result;
    }

    ForceInline_ float3 operator*(float3 lhs, float3 rhs) {
        float3 result = {lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z};
        return result;
    }

    ForceInline_ float4 operator*(float4 lhs, float4 rhs) {
        float4 result = { lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w };
        return result;
    }

    ForceInline_ float2 operator*(float2 lhs, float scale) {
        float2 result = {lhs.x * scale, lhs.y * scale};
        return result;
    }

    ForceInline_ float3 operator*(float3 lhs, float scale) {
        float3 result = {lhs.x * scale, lhs.y * scale, lhs.z * scale};
        return result;
    }

    ForceInline_ float4 operator*(float4 lhs, float scale) {
        float4 result = {lhs.x * scale, lhs.y * scale, lhs.z * scale, lhs.w * scale};
        return result;
    }

    ForceInline_ float2 operator*(float scale, float2 lhs) {
        float2 result = {lhs.x * scale, lhs.y * scale};
        return result;
    }

    ForceInline_ float3 operator*(float scale, float3 lhs) {
        float3 result = {lhs.x * scale, lhs.y * scale, lhs.z * scale};
        return result;
    }

    ForceInline_ float4 operator*(float scale, float4 lhs) {
        float4 result = {lhs.x * scale, lhs.y * scale, lhs.z * scale, lhs.w * scale};
        return result;
    }

    ForceInline_ void operator+=(float2& lhs, float2 rhs) {
        lhs.x += rhs.x;
        lhs.y += rhs.y;
    }

    ForceInline_ void operator+=(float3& lhs, float3 rhs) {
        lhs.x += rhs.x;
        lhs.y += rhs.y;
        lhs.z += rhs.z;
    }

    ForceInline_ void operator+=(float4& lhs, float4 rhs) {
        lhs.x += rhs.x;
        lhs.y += rhs.y;
        lhs.z += rhs.z;
        lhs.w += rhs.w;
    }

    ForceInline_ float2 operator-(float2 lhs)
    {
        float2 result = { -lhs.x, -lhs.y };
        return result;
    }

    ForceInline_ float3 operator-(float3 lhs)
    {
        float3 result = { -lhs.x, -lhs.y, -lhs.z};
        return result;
    }

    ForceInline_ float4 operator-(float4 lhs)
    {
        float4 result = { -lhs.x, -lhs.y, -lhs.z, -lhs.w };
        return result;
    }

    ForceInline_ float3 Cross(float3 lhs, float3 rhs) {
        float3 result = {
          lhs.y * rhs.z - lhs.z * rhs.y,
          lhs.z * rhs.x - lhs.x * rhs.z,
          lhs.x * rhs.y - lhs.y * rhs.x,
        };
        return result;
    }

    ForceInline_ float Dot(float2 lhs, float2 rhs) {
        return (lhs.x * rhs.x) + (lhs.y * rhs.y);
    }

    ForceInline_ float Dot(float3 lhs, float3 rhs) {
        return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
    }

    ForceInline_ float Dot(float4 lhs, float4 rhs) {
        return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z) + (lhs.w * rhs.w);
    }

    ForceInline_ float LengthSquared(float2 vec)
    {
        return Dot(vec, vec);
    }

    ForceInline_ float LengthSquared(float3 vec) {
        return Dot(vec, vec);
    }

    ForceInline_ float LengthSquared(float4 vec) {
        return Dot(vec, vec);
    }

    ForceInline_ float Length(float2 vec) {
        return Math::Sqrtf(Dot(vec, vec));
    }

    ForceInline_ float Length(float3 vec) {
        return Math::Sqrtf(Dot(vec, vec));
    }

    ForceInline_ float Length(float4 vec) {
        return Math::Sqrtf(Dot(vec, vec));
    }

    ForceInline_ float LengthInverse(float3 vec3) {
        return 1.f / Length(vec3);
    }

    ForceInline_ float LengthInverse(float4 vec4) {
        return 1.f / Length(vec4);
    }

    // -- both should face outward
    ForceInline_ float3 Reflect(float3 n, float3 l)
    {
        return 2.0f * Dot(n, l) * n - l;
    }

    ForceInline_ float Lerp(float a, float b, float t)
    {
        return (1.0f - t) * a + t * b;
    }

    ForceInline_ float2 Lerp(float2 a, float2 b, float t)
    {
        return (1.0f - t) * a + t * b;
    }

    ForceInline_ float3 Lerp(float3 a, float3 b, float t)
    {
        return (1.0f - t) * a + t * b;
    }

    ForceInline_ float4 Lerp(float4 a, float4 b, float t)
    {
        return (1.0f - t) * a + t * b;
    }

    ForceInline_ float Saturate(float x) {
        if (x < 0.0f) {
            return 0.0f;
        }
        else if (x > 1.0f) {
            return 1.0f;
        }

        return x;
    }

    ForceInline_ float3 Normalize(float3 vec3) {
        float invLength = LengthInverse(vec3);

        float3 result = {vec3.x * invLength, vec3.y * invLength, vec3.z * invLength};
        return result;
    }

    ForceInline_ float4 Normalize(float4 vec4) {
        float invLength = LengthInverse(vec4);

        float4 result = {vec4.x * invLength, vec4.y * invLength, vec4.z * invLength, vec4.w * invLength};
        return result;
    }

    namespace Matrix4x4 {
        float4x4 Identity(void);
        float4x4 Zero(void);
        float4x4 Translate(float x, float y, float z);
        float4x4 ScaleTranslate(float s, float tx, float ty, float tz);
        float4x4 ScaleTranslate(float sx, float sy, float sz, float tx, float ty, float tz);
    };

    namespace Matrix3x3 {
        float3x3 Identity(void);
    };

    namespace Matrix2x2
    {
        bool SolveLinearSystem(float2x2 A, float2 B, float2& r);
    }

    // 4x4 matrix functions
    float4x4 MatrixTranspose(float4x4 const& mat);
    float4x4 MatrixInverse(float4x4 const& mat);
    float4x4 MatrixMultiply(float4x4 const& lhs, float4x4 const& rhs);
    float3   MatrixMultiplyFloat3h(float3 const& vec, float4x4 const& mat);
    float4   MatrixMultiplyFloat4(float4 const& vec, float4x4 const& mat);

    float4x4 ScreenProjection(uint width, uint height);
    float4x4 ScreenProjection(float x, float y, uint width, uint height);
    float4x4 PerspectiveFovLhProjection(float fov, float aspect, float near, float far);
    float4x4 OffsetCenterProjectionLh(float min_x, float max_x, float min_y, float max_y, float min_z, float max_z);
    float4x4 LookAtLh(float3 eye, float3 up, float3 target);
    float4x4 ViewLh(float3 position, float3 forward, float3 up, float3 right);
}