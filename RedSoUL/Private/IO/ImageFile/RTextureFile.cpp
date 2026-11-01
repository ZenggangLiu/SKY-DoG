/// System headers
#include <cstdio>   /// std::snprintf
#include <cstdlib>  /// std::malloc, std::free
/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Common/CommonDefines.hpp" /// MAKE_FILE_VERSION
#include "FileSystem/FileName.hpp"
#include "FileSystem/NativeFile.hpp"
#include "FileSystem/NativeReadStream.hpp"
#include "FileSystem/NativeWriteStream.hpp"
#include "Memory/MemoryUtilities.hpp"
#include "Render/RenderTexture.hpp"
#include "IO/ImageFile/RTextureHead_v1.0.hpp"
/// Self header
#include "IO/ImageFile/RTextureFile.hpp"


/// 定义v1.0
static constexpr uint16_t FILE_VERSION_1_0 = MAKE_FILE_VERSION(1, 0);

/// 当前版本定义
/// NOTE: 如果添加新的版本, 需要提供对老版本的转换函数!!
static constexpr uint16_t CURRENT_VERSION = FILE_VERSION_1_0;


/// 反序列化RenderMesh文件(v1.0版本)
///
/// 如果出现错误: buffer内保存错误信息
static
bool
deserialize_from_rtexture_file_v1_0 (
    NativeReadStream &     texture_file,
    const RenderTextureIdT exp_texture_id,
    uint32_t &             texture_width,
    uint32_t &             texture_height,
    uint8_t  &             mipmap_count,
    bool &                 is_cube_map,
    bool &                 is_linear_rgb,
    TextureDataType &      texture_data_type,
    uint32_t &             slice_data_size,
    uint8_t * &            texel_list,
    uint32_t &             texel_list_size,
    char * const           buffer,
    const uint32_t         buffer_size)
{
    slice_data_size = 0;
    texel_list_size = 0;

    /// 读入文件头的后续部分
    int32_t loaded_bytes =
        texture_file.read(
            (uint8_t*)buffer, buffer_size - sizeof(SoULFileHead),
            sizeof(SoULFileHead), sizeof(RTextureHead_v1_0) - sizeof(SoULFileHead));
    if ((uint32_t)loaded_bytes == sizeof(RTextureHead_v1_0) - sizeof(SoULFileHead))
    {
        const RTextureHead_v1_0 & rtexture_file_head =
            *(const RTextureHead_v1_0*)buffer;
        if (rtexture_file_head.head_end_mark == RTEXTURE_FILE_HEAD_END_MARK)
        {
            /// 判断文件中存储的TextureId是否与期待的Id相同
            const RenderTextureIdT stored_texture_id(
                (RenderTextureIdT::HashedValueTypeT)rtexture_file_head.texture_id);
            if (stored_texture_id == exp_texture_id)
            {
                RUNTIME_ASSERT(
                    rtexture_file_head.texel_data_offset >= sizeof(RTextureHead_v1_0),
                    "Texel data offset is inside file head!!");
                RUNTIME_ASSERT(
                    (rtexture_file_head.texel_data_offset +
                     rtexture_file_head.texel_list_size) <= texture_file.file_length(),
                    "Texel data is out of file range!!");

                /// 赋值
                texture_width  = rtexture_file_head.texture_width;
                texture_height = rtexture_file_head.texture_height;
                mipmap_count   = rtexture_file_head.mipmap_count;

                rtexture_file_head.decompose_texture_flags(is_cube_map, is_linear_rgb);
                texture_data_type =
                    (TextureDataType)rtexture_file_head.texture_data_type;

                /// 创建Texel数据列表
                texel_list = (uint8_t*)std::malloc(rtexture_file_head.texel_list_size);

                /// 读入Texel数据
                if (texel_list)
                {
                    /// 尝试SEEK至Texel数据保存处
                    if (texture_file.seek(
                        (int32_t)rtexture_file_head.texel_data_offset,
                        SeekMode::FILE_BEGIN_SEEK_MODE) == false)
                    {
                        std::free(texel_list);
                        std::snprintf(
                            buffer, buffer_size,
                            "can not seek to the texel data list!!\n");
                        return false;
                    }

                    loaded_bytes = texture_file.read(
                        texel_list, rtexture_file_head.texel_list_size,
                        0, rtexture_file_head.texel_list_size);
                    if ((uint32_t)loaded_bytes == rtexture_file_head.texel_list_size)
                    {
                        slice_data_size = rtexture_file_head.slice_data_size;
                        texel_list_size = rtexture_file_head.texel_list_size;
                        return true;
                    }
                    else
                    {
                        std::free(texel_list);
                        std::snprintf(
                            buffer, buffer_size, "can not load the texel data list!!\n");
                        return false;
                    }
                }
                else
                {
                    std::snprintf(
                        buffer, buffer_size,
                        "can not allocate memory for the texel data list!!\n");
                    return false;
                }
            }
            else
            {
                std::snprintf(
                    buffer, buffer_size,
                    "stored texture id does not match the expecting one!!\n");
                return false;
            }
        }
        else
        {
            std::snprintf(
                buffer, buffer_size,
                "render texture file head end mark does not match!!\n");
            return false;
        }
    }
    else
    {
        std::snprintf(
            buffer, buffer_size,
            "can not load the Render Texture file head version 1.0\n");
        return false;
    }
}


