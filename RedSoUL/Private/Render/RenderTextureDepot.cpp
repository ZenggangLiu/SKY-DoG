/// System headers
#include <cstdio>   /// std::printf, std::snprintf
#include <cstdlib>  /// std::free, std::malloc
#include <stdint.h> /// uint64_t
/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Common/CommonDefines.hpp" /// GENERATE_BUILTIN_NAME
#include "DataType/HdrColor.hpp"
#include "DataType/LdrColor.hpp"
#include "DataType/LdrGrayscale.hpp"
#include "FileSystem/FileName.hpp"
#include "IO/ImageFile/ExrFile.hpp"
#include "IO/ImageFile/PpmFile.hpp"
#include "IO/ImageFile/RTextureFile.hpp"
/// Self header
#include "Render/RenderTextureDepot.hpp"


// MARK: == Helpers ==
static
uint32_t
next_mipmap_dimension (
    const uint32_t mipmap_dim)
{
    return mipmap_dim > 1 ? mipmap_dim >> 1 : 1;
}


static
bool
is_cell_border_texel (
    const uint32_t texel_position,
    const uint32_t texel_count,
    const uint32_t cell_dimension,
    const uint32_t cell_border_width)
{
    if (cell_border_width > 0)
    {
        if (cell_border_width >= cell_dimension ||
            cell_border_width >= texel_count)
        {
            return true;
        }
        else
        {
            /// 计算指定texel在Cell中的位置
            const uint32_t local_pos = texel_position % cell_dimension;
            if (local_pos < cell_border_width)
            {
                return true;
            }
            else
            {
                return texel_position >= texel_count - cell_border_width;
            }
        }
    }
    else
    {
        return false;
    }
}


static
LdrColor
debug_color_from_mipmap_level (
    const uint8_t level)
{
    switch (level) /// mipmap level: [0, n]
    {
        case 0:  return LdrColor::make(255,   0,   0, 255); /// 赤
        case 1:  return LdrColor::make(255, 128,   0, 255); /// 橙
        case 2:  return LdrColor::make(255, 255,   0, 255); /// 黄
        case 3:  return LdrColor::make(  0, 255,   0, 255); /// 绿
        case 4:  return LdrColor::make(  0, 255, 255, 255); /// 青
        case 5:  return LdrColor::make(  0,   0, 255, 255); /// 蓝
        case 6:  return LdrColor::make(128,   0, 255, 255); /// 紫
        default: return LdrColor::make(255, 255, 255, 255); /// 白
    }
}


static
HdrColor
debug_color_from_slice_index (
    const uint8_t index)
{
    /// 使用LDR色彩范围[0, 1], 但保存为HdrColor.
    switch (index)
    {
        case 0:  return HdrColor::make(0.40f, 0.00f, 0.00f, 1.0f); /// +X: 赤
        case 1:  return HdrColor::make(1.00f, 0.58f, 0.00f, 1.0f); /// -X: 橙
        case 2:  return HdrColor::make(1.00f, 1.00f, 0.00f, 1.0f); /// +Y: 黄
        case 3:  return HdrColor::make(0.00f, 0.40f, 0.00f, 1.0f); /// -Y: 绿
        case 4:  return HdrColor::make(0.00f, 0.60f, 1.00f, 1.0f); /// +Z: 青
        default: return HdrColor::make(0.20f, 0.00f, 0.60f, 1.0f); /// -Z: 紫
    }
}


static
void
generate_checker_board (
    const uint32_t   texture_width,
    const uint32_t   texture_height,
    const uint32_t   cell_dimension,
    const uint32_t   cell_border_width,
    const LdrColor   cell_color_0,
    const LdrColor   cell_color_1,
    const LdrColor   cell_border_color,
    LdrColor * const texel_list)
{
    for (uint32_t row_idx = 0; row_idx < texture_height; ++row_idx)
    {
        for (uint32_t coln_idx = 0; coln_idx < texture_width; ++coln_idx)
        {
            const bool is_border =
                is_cell_border_texel(coln_idx, texture_width,
                                     cell_dimension,
                                     cell_border_width) ||
                is_cell_border_texel(row_idx, texture_height,
                                     cell_dimension,
                                     cell_border_width);

            const uint32_t texel_idx = row_idx * texture_width + coln_idx;
            if (is_border)
            {
                texel_list[texel_idx] = cell_border_color;
            }
            else
            {
                const uint32_t cell_x = coln_idx / cell_dimension;
                const uint32_t cell_y = row_idx / cell_dimension;
                const bool use_cell_color_0 = ((cell_x + cell_y) & 1) == 0;
                texel_list[texel_idx] =
                    use_cell_color_0 ? cell_color_0 : cell_color_1;
            }
        }
    }
}


