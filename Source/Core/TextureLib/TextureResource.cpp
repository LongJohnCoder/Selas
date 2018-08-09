//=================================================================================================================================
// Joe Schutte
//=================================================================================================================================

#include "TextureLib/TextureResource.h"
#include "TextureLib/StbImage.h"
#include "Assets/AssetFileUtils.h"
#include "StringLib/FixedString.h"
#include "StringLib/StringUtil.h"
#include "IoLib/BinarySerializer.h"
#include "IoLib/File.h"
#include "IoLib/Directory.h"
#include "SystemLib/BasicTypes.h"

#include <stdio.h>
#include <ptex/Include/Ptexture.h>
#include <ptex/Include/PtexUtils.h>

namespace Selas
{
    cpointer TextureResource::kDataType = "Textures";
    const uint64 TextureResource::kDataVersion = 1533699457ul;

    //=============================================================================================================================
    TextureResource::TextureResource()
        : data(nullptr)
        , ptex(nullptr)
        , ptexFilter(nullptr)
    {

    }

    //=============================================================================================================================
    Error ReadTextureResource(cpointer textureName, TextureResource* texture)
    {
        FilePathString filepath;
        AssetFileUtils::AssetFilePath(TextureResource::kDataType, TextureResource::kDataVersion, textureName, filepath);

        void* fileData = nullptr;
        uint32 fileSize = 0;
        ReturnError_(File::ReadWholeFile(filepath.Ascii(), &fileData, &fileSize));

        BinaryReader reader;
        SerializerStart(&reader, fileData, fileSize);

        SerializerAttach(&reader, reinterpret_cast<void**>(&texture->data), fileSize);
        SerializerEnd(&reader);

        FixupPointerX64(fileData, texture->data->texture);
        return Success_;
    }

    //=============================================================================================================================
    Error InitializeTextureResource(TextureResource* texture)
    {
        if(StringUtil::Length(texture->data->ptexfilepath.Ascii()) == 0) {
            return Success_;
        }

        Ptex::String errorStr;
        Ptex::PtexTexture* ptex = Ptex::PtexTexture::open(texture->data->ptexfilepath.Ascii(), errorStr);
        if(ptex == nullptr) {
            return Error_("Failed to load ptex file (%s) with error: %s", texture->data->ptexfilepath.Ascii(), errorStr.c_str());
        }

        texture->ptex = (void*)ptex;
        
        Ptex::PtexFilter::Options options(Ptex::PtexFilter::f_box, true);

        Ptex::PtexFilter* filter = Ptex::PtexFilter::getFilter(ptex, options);
        texture->ptexFilter = (void*)filter;

        return Success_;
    }

    //=============================================================================================================================
    void ShutdownTextureResource(TextureResource* texture)
    {
        if(texture->ptex) {
            Ptex::PtexTexture* ptex = (Ptex::PtexTexture*)texture->ptex;
            Ptex::PtexFilter* filter = (Ptex::PtexFilter*)texture->ptexFilter;

            filter->release();
            ptex->release();

            texture->ptex = nullptr;
            texture->ptexFilter = nullptr;
        }

        SafeFreeAligned_(texture->data);
    }

    //=============================================================================================================================
    static void DebugWriteTextureMip(TextureResource* texture, uint level, cpointer filepath)
    {
        uint channels = (uint)texture->data->format + 1;

        uint64 mipOffset = texture->data->mipOffsets[level];
        uint32 mipWidth  = texture->data->mipWidths[level];
        uint32 mipHeight = texture->data->mipHeights[level];
        void*  mip       = &texture->data->texture[mipOffset];
        StbImageWrite(filepath, mipWidth, mipHeight, channels, HDR, (void*)mip);
    }

    //=============================================================================================================================
    void DebugWriteTextureMips(TextureResource* texture, cpointer folder, cpointer name)
    {
        for(uint scan = 0, count = texture->data->mipCount; scan < count; ++scan) {
            FixedString256 path;
            #if IsWindows_
                sprintf_s(path.Ascii(), path.Capacity(), "%s/%s_mip_%llu.hdr", folder, name, scan);
            #else
                sprintf(path.Ascii(), "%s/%s_mip_%llu.hdr", folder, name, scan);
            #endif

            Directory::EnsureDirectoryExists(path.Ascii());

            DebugWriteTextureMip(texture, scan, path.Ascii());
        }
    }
}