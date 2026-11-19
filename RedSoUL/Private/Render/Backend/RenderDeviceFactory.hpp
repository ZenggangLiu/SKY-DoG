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
    Created:  5/11/26  @  4:54 PM
    FileName: RenderDeviceFactory.hpp @ RedSoUL Project
    History:
             - created by: 5/11/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// Library headers
#include "Render/Backend/RenderDevice.hpp"


struct RenderDeviceDesc;


/// RenderDevice工厂(唯一的创建/销毁RenderDevice的接口)
///
class RenderDeviceFactory
{
public:
    /// 创建RenderDevice的具体实现
    static
    RenderDevice *
    create (
        const RenderDeviceDesc & desc);

    /// 销毁RenderDevice的具体实现
    static
    void
    destroy (
        RenderDevice * const device_impl);


private:
    RenderDeviceFactory () = delete;
};
