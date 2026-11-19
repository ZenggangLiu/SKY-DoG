#if defined(__APPLE__)

/// System headers
#import  <Metal/Metal.h>
#import  <MetalKit/MetalKit.h>
#include <limits> /// std::numeric_limits
/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Common/CommonDefines.hpp"    /// INLINE_FUNCTION
#include "Common/PlatformDefines.hpp"  /// OS_TYPE
#include "Render/Backend/DeviceTextureFormat.hpp"
#include "Render/Backend/DeviceTextureUtilities.hpp"
#include "Render/Backend/Metal/MetalShaderTypes.h"
#include "Render/Backend/Metal/MetalTextureUtility.hpp"
#include "Render/TextureUtilities.hpp"
/// Self header
#include "Render/Backend/Metal/MetalRenderDevice.hpp"


/// MARK: == Helpers ==
/// 将Row Major的matrix_4x4转换为MSL需要的Column Major类型matrix_float4x4
static
matrix_float4x4
row_major_matrix_to_column_major (
    const matrix_4x4 & matrix)
{
    return (matrix_float4x4)
    {
        {
            { matrix.e00, matrix.e10, matrix.e20, matrix.e30 },
            { matrix.e01, matrix.e11, matrix.e21, matrix.e31 },
            { matrix.e02, matrix.e12, matrix.e22, matrix.e32 },
            { matrix.e03, matrix.e13, matrix.e23, matrix.e33 },
        }
    };
}



/// MARK: == MetalPipelineResouece ==
@interface MetalPipelineResource : NSObject

@property (nonatomic, strong) id<MTLRenderPipelineState> pipeline_state;
@property (nonatomic, strong) id<MTLDepthStencilState>   depth_stencil_state;

@end


@implementation MetalPipelineResource
@end



/// MARK: == MetalRenderDevice::State ===
struct MetalRenderDevice::State
{
    id<MTLDevice>        __strong mtl_device;
    id<MTLLibrary>       __strong shader_library;
    id<MTLCommandQueue>  __strong command_queue;
    dispatch_semaphore_t          frame_semaphore;

    NSMutableDictionary<NSNumber*, id<MTLBuffer>> *          __strong buffer_table;
    NSMutableDictionary<NSNumber*, id<MTLTexture>> *         __strong texture_table;
    NSMutableDictionary<NSNumber*, MetalPipelineResource*> * __strong pipeline_table;

    DeviceBufferIdT   next_buffer_id;
    DeviceTextureIdT  next_texture_id;
    DevicePipelineIdT next_pipeline_id;

    id<MTLCommandBuffer>        __strong current_command_buffer;
    id<MTLRenderCommandEncoder> __strong current_render_encoder;
    id<CAMetalDrawable>         __strong current_drawable;

    INLINE_FUNCTION
    State ()
    :
        mtl_device(nil),
        shader_library(nil),
        command_queue(nil),
        frame_semaphore(nil),
        buffer_table(nil),
        texture_table(nil),
        pipeline_table(nil),
        next_buffer_id(0),
        next_texture_id(0),
        next_pipeline_id(0),
        current_command_buffer(nil),
        current_render_encoder(nil),
        current_drawable(nil)
    {

    }

    INLINE_FUNCTION
    id<MTLBuffer>
    buffer (
        const DeviceBufferIdT buffer_id) const
    {
        return [buffer_table objectForKey:
            [NSNumber numberWithUnsignedLongLong:
                (unsigned long long)buffer_id.value]];
    }

    INLINE_FUNCTION
    id<MTLTexture>
    texture (
        const DeviceTextureIdT texture_id) const
    {
        return [texture_table objectForKey:
            [NSNumber numberWithUnsignedLongLong:
                (unsigned long long)texture_id.value]];
    }

    INLINE_FUNCTION
    MetalPipelineResource *
    pipeline (
        const DevicePipelineIdT pipeline_id) const
    {
        return [pipeline_table objectForKey:
            [NSNumber numberWithUnsignedLongLong:
                (unsigned long long)pipeline_id.value]];
    }
};



