/***************************************************************************************
                                                                                        
        *          .               *                              .               *     
        ███████╗██╗  ██╗██╗   ██╗        ██████╗  ██████╗  ██████╗         *            
        ██╔════╝██║ ██╔╝╚██╗ ██╔╝        ██╔══██╗██╔═══██╗██╔════╝                      
        ███████╗█████╔╝  ╚████╔╝         ██║  ██║██║   ██║██║  ███╗        .            
        ╚════██║██╔═██╗   ╚██╔╝          ██║  ██║██║   ██║██║   ██║                     
        ███████║██║  ██╗    ██║           ██████╔╝╚██████╔╝╚██████╔╝         *          
        ╚══════╝╚═╝  ╚═╝    ╚═╝           ╚═════╝  ╚═════╝  ╚═════╝                     
                                                                                        
        <~~~               .        SKY Dog Game                      ~~~>        *     
                                Real-Time | Cross-Platform           .                  
----------------------------------------------------------------------------------------
                                                                                        
                                  ,,                                                    
                  __           o-°°|\_____/)                                            
    Author:   (___()'`; Zee...  \_/|_)     )                                            
              /,    /`             \  __  /                                             
              \\"--\\              (_/ (_/                                              
    Created:  24/08/26  @  6:51 PM
    FileName: RTextureHead_v1.0.hpp @ RedSoUL Project
    History:
             - created by: 24/08/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint8_t
/// Library headers
#include "Common/CommonDefines.hpp" /// INLINE_FUNCTION, PACKED_STRUCT
#include "IO/SoULFileHead.hpp"
#include "Render/RenderTextureId.hpp"
#include "Render/TextureDataType.hpp"


#define RTEXTURE_FILE_MAGIC_NUMBER  FOUR_CC_32('R', 'T', 'E', 'X')
#define RTEXTURE_FILE_HEAD_END_MARK FOUR_CC_64('R', 'T', 'E', 'X', 'S', 'L', 'U', 'S')


/// Render Texture文件1.0版本
///
/// +--------------------------+
/// |                          |
/// | == RenderTexture v1.0 == |
/// |                          |
/// +--------------------------+
/// | Magic Number             | <-- 四个字节(32位):"RTEX"的FourCC
/// +--------------------------+
/// | Major Version            | <-- 二个字节(16位):主版本号(1)
/// |--------------------------+
/// | Minor Version            | <-- 二个字节(16位):辅版本号(0)
/// |--------------------------+
/// | File Flags               | <-- 二个字节(16位):文件标记(保留。设置为0)
/// +--------------------------+
/// | Texture Id               | <-- 八个字节(64位):作为Key使用,必须转换为RenderTextureIdT
/// +--------------------------+
/// | Texture Width            | <-- 四个字节(32位)
/// +--------------------------+
/// | Texture Height           | <-- 四个字节(32位)
/// +--------------------------+
/// | Mipmap Count             | <-- 一个字节(8位)
/// +--------------------------+
/// | Texture Flags            | <-- 二个字节(16位)
/// +--------------------------+
/// | Texture Data Type        | <-- 一个字节(8位):TextureDataType类型
/// +--------------------------+
/// | Slice Data Size          | <-- 四个字节(32位):每个Slice的Texel列表占多少字节
/// +--------------------------+
/// | Texel List Size          | <-- 四个字节(32位):所有Slice的Texel列表占多少字节
/// +--------------------------+
/// | Texel Data Offset        | <-- 四个字节(32位):第一个Texel数据相对文件头偏移多少字节
/// +--------------------------+
/// | Header End               | <-- 八个字节(64位):"RTEXSLUS"的FourCC
/// +--------------------------+
///
/// === Texture Flags ===:
/// MSB                         LSB
/// 15  14  13 ...  1           0
/// +---+---+-------+-----------+-------------+
/// | 0 | 0 |  ...  | IsCubeMap | IsLinearRgb |
/// +---+---+-------+-----------+-------------+
///
PACKED_STRUCT(RTextureHead_v1_0 : public SoULFileHead,
    typedef SoULFileHead SuperT;

    const uint64_t texture_id;
    const uint32_t texture_width;
    const uint32_t texture_height;
    const uint8_t  mipmap_count;
    const uint16_t texture_flags;
    const uint8_t  texture_data_type;
    const uint32_t slice_data_size;
    const uint32_t texel_list_size;
    const uint32_t texel_data_offset;
    const uint64_t head_end_mark;

    INLINE_FUNCTION
    RTextureHead_v1_0 (
        const RenderTextureIdT _texture_id,
        const uint32_t         _texture_width,
        const uint32_t         _texture_height,
        const uint8_t          _mipmap_count,
        const bool             _is_cube_map,
        const bool             _is_linear_rgb,
        const TextureDataType  _texture_data_type,
        const uint32_t         _slice_data_size,
        const uint32_t         _texel_list_size,
        const uint32_t         _texel_data_offset)
    :
        SuperT(RTEXTURE_FILE_MAGIC_NUMBER, 1, 0, 0),
        texture_id((uint64_t)_texture_id.value),
        texture_width(_texture_width),
        texture_height(_texture_height),
        mipmap_count(_mipmap_count),
        texture_flags(compose_texture_flags(_is_cube_map, _is_linear_rgb)),
        texture_data_type((uint8_t)_texture_data_type),
        slice_data_size(_slice_data_size),
        texel_list_size(_texel_list_size),
        texel_data_offset(_texel_data_offset),
        head_end_mark(RTEXTURE_FILE_HEAD_END_MARK)
    {}

    INLINE_FUNCTION
    uint16_t
    compose_texture_flags (
        const bool _is_cube_map,
        const bool _is_linear_rgb) const
    {
        /// === Texture Flags ===:
        /// MSB                         LSB
        /// 15  14  13 ...  1           0
        /// +---+---+-------+-----------+-------------+
        /// | 0 | 0 |  ...  | IsCubeMap | IsLinearRgb |
        /// +---+---+-------+-----------+-------------+
        return (((uint16_t)_is_cube_map) << 1) | (uint16_t)_is_linear_rgb;
    }

    INLINE_FUNCTION
    void
    decompose_texture_flags (
        bool & _is_cube_map,
        bool & _is_linear_rgb) const
    {
        /// === Texture Flags ===:
        /// MSB                         LSB
        /// 15  14  13 ...  1           0
        /// +---+---+-------+-----------+-------------+
        /// | 0 | 0 |  ...  | IsCubeMap | IsLinearRgb |
        /// +---+---+-------+-----------+-------------+
        ///
        _is_cube_map   = ((texture_flags & 0x02) >> 1) == 1;
        _is_linear_rgb = ((texture_flags & 0x01) >> 0) == 1;
    }
);
