#include "External/Catch/catch_v2.hpp"
#include "UT_Config.hpp"

#if (RENDER_TEXTURE_FILE_CHECKING == 1)
/// System headers
#include <cstdio>   /// std::printf, std::snprintf
#include <cstring>  /// std::memcmp, std::memcpy
#include <stdint.h> /// uint32_t,...
#include <vector>
/// Library headers
#include "FileSystem/NativeDirectory.hpp"
#include "FileSystem/NativeFile.hpp"
#include "IO/ImageFile/RTextureFile.hpp"
#include "Render/RenderTexture.hpp"
#include "Render/RenderTextureDepot.hpp"
#include "Render/RenderTextureId.hpp"


struct RenderTextureDataCopy
{
    RenderTextureIdT     texture_id;
    uint32_t             texture_width;
    uint32_t             texture_height;
    uint32_t             slice_data_size;
    uint32_t             texel_list_size;
    uint8_t              mipmap_count;
    TextureDataType      texture_data_type;
    bool                 is_cube_map;
    bool                 is_linear_rgb;
    std::vector<uint8_t> texel_list;
};


static
void
copy_render_texture_data (
    const RenderTexture &    texture_data,
    RenderTextureDataCopy &  texture_data_copy)
{
    REQUIRE((texture_data.texel_list));
    REQUIRE((texture_data.texel_list_size > 0));

    texture_data_copy.texture_id        = texture_data.texture_id;
    texture_data_copy.texture_width     = texture_data.texture_width;
    texture_data_copy.texture_height    = texture_data.texture_height;
    texture_data_copy.slice_data_size   = texture_data.slice_data_size;
    texture_data_copy.texel_list_size   = texture_data.texel_list_size;
    texture_data_copy.mipmap_count      = texture_data.mipmap_count;
    texture_data_copy.texture_data_type = texture_data.texture_data_type;
    texture_data_copy.is_cube_map       = texture_data.is_cube_map;
    texture_data_copy.is_linear_rgb     = texture_data.is_linear_rgb;

    texture_data_copy.texel_list.resize(texture_data.texel_list_size);
    std::memcpy(
        texture_data_copy.texel_list.data(),
        texture_data.texel_list,
        texture_data.texel_list_size);
}


static
void
require_same_render_texture_content (
    const RenderTextureDataCopy &  texture_data_0,
    const RenderTextureDataCopy &  texture_data_1)
{
    REQUIRE((texture_data_0.texture_id == texture_data_1.texture_id));
    REQUIRE((texture_data_0.texture_width == texture_data_1.texture_width));
    REQUIRE((texture_data_0.texture_height == texture_data_1.texture_height));
    REQUIRE((texture_data_0.slice_data_size == texture_data_1.slice_data_size));
    REQUIRE((texture_data_0.texel_list_size == texture_data_1.texel_list_size));
    REQUIRE((texture_data_0.mipmap_count == texture_data_1.mipmap_count));
    REQUIRE((texture_data_0.texture_data_type == texture_data_1.texture_data_type));
    REQUIRE((texture_data_0.is_cube_map == texture_data_1.is_cube_map));
    REQUIRE((texture_data_0.is_linear_rgb == texture_data_1.is_linear_rgb));
    REQUIRE((texture_data_0.texel_list.size() == texture_data_1.texel_list.size()));
    REQUIRE((std::memcmp(
        texture_data_0.texel_list.data(),
        texture_data_1.texel_list.data(),
        texture_data_0.texel_list.size()) == 0));
}


static
const char *
image_file_extension (
    const TextureDataType texture_data_type)
{
    switch (texture_data_type)
    {
        case TextureDataType::LDR_RGBA_TEXTURE:
        case TextureDataType::LDR_GRAYSCALE_TEXTURE:
        {
            return "ppm";
        }

        case TextureDataType::HALF_RGBA_TEXTURE:
        {
            return "exr";
        }

        default:
        {
            REQUIRE((false));
            return "";
        }
    }
}


