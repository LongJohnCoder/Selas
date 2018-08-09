//=================================================================================================================================
// Joe Schutte
//=================================================================================================================================

#include "BuildCommon/CPtexBuildProcessor.h"
#include "BuildCommon/BakeTexture.h"
#include "BuildCore/BuildContext.h"
#include "TextureLib/TextureResource.h"
#include "Assets/AssetFileUtils.h"
#include "SystemLib/MemoryAllocation.h"

namespace Selas
{
    //=============================================================================================================================
    Error CPtexBuildProcessor::Setup()
    {
        AssetFileUtils::EnsureAssetDirectory<TextureResource>();

        return Success_;
    }

    //=============================================================================================================================
    cpointer CPtexBuildProcessor::Type()
    {
        return "ptex";
    }

    //=============================================================================================================================
    uint64 CPtexBuildProcessor::Version()
    {
        return TextureResource::kDataVersion;
    }

    //=============================================================================================================================
    Error CPtexBuildProcessor::Process(BuildProcessorContext* context)
    {
        TextureResourceData textureData;
        Memory::Zero(&textureData, sizeof(textureData));

        AssetFileUtils::ContentFilePath(context->source.name.Ascii(), textureData.ptexfilepath);

        //ReturnError_(ImportTexture(context, Box, &textureData));
        ReturnError_(BakeTexture(context, &textureData));

        //Free_(textureData.texture);

        return Success_;
    }
}