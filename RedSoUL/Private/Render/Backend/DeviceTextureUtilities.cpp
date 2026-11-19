/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Render/TextureUtilities.hpp"
/// Self header
#include "Render/Backend/DeviceTextureUtilities.hpp"


TextureDataType
DeviceTextureUtility::to_host_texture_format (
    const DeviceTextureFormat device_format)
{
    switch (device_format)
    {
        case DeviceTextureFormat::R16_UNORM_TEXTURE:
        {
            return TextureDataType::LDR_GRAYSCALE_TEXTURE;
            break;
        }

        case DeviceTextureFormat::RGBA8_UNORM_TEXTURE:
        case DeviceTextureFormat::RGBA8_UNORM_SRGB_TEXTURE:
        {
            return TextureDataType::LDR_RGBA_TEXTURE;
            break;
        }

        case DeviceTextureFormat::RGBA16_FLOAT_TEXTURE:
        {
            return TextureDataType::HALF_RGBA_TEXTURE;
            break;
        }

        default:
        {
            RUNTIME_ASSERT(false, "Unknown DeviceTextureFormat!!");
            return static_cast<TextureDataType>((uint8_t)(-1));
            break;
        }
    }
}


DeviceTextureFormat
DeviceTextureUtility::to_device_texture_format (
    const TextureDataType host_format,
    const bool            is_linear_rgb)
{
    switch (host_format)
    {
        case TextureDataType::LDR_RGBA_TEXTURE:
        {
            return is_linear_rgb
                   ? DeviceTextureFormat::RGBA8_UNORM_TEXTURE
                   : DeviceTextureFormat::RGBA8_UNORM_SRGB_TEXTURE;
            break;
        }

        case TextureDataType::LDR_GRAYSCALE_TEXTURE:
        {
            return DeviceTextureFormat::R16_UNORM_TEXTURE;
            break;
        }

        case TextureDataType::HALF_RGBA_TEXTURE:
        {
            return DeviceTextureFormat::RGBA16_FLOAT_TEXTURE;
            break;
        }

        default:
        {
            RUNTIME_ASSERT(false, "Unknown TextureDataType!!");
            return static_cast<DeviceTextureFormat>((uint8_t)(-1));
        }
    }
}


uint8_t
DeviceTextureUtility::texel_size_in_bytes (
    const DeviceTextureFormat device_format)
{
    const TextureDataType host_format = to_host_texture_format(device_format);
    return TextureUtility::texel_size_in_bytes(host_format);
}