/// MARK: == MetalRenderDevice ==
bool
MetalRenderDevice::initialize (
    void * const native_surface)
{
    /// 限制同时处于GPU执行队列中的Frame数量, 防止CPU无限领先GPU
    static constexpr uint8_t MAX_BUFFERED_FRAME_COUNT = 3;

    if (m_state->mtl_device == nil)
    {
        if (native_surface)
        {
            MTKView * const mtk_view = (__bridge MTKView *)native_surface;

            /// 获得确实Metal Device
            id<MTLDevice> const mtl_device =
                mtk_view.device
                ? mtk_view.device
                : MTLCreateSystemDefaultDevice();
            if (mtl_device == nil)
            {
                NSLog(@"Metal is not supported on this device!!");
                return false;
            }
            else
            {
                /// 初始化Metal Device
                mtk_view.device = mtl_device;
                m_state->mtl_device = mtl_device;

                m_state->shader_library = [mtl_device newDefaultLibrary];
                m_state->command_queue  = [mtl_device newCommandQueue];
                if (m_state->shader_library && m_state->command_queue)
                {
                    m_state->frame_semaphore =
                        dispatch_semaphore_create(MAX_BUFFERED_FRAME_COUNT);
                    m_state->buffer_table   = [[NSMutableDictionary alloc] init];
                    m_state->texture_table  = [[NSMutableDictionary alloc] init];
                    m_state->pipeline_table = [[NSMutableDictionary alloc] init];
                    if (m_state->frame_semaphore && m_state->buffer_table &&
                        m_state->texture_table   && m_state->pipeline_table)
                    {
                        mtk_view.depthStencilPixelFormat =
                            MTLPixelFormatDepth32Float_Stencil8;
                        mtk_view.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
                        mtk_view.sampleCount = 1;
                        /// Reversed-Z, 使用0清除Z-Buffer
                        mtk_view.clearDepth  = 0.0;
                        return true;
                    }
                    else
                    {
                        destroy();
                        return false;
                    }
                }
                else
                {
                    destroy();
                    return false;
                }
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return true;
    }
}


void
MetalRenderDevice::destroy ()
{
    m_state->current_render_encoder = nil;
    m_state->current_command_buffer = nil;
    m_state->current_drawable       = nil;

    [m_state->pipeline_table removeAllObjects];
    [m_state->texture_table  removeAllObjects];
    [m_state->buffer_table   removeAllObjects];
    m_state->pipeline_table = nil;
    m_state->texture_table  = nil;
    m_state->buffer_table   = nil;

    m_state->next_buffer_id.reset();
    m_state->next_texture_id.reset();
    m_state->next_pipeline_id.reset();

    m_state->frame_semaphore = nil;
    m_state->command_queue   = nil;
    m_state->shader_library  = nil;
    m_state->mtl_device      = nil;
}


DeviceBufferIdT
MetalRenderDevice::create_buffer (
    const DeviceBufferDesc & desc)
{
    /// 非法Device Buffer Id
    DeviceBufferIdT buffer_id{};

    if (m_state->mtl_device && desc.array_size)
    {
        id<MTLBuffer> mtl_buffer;
        if (desc.data_array)
        {
            mtl_buffer =
                [m_state->mtl_device newBufferWithBytes:desc.data_array
                                                 length:desc.array_size
                                                options:MTLResourceStorageModeShared];
        }
        else
        {
            /// ZERO FILLED BUFFER
            mtl_buffer =
                [m_state->mtl_device newBufferWithLength:desc.array_size
                                                 options:MTLResourceStorageModeShared];
        }

        /// 成功创建Metal Buffer
        if (mtl_buffer)
        {
            switch (desc.buffer_usage)
            {
                case DeviceBufferUsage::VERTEX_BUFFER:
                {
                    mtl_buffer.label = @"VertexBuffer";
                    break;
                }

                case DeviceBufferUsage::INDEX_BUFFER:
                {
                    mtl_buffer.label = @"IndexBuffer";
                    break;
                }

                case DeviceBufferUsage::UNIFORM_BUFFER:
                {
                    mtl_buffer.label = @"UniformBuffer";
                    break;
                }

                default:
                {
                    RUNTIME_ASSERT(false, "Unknown DeviceBufferUsage!!");
                    break;
                }
            }

            buffer_id = m_state->next_buffer_id;
            m_state->next_buffer_id.next_id();

            [m_state->buffer_table setObject:mtl_buffer
                                      forKey:[NSNumber numberWithUnsignedLongLong:
                                              (unsigned long long)buffer_id.value]];
        }
    }
    return buffer_id;
}


void
MetalRenderDevice::destroy_buffer (
    const DeviceBufferIdT buffer_id)
{
    if (buffer_id.is_valid())
    {
        [m_state->buffer_table removeObjectForKey:
            [NSNumber numberWithUnsignedLongLong:(unsigned long long)buffer_id.value]];
    }
}


DeviceTextureIdT
MetalRenderDevice::create_texture (
    const DeviceTextureDesc & desc)
{
    DeviceTextureIdT texture_id{};

    if (m_state->mtl_device && desc.texel_array && desc.array_size &&
        desc.texture_width  && desc.texture_height &&
        desc.mipmap_count   && desc.slice_count &&
        desc.mipmap_count <= TextureUtility::maximal_mipmap_count(
            desc.texture_width, desc.texture_height) &&
        (desc.is_cube_map == false ||
         (desc.slice_count == 6 &&
          desc.texture_width == desc.texture_height)))
    {
        const MTLPixelFormat pixel_format =
            MetalTextureUtility::metal_texture_format(desc.texture_format);
        const uint8_t texel_size_in_bytes =
            DeviceTextureUtility::texel_size_in_bytes(desc.texture_format);
        if (pixel_format != MTLPixelFormatInvalid && texel_size_in_bytes)
        {
            bool is_texture_data_size_valid = true;
            uint64_t required_slice_data_size = 0;
            uint32_t mipmap_width  = desc.texture_width;
            uint32_t mipmap_height = desc.texture_height;
            for (uint8_t mipmap_idx = 0;
                 mipmap_idx < desc.mipmap_count &&
                 is_texture_data_size_valid;
                 ++mipmap_idx)
            {
                const uint64_t mipmap_data_size =
                    (uint64_t)mipmap_width * mipmap_height * texel_size_in_bytes;
                if (mipmap_data_size <= desc.array_size &&
                    required_slice_data_size <=
                        desc.array_size - mipmap_data_size)
                {
                    required_slice_data_size += mipmap_data_size;
                    mipmap_width =
                        TextureUtility::next_mipmap_dimension(mipmap_width);
                    mipmap_height =
                        TextureUtility::next_mipmap_dimension(mipmap_height);
                }
                else
                {
                    is_texture_data_size_valid = false;
                }
            }

            if (is_texture_data_size_valid &&
                required_slice_data_size <=
                    std::numeric_limits<uint32_t>::max() / desc.slice_count &&
                required_slice_data_size * desc.slice_count == desc.array_size)
            {
                MTLTextureDescriptor * texture_descriptor = nil;
                if (desc.is_cube_map)
                {
                    texture_descriptor =
                        [MTLTextureDescriptor
                            textureCubeDescriptorWithPixelFormat:pixel_format
                                                            size:desc.texture_width
                                                       mipmapped:desc.mipmap_count > 1];
                }
                else
                {
                    texture_descriptor =
                        [MTLTextureDescriptor
                            texture2DDescriptorWithPixelFormat:pixel_format
                                                         width:desc.texture_width
                                                        height:desc.texture_height
                                                     mipmapped:desc.mipmap_count > 1];
                    if (desc.slice_count > 1)
                    {
                        texture_descriptor.textureType = MTLTextureType2DArray;
                        texture_descriptor.arrayLength = desc.slice_count;
                    }
                }

                if (texture_descriptor)
                {
                    texture_descriptor.mipmapLevelCount = desc.mipmap_count;
                    texture_descriptor.usage = MTLTextureUsageShaderRead;
#if (OS_TYPE == OS_TYPE_MAC)
                    texture_descriptor.storageMode = MTLStorageModeManaged;
#elif (OS_TYPE == OS_TYPE_IOS)
                    texture_descriptor.storageMode = MTLStorageModeShared;
#else
#error Not supported Platform!!
#endif

                    /// 创建Metal Texture
                    id<MTLTexture> const mtl_texture =
                        [m_state->mtl_device
                            newTextureWithDescriptor:texture_descriptor];
                    if (mtl_texture)
                    {
                        mtl_texture.label = @"SoULTexture";

                        const uint8_t * const texel_list =
                            (const uint8_t *)desc.texel_array;
                        const uint32_t slice_data_size =
                            (uint32_t)required_slice_data_size;

                        for (uint8_t slice_idx = 0;
                             slice_idx < desc.slice_count;
                             ++slice_idx)
                        {
                            uint32_t texel_list_offset =
                                slice_idx * slice_data_size;

                            mipmap_width  = desc.texture_width;
                            mipmap_height = desc.texture_height;
                            for (uint8_t mipmap_idx = 0;
                                 mipmap_idx < desc.mipmap_count;
                                 ++mipmap_idx)
                            {
                                const uint32_t bytes_per_row =
                                    mipmap_width * texel_size_in_bytes;
                                const uint32_t mipmap_data_size =
                                    bytes_per_row * mipmap_height;

                                const MTLRegion texture_region =
                                    MTLRegionMake2D(0, 0, mipmap_width, mipmap_height);
                                [mtl_texture
                                    replaceRegion:texture_region
                                      mipmapLevel:mipmap_idx
                                            slice:slice_idx
                                        withBytes:texel_list + texel_list_offset
                                      bytesPerRow:bytes_per_row
                                    bytesPerImage:mipmap_data_size];

                                texel_list_offset += mipmap_data_size;
                                mipmap_width =
                                    TextureUtility::next_mipmap_dimension(
                                        mipmap_width);
                                mipmap_height =
                                    TextureUtility::next_mipmap_dimension(
                                        mipmap_height);
                            }
                        }

                        texture_id = m_state->next_texture_id;
                        m_state->next_texture_id.next_id();
                        [m_state->texture_table
                            setObject:mtl_texture
                               forKey:[NSNumber numberWithUnsignedLongLong:
                                   (unsigned long long)texture_id.value]];
                    }
                }
            }
        }
    }

    return texture_id;
}


void
MetalRenderDevice::destroy_texture (
    const DeviceTextureIdT texture_id)
{
    if (texture_id.is_valid())
    {
        [m_state->texture_table removeObjectForKey:
            [NSNumber numberWithUnsignedLongLong:
                (unsigned long long)texture_id.value]];
    }
}


DevicePipelineIdT
MetalRenderDevice::create_pipeline (
    const DevicePipelineDesc & desc)
{
    DevicePipelineIdT pipeline_id{};

    if (m_state->mtl_device  && m_state->shader_library &&
        desc.vertex_function && desc.fragment_function  && desc.vertex_stride)
    {
        NSString * const vertex_function_name =
            [NSString stringWithUTF8String:desc.vertex_function];
        NSString * const fragment_function_name =
            [NSString stringWithUTF8String:desc.fragment_function];
        if (vertex_function_name && fragment_function_name)
        {
            id<MTLFunction> const vertex_function =
                [m_state->shader_library
                    newFunctionWithName:vertex_function_name];
            id<MTLFunction> const fragment_function =
                [m_state->shader_library
                    newFunctionWithName:fragment_function_name];
            if (vertex_function && fragment_function)
            {
                MTLVertexDescriptor * const vertex_descriptor =
                    [[MTLVertexDescriptor alloc] init];
                MTLRenderPipelineDescriptor * const pipeline_descriptor =
                    [[MTLRenderPipelineDescriptor alloc] init];
                if (vertex_descriptor && pipeline_descriptor)
                {
                    vertex_descriptor
                        .attributes[SoULMetalVertexAttributePosition].format =
                            MTLVertexFormatFloat3;
                    vertex_descriptor
                        .attributes[SoULMetalVertexAttributePosition].offset =
                            desc.position_offset;
                    vertex_descriptor
                        .attributes[SoULMetalVertexAttributePosition].bufferIndex =
                            SoULMetalBufferIndexMesh;
                    vertex_descriptor
                        .attributes[SoULMetalVertexAttributeTexcoord].format =
                            MTLVertexFormatFloat2;
                    vertex_descriptor
                        .attributes[SoULMetalVertexAttributeTexcoord].offset =
                            desc.texcoord_offset;
                    vertex_descriptor
                        .attributes[SoULMetalVertexAttributeTexcoord].bufferIndex =
                            SoULMetalBufferIndexMesh;
                    vertex_descriptor
                        .layouts[SoULMetalBufferIndexMesh].stride =
                            desc.vertex_stride;
                    vertex_descriptor
                        .layouts[SoULMetalBufferIndexMesh].stepRate = 1;
                    vertex_descriptor
                        .layouts[SoULMetalBufferIndexMesh].stepFunction =
                            MTLVertexStepFunctionPerVertex;

                    pipeline_descriptor.label = @"SoULRenderPipeline";
                    pipeline_descriptor.vertexFunction   = vertex_function;
                    pipeline_descriptor.fragmentFunction = fragment_function;
                    pipeline_descriptor.vertexDescriptor = vertex_descriptor;
                    pipeline_descriptor.sampleCount = 1;
                    pipeline_descriptor.colorAttachments[0].pixelFormat =
                        MTLPixelFormatBGRA8Unorm_sRGB;
                    pipeline_descriptor.depthAttachmentPixelFormat =
                        MTLPixelFormatDepth32Float_Stencil8;
                    pipeline_descriptor.stencilAttachmentPixelFormat =
                        MTLPixelFormatDepth32Float_Stencil8;

                    NSError * error = nil;
                    id<MTLRenderPipelineState> const pipeline_state =
                        [m_state->mtl_device
                            newRenderPipelineStateWithDescriptor:pipeline_descriptor
                                                           error:&error];
                    if (pipeline_state)
                    {
                        MTLDepthStencilDescriptor * const depth_descriptor =
                            [[MTLDepthStencilDescriptor alloc] init];
                        if (depth_descriptor)
                        {
                            depth_descriptor.depthCompareFunction =
                                MTLCompareFunctionGreater;
                            depth_descriptor.depthWriteEnabled = YES;
                            id<MTLDepthStencilState> const depth_stencil_state =
                                [m_state->mtl_device
                                    newDepthStencilStateWithDescriptor:
                                        depth_descriptor];
                            if (depth_stencil_state)
                            {
                                MetalPipelineResource * const pipeline_resource =
                                    [[MetalPipelineResource alloc] init];
                                if (pipeline_resource)
                                {
                                    pipeline_resource.pipeline_state =
                                        pipeline_state;
                                    pipeline_resource.depth_stencil_state =
                                        depth_stencil_state;

                                    pipeline_id = m_state->next_pipeline_id;
                                    m_state->next_pipeline_id.next_id();
                                    [m_state->pipeline_table
                                        setObject:pipeline_resource
                                           forKey:[NSNumber
                                               numberWithUnsignedLongLong:
                                                   (unsigned long long)
                                                       pipeline_id.value]];
                                }
                            }
                        }
                    }
                    else
                    {
                        NSLog(
                            @"Can not create SoUL Metal pipeline: %@",
                            error.localizedDescription);
                    }
                }
            }
        }
    }

    return pipeline_id;
}


void
MetalRenderDevice::destroy_pipeline (
    const DevicePipelineIdT pipeline_id)
{
    if (pipeline_id.is_valid())
    {
        [m_state->pipeline_table removeObjectForKey:
            [NSNumber numberWithUnsignedLongLong:
                (unsigned long long)pipeline_id.value]];
    }
}


bool
MetalRenderDevice::begin_frame (
    void * const native_surface)
{
    if (native_surface && m_state->mtl_device &&
        m_state->current_command_buffer == nil)
    {
        MTKView * const view = (__bridge MTKView *)native_surface;
        if (view.device == m_state->mtl_device)
        {
            dispatch_semaphore_wait(
                m_state->frame_semaphore, DISPATCH_TIME_FOREVER);

            MTLRenderPassDescriptor * const render_pass_desc =
                view.currentRenderPassDescriptor;
            id<CAMetalDrawable> const drawable = view.currentDrawable;
            if (render_pass_desc && drawable)
            {
                m_state->current_command_buffer =
                    [m_state->command_queue commandBuffer];
                if (m_state->current_command_buffer)
                {
                    m_state->current_command_buffer.label = @"FrameCommandBuffer";

                    [m_state->current_command_buffer
                        addCompletedHandler:^(id<MTLCommandBuffer> buffer)
                    {
                        dispatch_semaphore_signal(m_state->frame_semaphore);
                    }];

                    m_state->current_render_encoder =
                        [m_state->current_command_buffer
                            renderCommandEncoderWithDescriptor:render_pass_desc];
                    if (m_state->current_render_encoder)
                    {
                        m_state->current_render_encoder.label = @"GraphicRenderPass";
                        /// 左手系的Winding Order
                        [m_state->current_render_encoder
                            setFrontFacingWinding:MTLWindingClockwise];
                        [m_state->current_render_encoder setCullMode:MTLCullModeBack];
                        m_state->current_drawable = drawable;
                        return true;
                    }
                    else
                    {
                        m_state->current_command_buffer = nil;
                        dispatch_semaphore_signal(m_state->frame_semaphore);
                        return false;
                    }
                }
                else
                {
                    dispatch_semaphore_signal(m_state->frame_semaphore);
                    return false;
                }
            }
            else
            {
                dispatch_semaphore_signal(m_state->frame_semaphore);
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}


bool
MetalRenderDevice::draw_indexed (
    const DeviceDrawCall & draw_call)
{
    if (m_state->current_render_encoder && draw_call.index_count)
    {
        MetalPipelineResource * const metal_pipeline =
            m_state->pipeline(draw_call.pipeline_id);
        id<MTLBuffer> const vertex_buffer =
            m_state->buffer(draw_call.vertex_buffer_id);
        id<MTLBuffer> const index_buffer =
            m_state->buffer(draw_call.index_buffer_id);
        id<MTLTexture> const color_texture =
            m_state->texture(draw_call.texture_id);
        if (metal_pipeline && vertex_buffer && index_buffer && color_texture)
        {
            SoULMetalUniforms uniforms;
            uniforms.model_view_transform =
                row_major_matrix_to_column_major(draw_call.model_view_transform);
            uniforms.projection_transform =
                row_major_matrix_to_column_major(draw_call.projection_transform);

            [m_state->current_render_encoder
                setRenderPipelineState:metal_pipeline.pipeline_state];
            [m_state->current_render_encoder
                setDepthStencilState:metal_pipeline.depth_stencil_state];
            [m_state->current_render_encoder setVertexBuffer:vertex_buffer
                                                      offset:0
                                                     atIndex:SoULMetalBufferIndexMesh];
            [m_state->current_render_encoder
                setVertexBytes:&uniforms
                        length:sizeof(uniforms)
                       atIndex:SoULMetalBufferIndexUniforms];
            [m_state->current_render_encoder
                setFragmentTexture:color_texture
                atIndex:SoULMetalTextureIndexColor];
            [m_state->current_render_encoder
                drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                           indexCount:draw_call.index_count
                            indexType:MTLIndexTypeUInt16
                          indexBuffer:index_buffer
                    indexBufferOffset:0];
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

}


bool
MetalRenderDevice::end_frame ()
{
    if (m_state->current_render_encoder &&
        m_state->current_command_buffer &&
        m_state->current_drawable)
    {
        [m_state->current_render_encoder endEncoding];
        [m_state->current_command_buffer presentDrawable:m_state->current_drawable];
        [m_state->current_command_buffer commit];

        m_state->current_render_encoder = nil;
        m_state->current_command_buffer = nil;
        m_state->current_drawable       = nil;
        return true;
    }
    else
    {
        return false;
    }
}


MetalRenderDevice::MetalRenderDevice ()
:
    m_state(new State())
{

}


MetalRenderDevice::~MetalRenderDevice ()
{
    release_resources();
    destroy();
    delete m_state;
    m_state = nullptr;
}

#endif /// defined(__APPLE__)
