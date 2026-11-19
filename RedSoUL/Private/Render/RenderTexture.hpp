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
    Created:  9/08/26  @  10:38 PM
    FileName: RenderTexture.hpp @ RedSoUL Project
    History:
             - created by: 9/08/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint32_t,...
/// Library headers
#include "Render/RenderTextureId.hpp"
#include "Render/TextureDataType.hpp"


/// Texel数据Layout: Slice0, Slice1, ..., SliceN
/// NOTE: 贴图由N个连续的Slice构成
///
///                  +-----------------------+
///                  |        Slice N        |
///          ~~~     |         Mip 0         |
/// +-----------------------+                |
/// |        Slice 0        |----+-----------+
/// |         Mip 0         | 1  | <- Mip 1
/// |                       |    |
/// +-----------+-----------+----+
/// |   Mip 1   |    | Mip 2 |
/// |           |    +-------+
/// +-------+---+
/// | Mip 2 |
/// +-------+
///
struct RenderTexture
{
    /// Texel数据列表
    const uint8_t * const  texel_list;
    /// Texture Id
    const RenderTextureIdT texture_id;
    /// 贴图宽度(Mipmap0横向Texel数)
    const uint32_t         texture_width;
    /// 贴图高度(Mipmap0纵向Texel数)
    const uint32_t         texture_height;
    /// Slice中的所有Texel数据大小(字节数|即, 所有Mipmap中Texel的总和)
    const uint32_t         slice_data_size;
    /// Texel数据列表大小(字节数)
    const uint32_t         texel_list_size;
    /// Mipmap总数。如果, 无MipMap, 则设定为1
    const uint8_t          mipmap_count;
    /// 贴图数据类型
    const TextureDataType  texture_data_type;
    /// 标记贴图是否为CubeMap(True)
    const bool             is_cube_map;
    /// 标记LdrColor Texel(RGBA8)数据是否使用Linear Rgb空间(True),
    /// 还是使用Gamma encoded Rgb空间(False)
    /// NOTE: HdrColor/LdrGrayscale Texel数据始终使用Linear Rgb空间(True)
    const bool             is_linear_rgb;
    /// 标记Texel数据是否否为动态分配(True)
    const bool             is_dyn_allocated;

    RenderTexture (
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
        const uint32_t         texel_list_size);
};
