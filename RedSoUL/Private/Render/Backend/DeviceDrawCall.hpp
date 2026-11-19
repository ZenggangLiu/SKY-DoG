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
    Created:  8/11/26  @  3:10 PM
    FileName: DeviceDrawCall.hpp @ RedSoUL Project
    History:
             - created by: 8/11/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint32_t
/// Library headers
#include "DataType/Matrix4x4.hpp"
#include "Render/Backend/DeviceBufferId.hpp"
#include "Render/Backend/DevicePipelineId.hpp"
#include "Render/Backend/DeviceTextureId.hpp"


/// 一次Indexed Draw所需的Device资源及变换数据
///
/// RenderDeviceBase负责组织此数据，具体Graphic API Backend使用它提交绘制命令
struct DeviceDrawCall
{
    DevicePipelineIdT pipeline_id;
    DeviceBufferIdT   vertex_buffer_id;
    DeviceBufferIdT   index_buffer_id;
    DeviceTextureIdT  texture_id;
    uint32_t          index_count;
    matrix_4x4        model_view_transform;
    matrix_4x4        projection_transform;
};