static
void
require_dumped_texture_image_files (
    const char * const             texture_name,
    const RenderTextureDataCopy &  texture_data)
{
    static constexpr const char * SLICE_NAME_LIST[] =
    {
        "+X",
        "-X",
        "+Y",
        "-Y",
        "+Z",
        "-Z",
    };

    const char * const extension = image_file_extension(texture_data.texture_data_type);
    const uint8_t slice_count = texture_data.is_cube_map ? 6 : 1;

    char file_name[1024];
    for (uint8_t slice_idx = 0; slice_idx < slice_count; ++slice_idx)
    {
        for (uint8_t mipmap_idx = 0;
             mipmap_idx < texture_data.mipmap_count; ++mipmap_idx)
        {
            if (texture_data.is_cube_map)
            {
                std::snprintf(
                    file_name, sizeof(file_name),
                    "%s/UNIT_TEST/RTextureFile/%s_%s_mip_%u.%s",
                    NativeDirectory::document_folder(),
                    texture_name, SLICE_NAME_LIST[slice_idx],
                    mipmap_idx, extension);
            }
            else
            {
                std::snprintf(
                    file_name, sizeof(file_name),
                    "%s/UNIT_TEST/RTextureFile/%s_mip_%u.%s",
                    NativeDirectory::document_folder(),
                    texture_name, mipmap_idx, extension);
            }
            REQUIRE((NativeFile::does_file_exist(file_name)));
        }
    }
}


static
void
require_rtexture_file_round_trip (
    const char * const     texture_name,
    const RenderTextureIdT texture_id)
{
    const RenderTexture * const created_texture_data =
        RenderTextureDepot::ref().texture_data(texture_id);
    REQUIRE((created_texture_data));

    /// 制作副本
    RenderTextureDataCopy copied_texture_data_created;
    copy_render_texture_data(
        *created_texture_data, copied_texture_data_created);

    char file_name[1024];
    std::snprintf(file_name, sizeof(file_name),
                  "%s/UNIT_TEST/RTextureFile/%s.rtexture",
                  NativeDirectory::document_folder(), texture_name);

    /// 创建RTexture文件
    RenderTextureDepot::ref().write_to_texture_file(file_name, texture_id);
    REQUIRE((NativeFile::does_file_exist(file_name)));
    std::printf("[Output rtexture]: %s\n", file_name);

    /// 清除Depot
    RenderTextureDepot::ref().clear();

    const RenderTextureIdT loaded_texture_id =
        RenderTextureDepot::ref().create_from_texture_file(
            file_name, copied_texture_data_created.texture_id);
    REQUIRE((copied_texture_data_created.texture_id == loaded_texture_id));

    const RenderTexture * const loaded_texture_data =
        RenderTextureDepot::ref().texture_data(loaded_texture_id);
    REQUIRE((loaded_texture_data));

    /// 制作副本
    RenderTextureDataCopy copied_texture_data_loaded;
    copy_render_texture_data(*loaded_texture_data, copied_texture_data_loaded);
    require_same_render_texture_content(
        copied_texture_data_created, copied_texture_data_loaded);

    /// 将读入的Texture以图像文件形式保存
    std::snprintf(file_name, sizeof(file_name),
                  "%s/UNIT_TEST/RTextureFile/%s.%s",
                  NativeDirectory::document_folder(), texture_name,
                  image_file_extension(
                      copied_texture_data_loaded.texture_data_type));
    RenderTextureDepot::ref().write_to_image_file(file_name, loaded_texture_id);
    require_dumped_texture_image_files(texture_name, copied_texture_data_loaded);
}


/// TEST_CASE( name [, tags ] )
TEST_CASE("Checking RTexture File", "[RTextureFile]")
{

    SECTION("Checking RTextureFile")
    {
        std::printf("--- Checking RTextureFile...\n");

        const RenderTextureIdT checker_board_id =
            RenderTextureDepot::ref().create_checker_board(
                512, 521, 256, 12,
                LdrColor::make( 87, 103, 134, 255),
                LdrColor::make( 87, 103, 134, 255),
                LdrColor::make(255, 255, 255, 255));
        REQUIRE((checker_board_id.is_valid()));
        require_rtexture_file_round_trip("checker_board", checker_board_id);

        const RenderTextureIdT debug_mipmap_id =
            RenderTextureDepot::ref().create_debug_mipmap();
        REQUIRE((debug_mipmap_id.is_valid()));
        require_rtexture_file_round_trip("debug_mipmap", debug_mipmap_id);

        const RenderTextureIdT debug_cubemap_id =
            RenderTextureDepot::ref().create_debug_cubemap(16);
        REQUIRE((debug_cubemap_id.is_valid()));
        require_rtexture_file_round_trip("debug_cubemap", debug_cubemap_id);

        std::printf("--- Checking RTextureFile: OK!\n");
    }

} /// TEST_CASE("Checking RTexture File", "[RTextureFile]")

#endif /// (RENDER_TEXTURE_FILE_CHECKING == 1)
