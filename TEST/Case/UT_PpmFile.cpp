#include "External/Catch/catch_v2.hpp"
#include "UT_Config.hpp"

#if (PPM_IMAGE_FILE_CHECKING == 1)
/// System headers
#include <cstdlib>  /// std::strtoul
#include <cstring>  /// std::strstr, std::strlen
#include <stdint.h> /// uint32_t,...
#include <vector>   /// std::vector
/// Library headers
#include "DataType/LdrColor.hpp"
#include "DataType/LdrGrayscale.hpp"
#include "FileSystem/NativeDirectory.hpp"
#include "FileSystem/NativeFile.hpp"
#include "FileSystem/NativeReadStream.hpp"
#include "IO/ImageFile/PpmFile.hpp"


static
void
require_ppm16_binary_file_data (
    const char * const         file_name,
    const uint32_t             image_width,
    const uint32_t             image_height,
    const LdrGrayscale * const image_data)
{
    const uint32_t exp_pixel_data_size =
        image_width * image_height * 3 * (uint32_t)sizeof(uint16_t);

    REQUIRE((NativeFile::does_file_exist(file_name)));

    NativeReadStream ppm_file(file_name);
    REQUIRE((ppm_file.is_opened()));

    const uint32_t file_size = ppm_file.file_length();
    REQUIRE((file_size > exp_pixel_data_size));

    /// PPM二进制数据保存在文件尾部. 前方Header长度可变, 因此读取尾部数据
    /// 来验证真实Pixel数据.
    const uint32_t header_size = file_size - exp_pixel_data_size;
    REQUIRE((header_size < 2048));

    char header[2048];
    REQUIRE((ppm_file.read(
        (uint8_t*)header, sizeof(header), 0, header_size) ==
        (int32_t)header_size));
    header[header_size] = 0;

    REQUIRE((std::strstr(header, "P6\n") == header));
    REQUIRE((std::strstr(header, "48bits RGB(RGB16)") != nullptr));
    REQUIRE((std::strstr(header, "\n65535\n") != nullptr));

    REQUIRE((ppm_file.seek(-(int32_t)exp_pixel_data_size, FILE_END_SEEK_MODE)));

    std::vector<uint8_t> pixel_data(exp_pixel_data_size);
    REQUIRE((ppm_file.read(
        pixel_data.data(), (uint32_t)pixel_data.size(), 0, exp_pixel_data_size) ==
        (int32_t)exp_pixel_data_size));

    uint32_t write_idx = 0;
    for (uint32_t row = image_height; row > 0; --row)
    {
        const uint32_t row_idx = row - 1;
        for (uint32_t coln = 0; coln < image_width; ++coln)
        {
            const uint16_t value = image_data[row_idx * image_width + coln].s;
            const uint8_t high  = (uint8_t)(value >> 8);
            const uint8_t low   = (uint8_t)(value & 0xFF);

            for (uint8_t channel = 0; channel < 3; ++channel)
            {
                REQUIRE((pixel_data[write_idx++] == high));
                REQUIRE((pixel_data[write_idx++] == low));
            }
        }
    }
    REQUIRE((write_idx == exp_pixel_data_size));

    ppm_file.close();
}


static
void
require_ppm16_ascii_file_data (
    const char * const         file_name,
    const uint32_t             image_width,
    const uint32_t             image_height,
    const LdrGrayscale * const image_data)
{
    REQUIRE((NativeFile::does_file_exist(file_name)));

    NativeReadStream ppm_file(file_name);
    REQUIRE((ppm_file.is_opened()));

    const uint32_t file_size = ppm_file.file_length();
    REQUIRE((file_size > 0));

    std::vector<uint8_t> file_data(file_size + 1);
    REQUIRE((ppm_file.read(
        file_data.data(), (uint32_t)file_data.size(), 0, file_size) ==
        (int32_t)file_size));
    file_data[file_size] = 0;

    const char * const file_text = (const char*)file_data.data();
    REQUIRE((std::strstr(file_text, "P3\n") == file_text));
    REQUIRE((std::strstr(file_text, "48bits RGB(RGB16)") != nullptr));
    REQUIRE((std::strstr(file_text, "\n65535\n") != nullptr));

    const char * const image_data_marker = "# --- image data ---\n";
    const char * cursor = std::strstr(file_text, image_data_marker);
    REQUIRE((cursor != nullptr));
    cursor += std::strlen(image_data_marker);

    for (uint32_t row = image_height; row > 0; --row)
    {
        const uint32_t row_idx = row - 1;
        for (uint32_t coln = 0; coln < image_width; ++coln)
        {
            const uint16_t value = image_data[row_idx * image_width + coln].s;

            for (uint8_t channel = 0; channel < 3; ++channel)
            {
                char * end = nullptr;
                const uint32_t output_value =
                    (uint32_t)std::strtoul(cursor, &end, 10);
                REQUIRE((end != cursor));
                REQUIRE((output_value == value));
                cursor = end;
            }
        }
    }

    ppm_file.close();
}


