#pragma once

//==============================================================================
// Joe Schutte
//==============================================================================

#include <UtilityLib/Color.h>
#include <UtilityLib/StbImageWrite.h>
#include <MathLib/FloatStructs.h>
#include <SystemLib/BasicTypes.h>

struct RTCSceneTy;
typedef struct RTCSceneTy* RTCScene;

namespace Shooty
{
    struct SceneResourceData;
    struct ImageBasedLightResourceData;

    struct SceneContext
    {
        RTCScene rtcScene;
        SceneResourceData* scene;
        ImageBasedLightResourceData* ibl;
        uint width;
        uint height;
    };  

    void PathTraceImage(const SceneContext& context, float3* imageData);
}