#if defined(__APPLE__)

/// Library headers
#include "Assert/RuntimeAssert.hpp"
/// Self header
#include "Render/Backend/Metal/MetalTextureUtility.hpp"


MTLPixelFormat
MetalTextureUtility::metal_texture_format (
    const DeviceTextureFormat device_format)
{
    switch (device_format)
    {
        case DeviceTextureFormat::R16_UNORM_TEXTURE:
        {
            return MTLPixelFormatR16Unorm;
        }

        case DeviceTextureFormat::RGBA8_UNORM_TEXTURE:
        {
            return MTLPixelFormatRGBA8Unorm;
        }

        case DeviceTextureFormat::RGBA8_UNORM_SRGB_TEXTURE:
        {
            return MTLPixelFormatRGBA8Unorm_sRGB;
        }

        case DeviceTextureFormat::RGBA16_FLOAT_TEXTURE:
        {
            return MTLPixelFormatRGBA16Float;
        }

        default:
        {
            RUNTIME_ASSERT(false, "Unknown DeviceTextureFormat!!");
            return MTLPixelFormatInvalid;
        }
    }
};

#endif // defined(__APPLE__)
