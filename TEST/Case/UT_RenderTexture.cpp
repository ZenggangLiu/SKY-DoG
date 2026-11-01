#include "External/Catch/catch_v2.hpp"
#include "UT_Config.hpp"

#if (RENDER_TEXTURE_CHECKING == 1)
/// System headers
#include <cstdio>   /// std::snprintf
#include <stdint.h> /// uint8_t, uint32_t,...
#include <tuple>    /// std::tie
/// Library headers
#include "DataType/HdrColor.hpp"
#include "DataType/LdrColor.hpp"
#include "FileSystem/NativeDirectory.hpp"
#include "FileSystem/NativeFile.hpp"
#include "Render/RenderTexture.hpp"
#include "Render/RenderTextureDepot.hpp"


static
HdrColor
debug_cubemap_slice_color (
    const uint8_t slice_idx)
{
    /// 使用LDR色彩范围[0, 1], 但保存为HdrColor.
    switch (slice_idx)
    {
        case 0:  return HdrColor::make(0.40f, 0.00f, 0.00f, 1.0f); /// +X: 赤
        case 1:  return HdrColor::make(1.00f, 0.58f, 0.00f, 1.0f); /// -X: 橙
        case 2:  return HdrColor::make(1.00f, 1.00f, 0.00f, 1.0f); /// +Y: 黄
        case 3:  return HdrColor::make(0.00f, 0.40f, 0.00f, 1.0f); /// -Y: 绿
        case 4:  return HdrColor::make(0.00f, 0.60f, 1.00f, 1.0f); /// +Z: 青
        default: return HdrColor::make(0.20f, 0.00f, 0.60f, 1.0f); /// -Z: 紫
    }
}