static
uint8_t
lerp_channel_u8 (
    const uint8_t  a,
    const uint8_t  b,
    const uint32_t t_255)
{
    return (uint8_t)(((uint32_t)a * (255u - t_255) +
                      (uint32_t)b * t_255 + 127u) / 255u);
}


static
LdrColor
hue_color_from_picker_x (
    const uint32_t coln,
    const uint32_t image_width)
{
    /// 横向按照Paint Color Picker中常见的Hue色环展开:
    /// Red -> Yellow -> Green -> Cyan -> Blue -> Magenta -> Red
    const uint32_t hue_pos = coln * (6u * 255u) / (image_width - 1u);
    const uint32_t segment = hue_pos / 255u;
    const uint32_t phase   = hue_pos % 255u;

    switch (segment)
    {
        case 0:  return LdrColor::make(255, (uint8_t)phase, 0, 255);
        case 1:  return LdrColor::make((uint8_t)(255u - phase), 255, 0, 255);
        case 2:  return LdrColor::make(0, 255, (uint8_t)phase, 255);
        case 3:  return LdrColor::make(0, (uint8_t)(255u - phase), 255, 255);
        case 4:  return LdrColor::make((uint8_t)phase, 0, 255, 255);
        default: return LdrColor::make(255, 0, (uint8_t)(255u - phase), 255);
    }
}


static
LdrColor
paint_color_picker_color (
    const uint32_t coln,
    const uint32_t row,
    const uint32_t image_width,
    const uint32_t image_height)
{
    /// 纵向:
    /// - 顶部: 白色
    /// - 中部: 当前Hue的高饱和色彩
    /// - 底部: 黑色
    const LdrColor hue_color =
        hue_color_from_picker_x(coln, image_width);
    const uint32_t y_from_top =
        (image_height - 1u - row) * 255u / (image_height - 1u);

    if (y_from_top <= 127u)
    {
        const uint32_t t = y_from_top * 255u / 127u;
        return LdrColor::make(
            lerp_channel_u8(255, hue_color.r, t),
            lerp_channel_u8(255, hue_color.g, t),
            lerp_channel_u8(255, hue_color.b, t),
            255);
    }
    else
    {
        const uint32_t t = (y_from_top - 128u) * 255u / 127u;
        return LdrColor::make(
            lerp_channel_u8(hue_color.r, 0, t),
            lerp_channel_u8(hue_color.g, 0, t),
            lerp_channel_u8(hue_color.b, 0, t),
            255);
    }
}


