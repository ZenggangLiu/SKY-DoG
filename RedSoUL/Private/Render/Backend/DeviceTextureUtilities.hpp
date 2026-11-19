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
    Created:  10/11/26  @  10:13 PM
    FileName: DeviceTextureUtilities.hpp @ RedSoUL Project
    History:
             - created by: 10/11/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint8_t
/// Library headers
#include "Render/TextureDataType.hpp"
#include "Render/Backend/DeviceTextureFormat.hpp"


struct DeviceTextureUtility
{
    static
    TextureDataType
    to_host_texture_format (
        const DeviceTextureFormat device_format);

    static
    DeviceTextureFormat
    to_device_texture_format (
        const TextureDataType host_format,
        const bool            is_linear_rgb);

    /// 获取每个Texel的字节大小
    ///
    /// @param[in]  device_format
    ///     Device Texture的格式
    static
    uint8_t
    texel_size_in_bytes (
        const DeviceTextureFormat device_format);
};