/// 序列化为RenderTexture文件(v1.0版本)
///
/// NOTE: 如果出现错误: buffer内保存错误信息
static
bool
serialize_to_rtexture_file_v1_0 (
    const RenderTexture & texture_data,
    NativeWriteStream &   texture_file,
    char * const          buffer,
    const uint32_t        buffer_size)
{
    /// Texel数据相对偏移
    static const uint32_t TEXEL_DATA_OFFSET = sizeof(RTextureHead_v1_0);

    const RTextureHead_v1_0 file_header
    {
        texture_data.texture_id, /// 使用RenderTexture保存的Id作为文件中的Id
        texture_data.texture_width,
        texture_data.texture_height,
        texture_data.mipmap_count,
        texture_data.is_cube_map,
        texture_data.is_linear_rgb,
        texture_data.texture_data_type,
        texture_data.slice_data_size,
        texture_data.texel_list_size,
        TEXEL_DATA_OFFSET
    };

    /// 输出文件头
    bool opcode = texture_file.write(
        (const uint8_t*)&file_header, sizeof(RTextureHead_v1_0),
        0, sizeof(RTextureHead_v1_0));
    if (opcode)
    {
        /// 输出Texel数据
        ///
        /// RenderTexel文件Layout:
        ///
        /// +--------------------------+
        /// | Render Texture File Head |
        /// +--------------------------+
        /// | Texel Data List          |
        /// +--------------------------+
        ///
        opcode = texture_file.write(
            texture_data.texel_list, texture_data.texel_list_size,
            0, texture_data.texel_list_size);
        if (opcode == false)
        {
            std::snprintf(buffer, buffer_size, "can not store texel data list!!\n");
        }
    }
    else
    {
        std::snprintf(buffer, buffer_size, "can not store file head!!\n");
    }
    return opcode;
}



// MARK: == RenderMesh File ==
bool
RTextureFile::read_from (
    const char * const     abs_file_name,
    const RenderTextureIdT exp_texture_id,
    uint32_t &             texture_width,
    uint32_t &             texture_height,
    uint8_t  &             mipmap_count,
    bool &                 is_cube_map,
    bool &                 is_linear_rgb,
    TextureDataType &      texture_data_type,
    uint32_t &             slice_data_size,
    uint8_t * &            texel_list,
    uint32_t &             texel_list_size)
{
    RUNTIME_ASSERT( abs_file_name, "File name can not be NULL!!");
    RUNTIME_ASSERT(*abs_file_name, "File name can not be empty!!");
    RUNTIME_ASSERT(exp_texture_id.is_valid(), "Expected texture Id must be valid!!");

    char buffer[1024];

    /// 确保使用'.rtexture'文件扩展符
    FileName::append_file_extension(abs_file_name, "rtexture", buffer, sizeof(buffer));

    slice_data_size = 0;
    texel_list_size = 0;

    /// 创建ReadOnly文件流
    NativeReadStream texture_file(buffer);
    if (texture_file.is_opened())
    {
        /// 读入RedSoUL文件头
        if (SoULFileHead::create_from_stream(texture_file, *(SoULFileHead*)buffer))
        {
            const SoULFileHead & soul_file_head = *(const SoULFileHead*)buffer;
            if (soul_file_head.magic_number == RTEXTURE_FILE_MAGIC_NUMBER)
            {
                const uint16_t file_version =
                    MAKE_FILE_VERSION(soul_file_head.major_version,
                                      soul_file_head.minor_version);
                switch (file_version)
                {
                    case FILE_VERSION_1_0:
                    {
                        const bool opcode = deserialize_from_rtexture_file_v1_0(
                            texture_file, exp_texture_id,
                            texture_width, texture_height,
                            mipmap_count, is_cube_map, is_linear_rgb,
                            texture_data_type, slice_data_size,
                            texel_list, texel_list_size,
                            buffer, (uint32_t)sizeof(buffer));
                        if (opcode == false)
                        {
                            std::printf(
                                "[ERROR]: can not read file: %s\nwith error: %s\n",
                                abs_file_name, buffer);
                        }
                        return opcode;
                    }

                    default:
                    {
                        RUNTIME_ASSERT(false, "Unknown file version!!");
                        return false;
                    }
                }
            }
            else
            {
                std::snprintf(
                    buffer, sizeof(buffer), "this file is not a Render Texture file!!\n");
                std::printf(
                    "[ERROR]: can not read file: %s\nwith error: %s\n",
                    abs_file_name, buffer);
                return false;
            }
        }
        else
        {
            std::snprintf(buffer, sizeof(buffer), "can not read RedSoUL file head!!\n");
            std::printf(
                "[ERROR]: can not read file: %s\nwith error: %s\n",
                abs_file_name, buffer);
            return false;
        }
    }
    else
    {
        return false;
    }
}


bool
RTextureFile::write_to (
    const char * const    abs_file_name,
    const RenderTexture & texture_data)
{
    RUNTIME_ASSERT( abs_file_name, "File name can not be NULL!!");
    RUNTIME_ASSERT(*abs_file_name, "File name can not be empty!!");

    char buffer[1024];

    /// 确保使用'.rtexture'文件扩展符
    FileName::append_file_extension(abs_file_name, "rtexture", buffer, sizeof(buffer));

    /// 创建WriteOnly文件流
    NativeWriteStream texture_file(buffer);
    if (texture_file.is_opened())
    {
        switch (CURRENT_VERSION)
        {
            case FILE_VERSION_1_0:
            {
                const bool opcode =
                    serialize_to_rtexture_file_v1_0(texture_data, texture_file,
                                                    buffer, sizeof(buffer));
                if (opcode == false)
                {
                    std::printf(
                        "[ERROR]: can not store file: %s\nwith error: %s\n",
                        abs_file_name, buffer);
                }
                return opcode;
            }

            default:
            {
                RUNTIME_ASSERT(false, "Unknown file version!!");
                return false;
            }
        }
    }
    else
    {
        return false;
    }
}
