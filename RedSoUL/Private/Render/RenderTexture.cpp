/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Render/RenderTextureDepot.hpp"
/// Self header
#include "Render/RenderTexture.hpp"


RenderTexture::RenderTexture (
    const RenderTextureIdT _texture_id,
    const uint32_t         _texture_width,
    const uint32_t         _texture_height,
    const uint8_t          _mipmap_count,
    const bool             _is_cube_map,
    const bool             _is_linear_rgb,
    const TextureDataType  _texture_data_type,
    const uint32_t         _slice_data_size,
    const bool             _is_dyn_allocated,
    const uint8_t * const  _texel_list,
    const uint32_t         _texel_list_size)
:
    texel_list(_texel_list),
    texture_id(_texture_id),
    texture_width(_texture_width),
    texture_height(_texture_height),
    slice_data_size(_slice_data_size),
    texel_list_size(_texel_list_size),
    mipmap_count(_mipmap_count),
    texture_data_type(_texture_data_type),
    is_cube_map(_is_cube_map),
    is_linear_rgb(_is_linear_rgb),
    is_dyn_allocated(_is_dyn_allocated)
{
    RUNTIME_ASSERT(_texture_id.is_valid(), "Texture Id must be valid!!");
    RUNTIME_ASSERT(_texture_width  > 0, "Texture width must be larger than zero!!");
    RUNTIME_ASSERT(_texture_height > 0, "Texture height must be larger than zero!!");
    RUNTIME_ASSERT(_is_cube_map == false || _texture_width == _texture_height,
                   "Cube map texture width and height must be the same!!");
    RUNTIME_ASSERT(_slice_data_size, "Zero slice data size!!");
    RUNTIME_ASSERT(_texel_list, "No texel list!!");
    RUNTIME_ASSERT(_texel_list_size, "Zero texel list size!!");
    RUNTIME_ASSERT(
        _texel_list_size ==
        _slice_data_size * RenderTextureDepot::ref().total_slice_count(_is_cube_map),
        "Texel list size does not match slice data size!!");
}