static
void
generate_ldr_debug_mipmap (
    const uint32_t   texture_width,
    const uint32_t   texture_height,
    const uint8_t    mipmap_count,
    LdrColor * const texel_list)
{
    /// Texel数据Layout:
    /// +-----------------------+
    /// |                       |
    /// |         Mip 0         |
    /// |                       |
    /// +-----------+-----------+
    /// |   Mip 1   |
    /// |           |
    /// +-------+---+
    /// | Mip 2 |
    /// +-------+
    ///
    uint32_t mipmap_width  = texture_width;
    uint32_t mipmap_height = texture_height;

    /// 当前Mipmap的Texel列表
    LdrColor * texel_list_mip = texel_list;
    for (uint8_t mip = 0; mip < mipmap_count; ++mip)
    {
        const LdrColor mipmap_color = debug_color_from_mipmap_level(mip);
        /// 当前Mipmap中的Texel个数
        const uint32_t texel_count_mip = mipmap_width * mipmap_height;

        /// 存储当前Mipmap的Texel数据
        for (uint32_t i = 0; i < texel_count_mip; ++i)
        {
            texel_list_mip[i] = mipmap_color;
        }

        /// 更新Mipmap的Texel列表起始地址
        texel_list_mip += texel_count_mip;

        /// 跳到下一级Mipmap
        mipmap_width  = next_mipmap_dimension(mipmap_width);
        mipmap_height = next_mipmap_dimension(mipmap_height);
    }
}


static
void
generate_hdr_debug_cubemap (
    const uint32_t   texture_size,
    HdrColor * const texel_list)
{
    /// Texel数据Layout: +X, -X, +Y, -Y, +Z, -Z
    /// +------Positive X(+X)------+
    /// |-----------------------+  |
    /// |                       |  |
    /// |         Mip 0         |  |
    /// |                       |  |
    /// |-----------+-----------+  |
    /// |   Mip 1   |              |
    /// |           |              |
    /// |-------+---+              |
    /// | Mip 2 |                  |
    /// +--------------------------+
    /// |                          |
    /// +------Negative X(-X)------+
    /// |-----------------------+  |
    /// |                       |  |
    /// |         Mip 0         |  |
    /// |                       |  |
    /// |-----------+-----------+  |
    /// |   Mip 1   |              |
    /// |           |              |
    /// |-------+---+              |
    /// | Mip 2 |                  |
    /// +--------------------------+
    /// |           ~~~            |
    /// +------Negative Z(-Z)------+
    /// |-----------------------+  |
    /// |                       |  |
    /// |         Mip 0         |  |
    /// |                       |  |
    /// |-----------+-----------+  |
    /// |   Mip 1   |              |
    /// |           |              |
    /// |-------+---+              |
    /// | Mip 2 |                  |
    /// +--------------------------+
    ///
    static constexpr uint8_t CUBEMAP_SLICE_COUNT = 6;

    /// Slice中Texel的个数
    const uint32_t texel_count_slice = texture_size * texture_size;

    /// 每个Slice贴图的样式:
    /// +----------+
    /// | +------+ | E
    /// V |      | | d
    /// | |      | | g
    /// | +------+ | e
    /// +----U-----+
    ///
    /// 贴图四周的一圈特殊区域: 32x32以下的贴图的边界Edge为1
    const uint32_t edge_width = texture_size >= 32 ? texture_size >> 5 : 1;

    const HdrColor slice_edge_color = HdrColor::make(8.0f, 8.0f, 8.0f, 1.0f); /// 白色边
    const HdrColor u_axis_color     = HdrColor::make(8.0f, 0.0f, 0.0f, 1.0f); /// 红U轴
    const HdrColor v_axis_color     = HdrColor::make(0.0f, 8.0f, 0.0f, 1.0f); /// 绿V轴

    /// Slice按照: +X, -X, +Y, -Y, +Z, -Z 顺序存储
    for (uint8_t slice_idx = 0; slice_idx < CUBEMAP_SLICE_COUNT; ++slice_idx)
    {
        const HdrColor slice_color = debug_color_from_slice_index(slice_idx);

        /// 当前Slice的Texel列表
        HdrColor * const texel_list_slice = texel_list + slice_idx * texel_count_slice;
        for (uint32_t row_idx = 0; row_idx < texture_size; ++row_idx)
        {
            for (uint32_t coln_idx = 0; coln_idx < texture_size; ++coln_idx)
            {
                const uint32_t texel_idx = row_idx * texture_size + coln_idx;
                const bool is_u_axis  = row_idx  < edge_width;
                const bool is_v_axis  = coln_idx < edge_width;
                /// 是否当前Texel在Edge中
                const bool is_in_edge =
                    is_u_axis || is_v_axis || /// 底侧/左侧
                    coln_idx >= texture_size - edge_width || /// 右侧
                    row_idx  >= texture_size - edge_width;   /// 上侧

                if (is_v_axis)
                {
                    texel_list_slice[texel_idx] = v_axis_color;
                }
                else if (is_u_axis)
                {
                    texel_list_slice[texel_idx] = u_axis_color;
                }
                else if (is_in_edge)
                {
                    texel_list_slice[texel_idx] = slice_edge_color;
                }
                else
                {
                    texel_list_slice[texel_idx] = slice_color;
                }
            }
        }
    }
}



