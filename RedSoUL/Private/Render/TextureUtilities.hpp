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
    Created:  10/11/26  @  5:34 PM
    FileName: TextureUtilities.hpp @ RedSoUL Project
    History:
             - created by: 10/11/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint8_t, uint32_t
/// Library headers
#include "Render/TextureDataType.hpp"


struct TextureUtility
{
    /// 计算下一个Mipmap的尺寸(宽度/高度)
    ///
    /// @param[in]  mipmap_dim
    ///     当前Mipmap的尺寸(宽度/高度)
    static
    uint32_t
    next_mipmap_dimension (
        const uint32_t mipmap_dim);

    /// 计算指定宽度, 高度的纹理最多的Mipmap个数
    static
    uint8_t
    maximal_mipmap_count (
        uint32_t width,
        uint32_t height);

    /// 获取每个Texel的字节大小
    ///
    /// @param[in]  host_format
    ///     Host Texture(即RenderTexture)的格式
    static
    uint8_t
    texel_size_in_bytes (
        const TextureDataType host_format);
};