/// TEST_CASE( name [, tags ] )
TEST_CASE("Checking PPM File", "[PPM File]")
{

    SECTION("Checking PPM File")
    {
        std::printf("--- Checking PPM File...\n");

        static constexpr uint8_t IMAGE_WIDTH  = 128;
        static constexpr uint8_t IMAGE_HEIGHT = 128;
        const uint32_t IMAGE_PIXEL_COUNT = IMAGE_WIDTH * IMAGE_HEIGHT;

        LdrColor image_data_bgra8[IMAGE_PIXEL_COUNT];
        LdrColor image_data_rgba8[IMAGE_PIXEL_COUNT];
        for (uint32_t row = 0; row < IMAGE_HEIGHT; ++row)
        {
            for (uint32_t coln = 0; coln < IMAGE_WIDTH; ++coln)
            {
                const uint32_t pixel_idx = row * IMAGE_WIDTH + coln;

                const LdrColor color =
                    paint_color_picker_color(
                        coln, row, IMAGE_WIDTH, IMAGE_HEIGHT);

                image_data_rgba8[pixel_idx] =
                    color;
                image_data_bgra8[pixel_idx] =
                    LdrColor::make(color.b, color.g, color.r, color.a);
            }
        }

        char file_name[1024];

        /// ### --- BGR8 --- ###
        /// BGR8 ASCII格式
        {
            std::snprintf(file_name, sizeof(file_name),
                          "%s/UNIT_TEST/PpmFile/BGR8_image_data_ASCII_file",
                          NativeDirectory::document_folder());

            REQUIRE((PpmFile::write_to(
                     file_name,
                     IMAGE_WIDTH, IMAGE_HEIGHT,
                     image_data_bgra8, IMAGE_PIXEL_COUNT,
                     false,    /// is RGBA
                     true,     /// use Alpha
                     false))); /// use ASCII file

            std::snprintf(file_name, sizeof(file_name),
                          "%s/UNIT_TEST/PpmFile/BGR8_image_data_ASCII_file.ppm",
                          NativeDirectory::document_folder());
            std::printf("[Output Ppm]: %s\n", file_name);
        }

        /// BGR8 BINARY格式
        {
            std::snprintf(file_name, sizeof(file_name),
                          "%s/UNIT_TEST/PpmFile/BGR8_image_data_BINARY_file.ppm",
                          NativeDirectory::document_folder());

            REQUIRE((PpmFile::write_to(
                     file_name,
                     IMAGE_WIDTH, IMAGE_HEIGHT,
                     image_data_bgra8, IMAGE_PIXEL_COUNT,
                     false,   /// is RGBA
                     true,    /// use Alpha
                     true))); /// use binary file

            std::printf("[Output Ppm]: %s\n", file_name);
        }

        /// RGB8 ASCII格式
        {
            std::snprintf(file_name, sizeof(file_name),
                          "%s/UNIT_TEST/PpmFile/RGB8_image_data_ASCII_file.SOMETHINGWRONG",
                          NativeDirectory::document_folder());

            REQUIRE((PpmFile::write_to(
                     file_name,
                     IMAGE_WIDTH, IMAGE_HEIGHT,
                     image_data_rgba8, IMAGE_PIXEL_COUNT,
                     true,     /// is RGBA
                     true,     /// use Alpha
                     false))); /// use ASCII file

            std::snprintf(file_name, sizeof(file_name),
                          "%s/UNIT_TEST/PpmFile/RGB8_image_data_ASCII_file.ppm",
                          NativeDirectory::document_folder());
            std::printf("[Output Ppm]: %s\n", file_name);
        }

        /// RGB8 BINARY格式
        {
            std::snprintf(file_name, sizeof(file_name),
                          "%s/UNIT_TEST/PpmFile/RGB8_image_data_BINARY_file",
                          NativeDirectory::document_folder());

            REQUIRE((PpmFile::write_to(
                     file_name,
                     IMAGE_WIDTH, IMAGE_HEIGHT,
                     image_data_rgba8, IMAGE_PIXEL_COUNT,
                     true,    /// is RGBA
                     true,    /// use Alpha
                     true))); /// use binary file

            std::snprintf(file_name, sizeof(file_name),
                          "%s/UNIT_TEST/PpmFile/RGB8_image_data_BINARY_file.ppm",
                          NativeDirectory::document_folder());
            std::printf("[Output Ppm]: %s\n", file_name);
         }

        std::printf("--- Checking PPM File: OK!\n");
    }

    SECTION("Checking 48bits grayscale PPM File")
    {
        std::printf("--- Checking 48bits grayscale PPM File...\n");

        static constexpr uint8_t IMAGE_WIDTH  = 128;
        static constexpr uint8_t IMAGE_HEIGHT = 128;
        static constexpr uint32_t IMAGE_PIXEL_COUNT = IMAGE_WIDTH * IMAGE_HEIGHT;

        LdrGrayscale image_data_gray16[IMAGE_PIXEL_COUNT];
        for (uint32_t i = 0; i < IMAGE_PIXEL_COUNT; ++i)
        {
            image_data_gray16[i].s =
                (uint16_t)((i * 65535u) / (IMAGE_PIXEL_COUNT - 1));
        }

        char file_name[1024];

        /// 输出48bits RGB16 Binary PPM文件
        std::snprintf(file_name, sizeof(file_name),
                      "%s/UNIT_TEST/PpmFile/GRAY16_image_data_BINARY_file",
                      NativeDirectory::document_folder());

        REQUIRE((PpmFile::write_to(
                 file_name,
                 IMAGE_WIDTH, IMAGE_HEIGHT,
                 image_data_gray16, IMAGE_PIXEL_COUNT,
                 true))); /// use binary file

        std::snprintf(file_name, sizeof(file_name),
                      "%s/UNIT_TEST/PpmFile/GRAY16_image_data_BINARY_file.ppm",
                      NativeDirectory::document_folder());

        require_ppm16_binary_file_data(
            file_name, IMAGE_WIDTH, IMAGE_HEIGHT, image_data_gray16);
        std::printf("[Output Ppm]: %s\n", file_name);

        /// 输出48bits RGB16 ASCII PPM文件
        std::snprintf(file_name, sizeof(file_name),
                      "%s/UNIT_TEST/PpmFile/GRAY16_image_data_ASCII_file",
                      NativeDirectory::document_folder());

        REQUIRE((PpmFile::write_to(
                 file_name,
                 IMAGE_WIDTH, IMAGE_HEIGHT,
                 image_data_gray16, IMAGE_PIXEL_COUNT,
                 false))); /// use ASCII file

        std::snprintf(file_name, sizeof(file_name),
                      "%s/UNIT_TEST/PpmFile/GRAY16_image_data_ASCII_file.ppm",
                      NativeDirectory::document_folder());

        require_ppm16_ascii_file_data(
            file_name, IMAGE_WIDTH, IMAGE_HEIGHT, image_data_gray16);
        std::printf("[Output Ppm]: %s\n", file_name);

        std::printf("--- Checking 48bits grayscale PPM File: OK!\n");
    }

} /// TEST_CASE("Checking PPM File", "[PPM File]")

#endif /// (PPM_IMAGE_FILE_CHECKING == 1)