// MARK: == RenderTextureDepot ==
RenderTextureDepot &
RenderTextureDepot::ref ()
{
    static RenderTextureDepot s_instance;
    return s_instance;
}


uint32_t
RenderTextureDepot::slice_texel_count (
    const uint32_t texture_width,
    const uint32_t texture_height,
    const uint8_t  mipmap_count) const
{
    uint32_t texel_count = 0;
    if (mipmap_count > 0)
    {
        uint32_t mipmap_width  = texture_width;
        uint32_t mipmap_height = texture_height;
        for (uint8_t i = 0; i < mipmap_count; ++i)
        {
            texel_count += mipmap_width * mipmap_height;

            /// 遍历下一个Mipmap
            mipmap_width  = next_mipmap_dimension(mipmap_width);
            mipmap_height = next_mipmap_dimension(mipmap_height);
        }
    }
    return texel_count;
}


uint8_t
RenderTextureDepot::total_slice_count (
    const bool is_cube_map) const
{
    return is_cube_map ? 6 : 1;
}


uint32_t
RenderTextureDepot::total_texel_count (
    const uint32_t texture_width,
    const uint32_t texture_height,
    const uint8_t  mipmap_count,
    const bool     is_cube_map) const
{
    const uint32_t texel_count_slice =
        slice_texel_count(texture_width, texture_height, mipmap_count);
    return texel_count_slice * total_slice_count(is_cube_map);
}


const RenderTexture *
RenderTextureDepot::texture_data (
    const RenderTextureIdT texture_id) const
{
    RUNTIME_ASSERT(texture_id != INVALID_RENDER_TEXTURE_ID, "Texture Id is invalid!!");

    ConstRenderTextureIteratorT cached_data = m_texture_table.find(texture_id);
    if (cached_data == m_texture_table.end())
    {
        return nullptr;
    }
    else
    {
        return &cached_data->second;
    }
}


std::tuple<const uint8_t *, uint32_t>
RenderTextureDepot::texel_list (
    const RenderTextureIdT texture_id,
    const uint8_t          slice_idx,
    const uint8_t          mipmap_idx) const
{
    const RenderTexture * const texture = texture_data(texture_id);
    if (texture &&
        slice_idx  < total_slice_count(texture->is_cube_map) &&
        mipmap_idx < texture->mipmap_count)
    {
        /// 计算Slice相对纹理数据头的偏移
        const uint32_t slice_data_offset = slice_idx == 0 
                                         ? 0
                                         : slice_idx * texture->slice_data_size;

        /// 计算Slice中, 指定Mipmap的Texel数据列表的相对偏移
        const uint32_t texel_list_offset =
            mipmap_texel_list_offset_in_bytes(texture, mipmap_idx);
        const uint32_t texel_list_size =
            mipmap_texel_list_size_in_bytes(texture, mipmap_idx);

        /// 计算Texel数据列表的绝对偏移(相对于纹理数据的起始位置)
        const uint8_t * const texel_list_offset_from_start =
            texture->texel_list + slice_data_offset + texel_list_offset;

        return std::make_tuple(texel_list_offset_from_start, texel_list_size);
    }
    else
    {
        return std::make_tuple(nullptr, 0);
    }
}


