/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "DataType/HdrColor.hpp"
#include "DataType/LdrColor.hpp"
#include "DataType/LdrGrayscale.hpp"
/// Self header
#include "Render/TextureUtilities.hpp"


uint32_t
TextureUtility::next_mipmap_dimension (
    const uint32_t mipmap_dim)
{
    return mipmap_dim > 1 ? mipmap_dim >> 1 : 1;
}


uint8_t
TextureUtility::maximal_mipmap_count (
    uint32_t width,
    uint32_t height)
{
    uint8_t mipmap_count = 1;
    while (width > 1 || height > 1)
    {
        ++mipmap_count;

        /// 获得下一个Mipmap的尺寸
        width  = next_mipmap_dimension(width);
        height = next_mipmap_dimension(height);
    }

    return mipmap_count;
}


uint8_t
TextureUtility::texel_size_in_bytes (
    const TextureDataType host_format)
{
    switch (host_format)
    {
        case TextureDataType::LDR_RGBA_TEXTURE:
        {
            /// LdrColor[]
            return sizeof(LdrColor);
        }

        case TextureDataType::LDR_GRAYSCALE_TEXTURE:
        {
            /// LdrGrayscale[]
            return sizeof(LdrGrayscale);
        }

        case TextureDataType::HALF_RGBA_TEXTURE:
        {
            /// HdrColor[]
            return sizeof(HdrColor);
        }

        default:
        {
            RUNTIME_ASSERT(false, "Unknown TextureDataType!!");
            return 0;
        }
    }
}
