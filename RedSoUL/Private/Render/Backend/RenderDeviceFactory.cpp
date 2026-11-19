/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Common/PlatformDefines.hpp" /// OS_TYPE
#include "Render/Backend/RenderDeviceDesc.hpp"
#if defined(__APPLE__)
#include "Render/Backend/Metal/MetalRenderDevice.hpp"
#elif (OS_TYPE == OS_TYPE_WIN)
#else
#endif
/// Self header
#include "Render/Backend/RenderDeviceFactory.hpp"


RenderDevice *
RenderDeviceFactory::create (
    const RenderDeviceDesc & desc)
{
    RenderDevice * new_device = nullptr;

    switch (desc.render_device_type)
    {
        case RenderDeviceType::METAL_RENDER_DEVICE:
        {
#if defined(__APPLE__)
            MetalRenderDevice * const mtl_device = new MetalRenderDevice();
            if (mtl_device->initialize(desc.native_surface))
            {
                new_device = mtl_device;
            }
            else
            {
                delete mtl_device;
            }
#elif (OS_TYPE == OS_TYPE_WIN)
            RUNTIME_ASSERT(false, "Windows uses DIRECT3D_RENDER_DEVICE Device!!");
#else
            RUNTIME_ASSERT(false, "Linux/Android uses VULKAN_RENDER_DEVICE Device!!");
#endif
            break;
        }

        case RenderDeviceType::DIRECT3D_RENDER_DEVICE:
        {
#if (OS_TYPE == OS_TYPE_WIN)
            RUNTIME_ASSERT(false, "No Implementation!!");
#elif defined(__APPLE__)
            RUNTIME_ASSERT(false, "Apple uses METAL_RENDER_DEVICE Device!!");
#else
            RUNTIME_ASSERT(false, "Linux/Android uses VULKAN_RENDER_DEVICE Device!!");
#endif
            break;
        }

        case RenderDeviceType::VULKAN_RENDER_DEVICE:
        {
#if defined(__APPLE__)
            RUNTIME_ASSERT(false, "Apple uses METAL_RENDER_DEVICE Device!!");
#elif (OS_TYPE == OS_TYPE_WIN)
            RUNTIME_ASSERT(false, "Windows uses DIRECT3D_RENDER_DEVICE Device!!");
#else
            RUNTIME_ASSERT(false, "No Implementation!!");
#endif
            break;
        }

        default:
        {
            RUNTIME_ASSERT(false, "Unknown RenderDeviceType!!");
            break;
        }
    }

    return new_device;
}


void
RenderDeviceFactory::destroy (
    RenderDevice * const device_impl)
{
#if defined(__APPLE__)
    delete static_cast<MetalRenderDevice *>(device_impl);
#elif (OS_TYPE == OS_TYPE_WIN)
    RUNTIME_ASSERT(false, "No Implementation!!");
    (void)device_impl;
#else
    RUNTIME_ASSERT(false, "No Implementation!!");
    (void)device_impl;
#endif
}