RenderTextureIdT
RenderTextureDepot::create_checker_board (
    const uint32_t texture_width,
    const uint32_t texture_height,
    const uint32_t cell_dimension,
    const uint32_t cell_border_width,
    const LdrColor color_cell_0,
    const LdrColor color_cell_1,
    const LdrColor color_cell_border)
{
    const uint64_t texel_count = (uint64_t)texture_width * (uint64_t)texture_height;

    RUNTIME_ASSERT(texture_width  != 0, "Zero texture width!!");
    RUNTIME_ASSERT(texture_height != 0, "Zero texture height!!");
    RUNTIME_ASSERT(cell_dimension != 0, "Zero cell dimension!!");
    RUNTIME_ASSERT(texel_count <= (uint64_t)0xFFFFFFFFu / (uint64_t)sizeof(LdrColor),
                   "Too many texels(uint32_t list size, maximal 1 billion texels)!!");

    char buffer[2048];

    /// 计算 Texture Id
    std::snprintf(
        buffer, sizeof(buffer),
        GENERATE_BUILTIN_NAME(
            "CHECKER_BOARD:%ux%u_c:%u:%3u,%3u,%3u,%3u:%3u,%3u,%3u,%3u_b:%u:%3u,%3u,%3u,%3u"),
        texture_width, texture_height,
        cell_dimension,
        color_cell_0.r, color_cell_0.g, color_cell_0.b, color_cell_0.a,
        color_cell_1.r, color_cell_1.g, color_cell_1.b, color_cell_1.a,
        cell_border_width,
        color_cell_border.r, color_cell_border.g,
        color_cell_border.b, color_cell_border.a);
    const RenderTextureIdT texture_id = RUNTIME_RENDER_TEXTURE_HASH(buffer);

    /// 无此Texture
    if (m_texture_table.find(texture_id) == m_texture_table.end())
    {
        /// 创建Texel列表
        const uint32_t   texel_list_size = (uint32_t)(sizeof(LdrColor) * texel_count);
        LdrColor * const texel_list_ptr  = (LdrColor*)std::malloc(texel_list_size);
        RUNTIME_ASSERT(texel_list_ptr, "Can not allocate checker board texel list!!");

        if (texel_list_ptr)
        {
            generate_checker_board(
                texture_width, texture_height,
                cell_dimension, cell_border_width,
                color_cell_0, color_cell_1, color_cell_border,
                texel_list_ptr);

            cache_texture(
                texture_id, texture_width, texture_height,
                1, false, true,
                TextureDataType::LDR_RGBA_TEXTURE,
                texel_list_size, true,
                (const uint8_t*)texel_list_ptr, texel_list_size);
        }
        else
        {
            return INVALID_RENDER_TEXTURE_ID;
        }
    }
    return texture_id;
}