/// TEST_CASE( name [, tags ] )
TEST_CASE("Checking Render Texture", "[Render Texture]")
{

    SECTION("Checking checker board texture")
    {
        static constexpr uint32_t TEXTURE_WIDTH  = 512;
        static constexpr uint32_t TEXTURE_HEIGHT = 521;
        static constexpr uint32_t CELL_DIMENSION = 256;
        static constexpr uint32_t BORDER_WIDTH   = 12;

        std::printf("--- Checking checker board texture...\n");

        const RenderTextureIdT texture_id =
            RenderTextureDepot::ref().create_checker_board(
                TEXTURE_WIDTH,
                TEXTURE_HEIGHT,
                CELL_DIMENSION,
                BORDER_WIDTH,
                LdrColor::make( 87, 103, 134, 255),
                LdrColor::make( 87, 103, 134, 255),
                LdrColor::make(255, 255, 255, 255));
        REQUIRE((texture_id.is_valid()));

        const RenderTexture * const texture =
            RenderTextureDepot::ref().texture_data(texture_id);
        REQUIRE((texture));
        REQUIRE((texture->texture_id == texture_id));
        REQUIRE((texture->texture_data_type == TextureDataType::LDR_RGBA_TEXTURE));
        REQUIRE((texture->texture_width == TEXTURE_WIDTH));
        REQUIRE((texture->texture_height == TEXTURE_HEIGHT));
        REQUIRE((texture->mipmap_count == 1));
        REQUIRE((texture->is_cube_map == (uint8_t)false));
        REQUIRE((texture->is_linear_rgb == (uint8_t)true));
        REQUIRE((texture->is_dyn_allocated == (uint8_t)true));
        REQUIRE((texture->slice_data_size ==
                 sizeof(LdrColor) * TEXTURE_WIDTH * TEXTURE_HEIGHT));
        REQUIRE((texture->texel_list));
        REQUIRE((texture->texel_list_size ==
                 sizeof(LdrColor) * TEXTURE_WIDTH * TEXTURE_HEIGHT));

        const uint8_t * texel_list_start = nullptr;
        uint32_t        texel_list_size  = 0;
        std::tie(texel_list_start, texel_list_size) =
            RenderTextureDepot::ref().texel_list(texture_id, 0, 0);
        REQUIRE((texel_list_start == texture->texel_list));
        REQUIRE((texel_list_size == texture->slice_data_size));

        char file_name[1024];
        /// 输出Checker Board纹理文件, 并检查其是否存在
        std::snprintf(
            file_name, sizeof(file_name),
            "%s/UNIT_TEST/RenderTexture/checker_board.ppm",
            NativeDirectory::document_folder());
        RenderTextureDepot::ref().write_to_image_file(file_name, texture_id);

        std::snprintf(
            file_name, sizeof(file_name),
            "%s/UNIT_TEST/RenderTexture/checker_board_mip_0.ppm",
            NativeDirectory::document_folder());
        REQUIRE((NativeFile::does_file_exist(file_name)));

        std::printf("--- Checking checker board texture: OK!\n");
    }

    SECTION("Checking debug mipmap texture")
    {
        static constexpr uint32_t TEXTURE_WIDTH  = 256;
        static constexpr uint32_t TEXTURE_HEIGHT = 256;
        static constexpr uint8_t  MIPMAP_COUNT   = 7;

        std::printf("--- Checking debug mipmap texture...\n");

        const RenderTextureIdT texture_id =
            RenderTextureDepot::ref().create_debug_mipmap();
        REQUIRE((texture_id.is_valid()));

        const RenderTexture * const texture =
            RenderTextureDepot::ref().texture_data(texture_id);
        REQUIRE((texture));
        REQUIRE((texture->texture_id == texture_id));
        REQUIRE((texture->texture_data_type == TextureDataType::LDR_RGBA_TEXTURE));
        REQUIRE((texture->texture_width == TEXTURE_WIDTH));
        REQUIRE((texture->texture_height == TEXTURE_HEIGHT));
        REQUIRE((texture->mipmap_count == MIPMAP_COUNT));
        REQUIRE((texture->is_cube_map == (uint8_t)false));
        REQUIRE((texture->is_linear_rgb == (uint8_t)true));
        REQUIRE((texture->is_dyn_allocated == (uint8_t)true));
        REQUIRE((texture->texel_list));
        const uint32_t total_texel_count =
            RenderTextureDepot::ref().total_texel_count(
                TEXTURE_WIDTH, TEXTURE_HEIGHT, MIPMAP_COUNT, false);
        REQUIRE((texture->slice_data_size == sizeof(LdrColor) * total_texel_count));
        REQUIRE((texture->texel_list_size == sizeof(LdrColor) * total_texel_count));

        for (uint8_t mipmap_idx = 0; mipmap_idx < MIPMAP_COUNT; ++mipmap_idx)
        {
            const uint8_t * mipmap_texel_list_start = nullptr;
            uint32_t        mipmap_texel_list_size  = 0;
            std::tie(mipmap_texel_list_start, mipmap_texel_list_size) =
                RenderTextureDepot::ref().texel_list(texture_id, 0, mipmap_idx);
            REQUIRE((mipmap_texel_list_start));
            REQUIRE((mipmap_texel_list_size > 0));
        }

        char file_name[1024];
        std::snprintf(
            file_name, sizeof(file_name),
            "%s/UNIT_TEST/RenderTexture/debug_mipmap.ppm",
            NativeDirectory::document_folder());
        RenderTextureDepot::ref().write_to_image_file(file_name, texture_id);

        for (uint8_t mipmap_idx = 0; mipmap_idx < MIPMAP_COUNT; ++mipmap_idx)
        {
            std::snprintf(
                file_name, sizeof(file_name),
                "%s/UNIT_TEST/RenderTexture/debug_mipmap_mip_%u.ppm",
                NativeDirectory::document_folder(),
                mipmap_idx);
            REQUIRE((NativeFile::does_file_exist(file_name)));
        }

        std::printf("--- Checking debug mipmap textur: OK!\n");
    }

    SECTION("Checking debug cubemap texture")
    {
        static constexpr uint32_t TEXTURE_DIMENSION = 16;
        static constexpr const char * SLICE_NAME_LIST[] =
        {
            "+X",
            "-X",
            "+Y",
            "-Y",
            "+Z",
            "-Z",
        };

        static const HdrColor face_edge_color =
            HdrColor::make(8.0f, 8.0f, 8.0f, 1.0f);
        static const HdrColor u_axis_color =
            HdrColor::make(8.0f, 0.0f, 0.0f, 1.0f);
        static const HdrColor v_axis_color =
            HdrColor::make(0.0f, 8.0f, 0.0f, 1.0f);

        std::printf("--- Checking debug cubemap texture...\n");

        const RenderTextureIdT texture_id =
            RenderTextureDepot::ref().create_debug_cubemap(TEXTURE_DIMENSION);
        REQUIRE((texture_id.is_valid()));

        const RenderTexture * const texture =
            RenderTextureDepot::ref().texture_data(texture_id);
        REQUIRE((texture));
        REQUIRE((texture->texture_id == texture_id));
        REQUIRE((texture->texture_data_type == TextureDataType::HALF_RGBA_TEXTURE));
        REQUIRE((texture->texture_width == TEXTURE_DIMENSION));
        REQUIRE((texture->texture_height == TEXTURE_DIMENSION));
        REQUIRE((texture->mipmap_count == 1));
        REQUIRE((texture->is_cube_map == (uint8_t)true));
        REQUIRE((texture->is_linear_rgb == (uint8_t)true));
        REQUIRE((texture->is_dyn_allocated == (uint8_t)true));
        REQUIRE((texture->texel_list));
        const uint32_t total_texel_count =
            RenderTextureDepot::ref().total_texel_count(
                TEXTURE_DIMENSION, TEXTURE_DIMENSION, 1, true);
        REQUIRE((texture->slice_data_size ==
                 sizeof(HdrColor) * TEXTURE_DIMENSION * TEXTURE_DIMENSION));
        REQUIRE((texture->texel_list_size == sizeof(HdrColor) * total_texel_count));

        for (uint8_t slice_idx = 0; slice_idx < 6; ++slice_idx)
        {
            const uint8_t * slice_texel_list_start = nullptr;
            uint32_t        slice_texel_list_size  = 0;
            std::tie(slice_texel_list_start, slice_texel_list_size) =
                RenderTextureDepot::ref().texel_list(texture_id, slice_idx, 0);
            REQUIRE((slice_texel_list_start ==
                     texture->texel_list + slice_idx * texture->slice_data_size));
            REQUIRE((slice_texel_list_size == texture->slice_data_size));

            const HdrColor * const slice_texel_list =
                (const HdrColor*)slice_texel_list_start;
            REQUIRE((slice_texel_list[0] == v_axis_color));
            REQUIRE((slice_texel_list[1] == u_axis_color));
            REQUIRE((slice_texel_list[TEXTURE_DIMENSION] == v_axis_color));
            REQUIRE((slice_texel_list[(TEXTURE_DIMENSION - 1) *
                                      TEXTURE_DIMENSION + 1] ==
                     face_edge_color));
            REQUIRE((slice_texel_list[TEXTURE_DIMENSION + 1] ==
                     debug_cubemap_slice_color(slice_idx)));
        }

        char file_name[1024];
        std::snprintf(
            file_name, sizeof(file_name),
            "%s/UNIT_TEST/RenderTexture/debug_cubemap.exr",
            NativeDirectory::document_folder());
        RenderTextureDepot::ref().write_to_image_file(file_name, texture_id);

        for (uint8_t face_idx = 0; face_idx < 6; ++face_idx)
        {
            std::snprintf(
                file_name, sizeof(file_name),
                "%s/UNIT_TEST/RenderTexture/debug_cubemap_%s_mip_0.exr",
                NativeDirectory::document_folder(),
                SLICE_NAME_LIST[face_idx]);
            REQUIRE((NativeFile::does_file_exist(file_name)));
        }

        std::printf("--- Checking debug cubemap texture: OK!\n");
    }
}

#endif /// (RENDER_TEXTURE_CHECKING == 1)
