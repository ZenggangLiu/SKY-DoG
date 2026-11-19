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
    Created:  8/11/26  @  3:16 PM
    FileName: RenderDevice.hpp @ RedSoUL Project
    History:
             - created by: 8/11/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// Library headers
#include "Common/PlatformDefines.hpp" /// OS_TYPE
#include "Render/Backend/DeviceMeshId.hpp"
#include "Render/Backend/DeviceTextureId.hpp"
#include "Render/Backend/DeviceDrawCall.hpp"
#include "Render/RenderMeshId.hpp"
#include "Render/RenderTextureId.hpp"


struct matrix_4x4;


/// Render BackEnd
///
/// - RenderDevice基类: 提供Static Dispatching
/// - 将Host资源(例如: RenderMesh)映射为Device资源
/// - 由当前平台的具体Graphic API实现类创建资源并提交绘制
///
template < typename DetailedRenderDevice > /// 具体的RenderDevice实现
class RenderDeviceBase
{
public:
    /// 准备一个Draw Item
    /// NOTE: 如果Host资源尚未缓存, 则创建对应Device资源
    bool
    prepare_draw (
        const RenderMeshIdT    host_mesh_id,
        const RenderTextureIdT host_texture_id,
        const matrix_4x4 &     model_view_transform,
        const matrix_4x4 &     projection_transform);

    /// 绘制并清空当前Frame中准备好的全部Draw Item
    bool
    draw (
        void * const native_surface);

    void
    clear_frame ();

    void
    release_resources ();


protected:
     RenderDeviceBase ();
    ~RenderDeviceBase ();


private:
    struct State;

    /// 获得RenderDevice的具体实现
    DetailedRenderDevice &
    detailed_device ();

    DeviceMeshIdT
    cache_mesh (
        const RenderMeshIdT host_mesh_id);

    DeviceTextureIdT
    cache_texture (
        const RenderTextureIdT host_texture_id);

    RenderDeviceBase (const RenderDeviceBase &) = delete;
    RenderDeviceBase & operator= (const RenderDeviceBase &) = delete;


private:
    State * m_state;
};


/// RenderDevice定义
#if defined(__APPLE__)
class MetalRenderDevice;
using RenderDevice = RenderDeviceBase<MetalRenderDevice>;
/// 外部显性实例化: 在RenderDevice.cpp中提供具体实例化
extern template class RenderDeviceBase<MetalRenderDevice>;
#elif (OS_TYPE == OS_TYPE_WIN)
class Direct3DRenderDevice;
using RenderDevice = RenderDeviceBase<Direct3DRenderDevice>;
/// 外部显性实例化: 在RenderDevice.cpp中提供具体实例化
extern template class RenderDeviceBase<Direct3DRenderDevice>;
#else
class VulkanRenderDevice;
using RenderDevice = RenderDeviceBase<VulkanRenderDevice>;
/// 外部显性实例化: 在RenderDevice.cpp中提供具体实例化
extern template class RenderDeviceBase<VulkanRenderDevice>;
#endif