RenderTextureIdT
RenderTextureDepot::create_debug_mipmap ()
{
    /// Texel数据Layout:
    /// +-----------------------+
    /// |                       |
    /// |         Mip 0         |
    /// |                       |
    /// +-----------+-----------+
    /// |   Mip 1   |
    /// |           |
    /// +-------+---+
    /// | Mip 2 |
    /// +-------+
    ///
    /// 纹理大小
    static constexpr uint32_t TEXTURE_DIMENSION  = 256;
    /// Mipmap总数
    static constexpr uint8_t  TOTAL_MIPMAP_COUNT = 7;

    char buffer[512];

    /// 计算 Texture Id
    std::snprintf(
        buffer, sizeof(buffer),
        GENERATE_BUILTIN_NAME("DEBUG_MIPMAP:%ux%u_mip:%u"),
        TEXTURE_DIMENSION, TEXTURE_DIMENSION, TOTAL_MIPMAP_COUNT);
    const RenderTextureIdT texture_id = RUNTIME_RENDER_TEXTURE_HASH(buffer);

    /// 无此Texture
    if (m_texture_table.find(texture_id) == m_texture_table.end())
    {
        const uint32_t texel_count_slice =
            slice_texel_count(TEXTURE_DIMENSION, TEXTURE_DIMENSION, TOTAL_MIPMAP_COUNT);
        /// 创建Texel列表
        const uint32_t slice_data_size  =
            (uint32_t)(sizeof(LdrColor) * texel_count_slice);
        const uint32_t texel_list_size  = slice_data_size;
        LdrColor * const texel_list_ptr = (LdrColor*)std::malloc(texel_list_size);
        RUNTIME_ASSERT(texel_list_ptr, "Can not allocate mipmap debug texel list!!");

        if (texel_list_ptr)
        {
            generate_ldr_debug_mipmap(
                TEXTURE_DIMENSION, TEXTURE_DIMENSION,
                TOTAL_MIPMAP_COUNT, texel_list_ptr);

            cache_texture(
                texture_id, TEXTURE_DIMENSION, TEXTURE_DIMENSION,
                TOTAL_MIPMAP_COUNT, false, true,
                TextureDataType::LDR_RGBA_TEXTURE,
                slice_data_size, true,
                (const uint8_t*)texel_list_ptr, texel_list_size);
        }
        else
        {
            return INVALID_RENDER_TEXTURE_ID;
        }
    }
    return texture_id;
}


RenderTextureIdT
RenderTextureDepot::create_debug_cubemap (
    const uint32_t texture_size)
{
    static constexpr uint8_t TOTAL_MIPMAP_COUNT = 1;

    char buffer[256];

    /// 计算 Texture Id
    std::snprintf(
        buffer, sizeof(buffer),
        GENERATE_BUILTIN_NAME("DEBUG_CUBEMAP:%ux%u_mip:%u"),
        texture_size, texture_size, TOTAL_MIPMAP_COUNT);
    const RenderTextureIdT texture_id = RUNTIME_RENDER_TEXTURE_HASH(buffer);

    /// 无此Texture
    if (m_texture_table.find(texture_id) == m_texture_table.end())
    {
        const uint32_t texel_count_slice =
            slice_texel_count(texture_size, texture_size, TOTAL_MIPMAP_COUNT);
        const uint32_t texel_count_total =
            total_texel_count(texture_size, texture_size, TOTAL_MIPMAP_COUNT, true);
        /// 创建Texel列表
        const uint32_t slice_data_size  =
            (uint32_t)(sizeof(HdrColor) * texel_count_slice);
        const uint32_t texel_list_size  =
            (uint32_t)(sizeof(HdrColor) * texel_count_total);
        HdrColor * const texel_list_ptr = (HdrColor*)std::malloc(texel_list_size);
        RUNTIME_ASSERT(texel_list_ptr, "Can not allocate cubemap debug texel list!!");

        if (texel_list_ptr)
        {
            generate_hdr_debug_cubemap(texture_size, texel_list_ptr);

            cache_texture(
                texture_id, texture_size, texture_size,
                TOTAL_MIPMAP_COUNT, true, true,
                TextureDataType::HALF_RGBA_TEXTURE,
                 slice_data_size, true,
                (const uint8_t*)texel_list_ptr, texel_list_size);
        }
        else
        {
            return INVALID_RENDER_TEXTURE_ID;
        }
    }
    return texture_id;
}


