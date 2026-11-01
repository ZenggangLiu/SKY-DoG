#include "External/Catch/catch_v2.hpp"
#include "UT_Config.hpp"

#if (EXR_IMAGE_FILE_CHECKING == 1)
/// System headers
#include <cstdio>   /// std::printf, std::snprintf
#include <stdint.h> /// uint32_t,...
#include <vector>
/// Library headers
#include "DataType/HdrColor.hpp"
#include "DataType/LdrColor.hpp"
#include "FileSystem/NativeDirectory.hpp"
#include "FileSystem/NativeFile.hpp"
#include "IO/ImageFile/ExrFile.hpp"
#include "Render/RenderTexture.hpp"
#include "Render/RenderTextureDepot.hpp"


static
void
convert_to_hdr_image_data (
    const RenderTexture &    texture,
    std::vector<HdrColor> &  image_data)
{
    REQUIRE((texture.texture_data_type == TextureDataType::LDR_RGBA_TEXTURE));
    REQUIRE((texture.texel_list));
    REQUIRE((texture.texture_width > 0));
    REQUIRE((texture.texture_height > 0));

    const uint32_t texel_count = texture.texture_width * texture.texture_height;
    const LdrColor * const texel_list = (const LdrColor*)texture.texel_list;
    image_data.resize(texel_count);

    for (uint32_t i = 0; i < texel_count; ++i)
    {
        image_data[i] = HdrColor::make(
            (float)texel_list[i].r / 255.0f,
            (float)texel_list[i].g / 255.0f,
            (float)texel_list[i].b / 255.0f,
            (float)texel_list[i].a / 255.0f);
    }
}


/// TEST_CASE( name [, tags ] )
TEST_CASE("Checking EXR File", "[EXR File]")
{

    SECTION("Checking EXR checker board")
    {
        std::printf("--- Checking EXR File...\n");

        static constexpr uint32_t CHECKER_BOARD_WIDTH        = 512;
        static constexpr uint32_t CHECKER_BOARD_HEIGHT       = 521;
        static constexpr uint32_t CHECKER_BOARD_CELL_SIZE    = 256;
        static constexpr uint32_t CHECKER_BOARD_BORDER_WIDTH = 12;

        const RenderTextureIdT texture_id =
            RenderTextureDepot::ref().create_checker_board(
                CHECKER_BOARD_WIDTH,
                CHECKER_BOARD_HEIGHT,
                CHECKER_BOARD_CELL_SIZE,
                CHECKER_BOARD_BORDER_WIDTH,
                LdrColor::make( 87, 103, 134, 255),
                LdrColor::make( 87, 103, 134, 255),
                LdrColor::make(255, 255, 255, 255));
        REQUIRE((texture_id.is_valid()));

        const RenderTexture * const texture =
            RenderTextureDepot::ref().texture_data(texture_id);
        REQUIRE((texture));

        std::vector<HdrColor> image_data;
        convert_to_hdr_image_data(*texture, image_data);
        REQUIRE((image_data.size() ==
                 CHECKER_BOARD_WIDTH * CHECKER_BOARD_HEIGHT));

        char file_name[1024];
        std::snprintf(
            file_name, sizeof(file_name),
            "%s/UNIT_TEST/ExrFile/checker_board_%ux%u_cell%u_border%u.exr",
            NativeDirectory::document_folder(),
            CHECKER_BOARD_WIDTH,
            CHECKER_BOARD_HEIGHT,
            CHECKER_BOARD_CELL_SIZE,
            CHECKER_BOARD_BORDER_WIDTH);

        REQUIRE((ExrFile::write_to(
                 file_name,
                 CHECKER_BOARD_WIDTH,
                 CHECKER_BOARD_HEIGHT,
                 image_data.data(),
                 (uint32_t)image_data.size())));
        REQUIRE((NativeFile::does_file_exist(file_name)));
        REQUIRE((NativeFile::file_length(file_name) > 0));

        std::printf("[Output Exr]: %s\n", file_name);
        std::printf("--- Checking EXR File: OK!\n");

        RenderTextureDepot::ref().clear();
    }

    SECTION("Checking EXR 32x32 LDR gray image")
    {
        std::printf("--- Checking EXR 32x32 LDR gray image...\n");

        static constexpr uint32_t IMAGE_WIDTH  = 32;
        static constexpr uint32_t IMAGE_HEIGHT = 32;
        static constexpr uint32_t PIXEL_COUNT  = IMAGE_WIDTH * IMAGE_HEIGHT;

        static constexpr float LDR_COLOR_R = 0.5f;
        static constexpr float LDR_COLOR_G = 0.5f;
        static constexpr float LDR_COLOR_B = 0.5f;
        static constexpr float LDR_COLOR_A = 1.0f;

        const HdrColor hdr_color = HdrColor::make(
            LDR_COLOR_R, LDR_COLOR_G, LDR_COLOR_B, LDR_COLOR_A);

        std::vector<HdrColor> image_data;
        image_data.resize(PIXEL_COUNT);
        for (uint32_t i = 0; i < PIXEL_COUNT; ++i)
        {
            image_data[i] = hdr_color;
        }

        char file_name[1024];
        std::snprintf(
            file_name, sizeof(file_name),
            "%s/UNIT_TEST/ExrFile/ldr_gray_32x32.exr",
            NativeDirectory::document_folder());

        REQUIRE((ExrFile::write_to(
                 file_name,
                 IMAGE_WIDTH,
                 IMAGE_HEIGHT,
                 image_data.data(),
                 (uint32_t)image_data.size())));
        REQUIRE((NativeFile::does_file_exist(file_name)));
        REQUIRE((NativeFile::file_length(file_name) > 0));

        std::printf("[Output Exr]: %s\n", file_name);
        std::printf("--- Checking EXR 32x32 LDR gray image: OK!\n");
    }

} /// TEST_CASE("Checking EXR File", "[EXR File]")

#endif /// (EXR_IMAGE_FILE_CHECKING == 1)
