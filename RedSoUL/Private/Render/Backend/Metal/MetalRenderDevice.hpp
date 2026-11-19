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
    Created:  10/11/26  @  10:09 PM
    FileName: MetalRenderDevice.hpp @ RedSoUL Project
    History:
             - created by: 10/11/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


#if defined(__APPLE__)

/// Library headers
#include "Render/Backend/DeviceBufferDesc.hpp"
#include "Render/Backend/DevicePipelineDesc.hpp"
#include "Render/Backend/DeviceTextureDesc.hpp"
#include "Render/Backend/RenderDevice.hpp"


class RenderDeviceFactory;


/// Metal RenderDevice
///
/// NOTE:只允许RenderDeviceFactory创建和销毁
///
class MetalRenderDevice final : public RenderDevice
{
public:
    bool
    initialize (
        void * const native_surface);

    void
    destroy ();

    DeviceBufferIdT
    create_buffer (
        const DeviceBufferDesc & desc);

    void
    destroy_buffer (
        const DeviceBufferIdT buffer_id);

    DeviceTextureIdT
    create_texture (
        const DeviceTextureDesc & desc);

    void
    destroy_texture (
        const DeviceTextureIdT texture_id);

    DevicePipelineIdT
    create_pipeline (
        const DevicePipelineDesc & desc);

    void
    destroy_pipeline (
        const DevicePipelineIdT pipeline_id);

    bool
    begin_frame (
        void * const native_surface);

    bool
    draw_indexed (
        const DeviceDrawCall & draw_call);

    bool
    end_frame ();


private:
    friend class RenderDeviceFactory;

     MetalRenderDevice ();
    ~MetalRenderDevice ();

private:
    struct State;

    State * m_state;
};

#endif /// defined(__APPLE__)
