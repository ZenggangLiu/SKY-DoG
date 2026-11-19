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
    Created:  5/11/26  @  11:33 PM
    FileName: RenderDeviceDesc.hpp @ RedSoUL Project
    History:
             - created by: 5/11/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint8_t


/// RenderDevice的具体类型
///
enum class RenderDeviceType : uint8_t
{
    /// Direct3D Api
    DIRECT3D_RENDER_DEVICE,
    /// Metal Api
    METAL_RENDER_DEVICE,
    /// Vulkan Api
    VULKAN_RENDER_DEVICE,
};


/// 创建RenderDevice所需的描述
///
struct RenderDeviceDesc
{
    const RenderDeviceType render_device_type;
    /// 系统提供的渲染窗口(Windows, View...)的指针
    void * const           native_surface;
};