RenderTextureIdT
RenderTextureDepot::create_from_texture_file (
    const char * const     abs_file_name,
    const RenderTextureIdT exp_texture_id)
{
    /// 无此Texture
    if (m_texture_table.find(exp_texture_id) == m_texture_table.end())
    {
        uint32_t texture_width;
        uint32_t texture_height;
        uint8_t  mipmap_count;
        bool     is_cube_map;
        bool     is_linear_rgb;
        uint32_t slice_data_size;
        uint8_t* texel_list;
        uint32_t texel_list_size;
        TextureDataType texture_data_type;
        const bool opcode = RTextureFile::read_from(
            abs_file_name, exp_texture_id,
            texture_width, texture_height, mipmap_count, is_cube_map, is_linear_rgb,
            texture_data_type, slice_data_size, texel_list, texel_list_size);
        if (opcode == false)
        {
            return INVALID_RENDER_TEXTURE_ID;
        }

        cache_texture(
            exp_texture_id, texture_width, texture_height,
            mipmap_count, is_cube_map, is_linear_rgb,
            texture_data_type, slice_data_size, true,
            texel_list, texel_list_size);
    }

    return exp_texture_id;
}


void
RenderTextureDepot::write_to_image_file (
    const char * const     abs_file_name,
    const RenderTextureIdT texture_id) const
{
    static constexpr uint32_t MAX_FILE_NAME_LENGTH = 1024;
    /// Cubemap Texel数据Layout:
    /// +X Mipmaps, -X Mipmaps, ..., +Z Mipmpas, -Z Mipmaps
    static constexpr const char * SLICE_NAME_LIST[] =
    {
        "+X",
        "-X",
        "+Y",
        "-Y",
        "+Z",
        "-Z",
    };

    RUNTIME_ASSERT( abs_file_name, "File name can not be NULL!!");
    RUNTIME_ASSERT(*abs_file_name, "File name can not be empty!!");

    const RenderTexture * const texture = texture_data(texture_id);
    if (texture)
    {
        char output_file_name[MAX_FILE_NAME_LENGTH];

        /// 获取文件的base name
        const int32_t base_name_length =
            FileName::base_name(
                abs_file_name, output_file_name, sizeof(output_file_name));
        RUNTIME_ASSERT(base_name_length > 0, "Can not get base file name!!");
        if (base_name_length > 0)
        {
            /// 遍历每个Slice
            for (uint8_t slice_idx = 0;
                 slice_idx < total_slice_count(texture->is_cube_map); ++slice_idx)
            {
                uint32_t mipmap_width  = texture->texture_width;
                uint32_t mipmap_height = texture->texture_height;

                /// 遍历每个Mip
                for (uint8_t mipmap_idx = 0;
                    mipmap_idx < texture->mipmap_count; ++mipmap_idx)
                {
                    /// 构造文件名称
                    if (texture->is_cube_map)
                    {
                        std::snprintf(
                            output_file_name + base_name_length,
                            sizeof(output_file_name) - base_name_length,
                            "_%s_mip_%u", /// BASENAME_X+_mip_0
                            SLICE_NAME_LIST[slice_idx], mipmap_idx);
                    }
                    else
                    {
                        std::snprintf(
                            output_file_name + base_name_length,
                            sizeof(output_file_name) - base_name_length,
                            "_mip_%u", /// BASENAME_mip_0
                            mipmap_idx);
                    }

                    /// 提取Texel数据列表
                    uint32_t        texel_list_size;  /// 列表字节大小
                    const uint8_t * texel_list_start; /// 列表起始位置
                    std::tie(texel_list_start, texel_list_size) =
                        texel_list(texture_id, slice_idx, mipmap_idx);
                    RUNTIME_ASSERT(texel_list_start, "No texel list!!");
                    RUNTIME_ASSERT(texel_list_size,  "Zero texel list size!!");

                    const uint32_t mipmap_texel_count = mipmap_width * mipmap_height;

                    /// 按照Texel数据类型生成文件
                    switch (texture->texture_data_type)
                    {
                        case TextureDataType::LDR_RGBA_TEXTURE:
                        {
                            PpmFile::write_to(
                                output_file_name, mipmap_width, mipmap_height,
                                (const LdrColor*)texel_list_start,
                                mipmap_texel_count, true, true, true);
                            break;
                        }

                        case TextureDataType::LDR_GRAYSCALE_TEXTURE:
                        {
                            PpmFile::write_to(
                                output_file_name, mipmap_width, mipmap_height,
                                (const LdrGrayscale*)texel_list_start,
                                mipmap_texel_count, true);
                            break;
                        }

                        case TextureDataType::HALF_RGBA_TEXTURE:
                        {
                            ExrFile::write_to(
                                output_file_name, mipmap_width, mipmap_height,
                                (const HdrColor*)texel_list_start, mipmap_texel_count);
                            break;
                        }

                        default:
                        {
                            RUNTIME_ASSERT(false, "Unknown texture data type!!");
                            break;
                        }
                    }

                    /// 遍历下一个Mipmap
                    mipmap_width  = next_mipmap_dimension(mipmap_width);
                    mipmap_height = next_mipmap_dimension(mipmap_height);
                }
            }
        }
    }
}


