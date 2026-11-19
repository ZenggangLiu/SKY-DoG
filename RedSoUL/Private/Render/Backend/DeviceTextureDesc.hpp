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
    Created:  6/11/26  @  5:39 PM
    FileName: DeviceTextureDesc.hpp @ RedSoUL Project
    History:
             - created by: 6/11/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint8_t, uint32_t
/// Library headers
#include "Render/Backend/DeviceTextureFormat.hpp"


/// DeviceTexture描述符
struct DeviceTextureDesc
{
    /// Texel Layout: Slice0[Mip0, ..., MipN], ..., SliceN[Mip0, ..., MipN]
    const void * const        texel_array;
    const uint32_t            array_size;
    const uint32_t            texture_width;
    const uint32_t            texture_height;
    const uint8_t             mipmap_count;
    const uint8_t             slice_count;
    const bool                is_cube_map;
    const DeviceTextureFormat texture_format;
};
