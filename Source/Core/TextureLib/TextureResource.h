#pragma once

//=================================================================================================================================
// Joe Schutte
//=================================================================================================================================

#include "StringLib/FixedString.h"
#include "MathLib/FloatStructs.h"
#include "SystemLib/Error.h"
#include "SystemLib/BasicTypes.h"

namespace Selas
{
    struct TextureResourceData
    {
        TextureResourceData()
        {
            ptexfilepath.Clear();
            texture = nullptr;
        }

        enum TextureDataType
        {
            // -- Convert to something more like d3d formats?
            Float,
            Float2,
            Float3,
            Float4
        };

        static const uint MaxMipCount = 16;

        FilePathString ptexfilepath;

        uint32 mipCount;
        uint32 dataSize;

        uint32 mipWidths[MaxMipCount];
        uint32 mipHeights[MaxMipCount];
        uint64 mipOffsets[MaxMipCount];

        TextureDataType format;
        uint32 pad;

        uint8* texture;
    };

    struct TextureResource
    {
        static cpointer kDataType;
        static const uint64 kDataVersion;

        TextureResource();

        TextureResourceData* data;
        void* ptex;
        void* ptexFilter;
    };

    Error ReadTextureResource(cpointer filepath, TextureResource* texture);
    Error InitializeTextureResource(TextureResource* texture);
    void ShutdownTextureResource(TextureResource* texture);
    void DebugWriteTextureMips(TextureResource* texture, cpointer folder, cpointer name);
}