void
RenderTextureDepot::write_to_texture_file (
    const char * const  abs_file_name,
    const RenderTextureIdT texture_id) const
{
    const RenderTexture * const texture = texture_data(texture_id);
    if (texture)
    {
        RTextureFile::write_to(abs_file_name, *texture);
    }
}


void
RenderTextureDepot::clear ()
{
    for (RenderTextureIteratorT texture = m_texture_table.begin();
         texture != m_texture_table.end(); ++texture)
    {
        if (texture->second.is_dyn_allocated)
        {
            std::free((void*)texture->second.texel_list);
        }
    }
    m_texture_table.clear();
}


RenderTextureDepot::RenderTextureDepot ()
{

}


RenderTextureDepot::~RenderTextureDepot ()
{
    clear();
}


uint8_t
RenderTextureDepot::texel_size_in_bytes (
    const TextureDataType data_type) const
{
    switch (data_type)
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


uint32_t
RenderTextureDepot::mipmap_texel_list_offset_in_bytes (
    const RenderTexture * const texture,
    const uint8_t               mipmap_idx) const
{
    RUNTIME_ASSERT(texture, "No render texture!!");
    RUNTIME_ASSERT(mipmap_idx < texture->mipmap_count,
                   "Mipmap index is out of bound!!");


    uint32_t offset_in_texels = 0;
    uint32_t mipmap_width  = texture->texture_width;
    uint32_t mipmap_height = texture->texture_height;
    for (uint8_t i = 0; i < mipmap_idx; ++i)
    {
        offset_in_texels += mipmap_width * mipmap_height;

        /// 遍历下一个Mipmap
        mipmap_width  = mipmap_width  > 1 ? mipmap_width  >> 1 : 1;
        mipmap_height = mipmap_height > 1 ? mipmap_height >> 1 : 1;
    }
    return offset_in_texels * texel_size_in_bytes(texture->texture_data_type);
}


uint32_t
RenderTextureDepot::mipmap_texel_list_size_in_bytes (
    const RenderTexture * const texture,
    const uint8_t               mipmap_idx) const
{
    RUNTIME_ASSERT(texture, "No render texture!!");
    RUNTIME_ASSERT(mipmap_idx < texture->mipmap_count,
                   "Mipmap index is out of bound!!");

    uint32_t mipmap_width  = texture->texture_width;
    uint32_t mipmap_height = texture->texture_height;
    for (uint8_t i = 0; i < mipmap_idx; ++i)
    {
        /// 遍历下一个Mipmap
        mipmap_width  = mipmap_width  > 1 ? mipmap_width  >> 1 : 1;
        mipmap_height = mipmap_height > 1 ? mipmap_height >> 1 : 1;
    }

    const uint32_t mipmap_texel_count = mipmap_width * mipmap_height;
    return mipmap_texel_count * texel_size_in_bytes(texture->texture_data_type);
}


void
RenderTextureDepot::cache_texture (
    const RenderTextureIdT texture_id,
    const uint32_t         texture_width,
    const uint32_t         texture_height,
    const uint8_t          mipmap_count,
    const bool             is_cube_map,
    const bool             is_linear_rgb,
    const TextureDataType  texture_data_type,
    const uint32_t         slice_data_size,
    const bool             is_dyn_allocated,
    const uint8_t * const  texel_list,
    const uint32_t         texel_list_size)
{
    /// 无此Texture
    if (m_texture_table.find(texture_id) == m_texture_table.end())
    {
        m_texture_table.insert(
        {
            texture_id,
            RenderTexture(
                texture_id, texture_width, texture_height,
                mipmap_count, is_cube_map, is_linear_rgb,
                texture_data_type, slice_data_size, is_dyn_allocated,
                texel_list, texel_list_size)
        });
    }
}
