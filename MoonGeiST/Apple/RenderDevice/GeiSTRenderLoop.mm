#import <simd/simd.h>
#import <ModelIO/ModelIO.h>

#import "RenderDevice/GeiSTRenderLoop.hpp"
#import "Shader/ShaderTypes.h" /// Shader Types(在Metal, C++代码间共享)


static const NSUInteger MaxBuffersInFlight = 3;



// *************************************************
// *                                               *
// *                    Helper                     *
// *                                               *
// *************************************************
//
// MARK: == Helper: Matrix Math Utilities ==
static
matrix_float4x4
matrix4x4_translation (
    const float tx,
    const float ty,
    const float tz)
{
    return (matrix_float4x4)
    {
        {
            { 1,   0,  0,  0 },
            { 0,   1,  0,  0 },
            { 0,   0,  1,  0 },
            { tx, ty, tz,  1 }
        }

    };
}


static
matrix_float4x4
matrix4x4_rotation (
    const float         radians,
    const vector_float3 axis)
{
    const vector_float3 unified_axis = vector_normalize(axis);
    const float ct = cosf(radians);
    const float st = sinf(radians);
    const float ci = 1 - ct;
    const float x = unified_axis.x;
    const float y = unified_axis.y;
    const float z = unified_axis.z;

    return (matrix_float4x4)
    {
        {
            { ct + x * x * ci,     y * x * ci + z * st, z * x * ci - y * st, 0},
            { x * y * ci - z * st,     ct + y * y * ci, z * y * ci + x * st, 0},
            { x * z * ci + y * st, y * z * ci - x * st,     ct + z * z * ci, 0},
            {                   0,                   0,                   0, 1}
        }
    };
}


static
matrix_float4x4
matrix_perspective_right_hand (
    const float fovyRadians,
    const float aspect,
    const float nearZ,
    const float farZ)
{
    const float ys = 1 / tanf(fovyRadians * 0.5);
    const float xs = ys / aspect;
    const float zs = farZ / (nearZ - farZ);

    return (matrix_float4x4)
    {
        {
            { xs,   0,          0,  0 },
            {  0,  ys,          0,  0 },
            {  0,   0,         zs, -1 },
            {  0,   0, nearZ * zs,  0 }
        }
    };
}



@implementation Renderer
{
    dispatch_semaphore_t _inFlightSemaphore;
    id <MTLDevice> _device;
    id <MTLCommandQueue> _commandQueue;

    id <MTLBuffer> _uniform_buffer[MaxBuffersInFlight];
    id <MTLRenderPipelineState> _pipelineState;
    id <MTLDepthStencilState> _depthState;
    id <MTLTexture> _colorMap;
    MTLVertexDescriptor *_vertex_desc;

    /// buffer索引
    uint8_t _uniformBufferIndex;

    matrix_float4x4 _projectionMatrix;

    float _rotation;

    MTKMesh *_mesh;
}


-(nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view;
{
    self = [super init];
    if(self)
    {
        /// 保存Metal Device
        _device = view.device;
        /// 创建Semaphore(3个Buffer)
        _inFlightSemaphore = dispatch_semaphore_create(MaxBuffersInFlight);

        [self _initalizeMetalResources:view];
        [self _loadGameAssets];
    }

    return self;
}


- (void)_initalizeMetalResources:(nonnull MTKView *)view;
{
    /// Load Metal state objects and initialize renderer dependent view properties

    /// 设置CAMetalLayer
    view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    view.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB; /// CAMetalLayer的原生格式为BGRA
    view.sampleCount = 1;

    // === 设置顶点数据 === //
    _vertex_desc = [[MTLVertexDescriptor alloc] init];

    /// 设置顶点的Pos定义: 使用BufferIndexMeshPositions(0) Buffer传输
    _vertex_desc.attributes[VertexAttributePosition].format = MTLVertexFormatFloat3;
    _vertex_desc.attributes[VertexAttributePosition].offset = 0;
    _vertex_desc.attributes[VertexAttributePosition].bufferIndex =
        BufferIndexMeshPositions;

    _vertex_desc.layouts[BufferIndexMeshPositions].stride = 12;
    _vertex_desc.layouts[BufferIndexMeshPositions].stepRate = 1;
    _vertex_desc.layouts[BufferIndexMeshPositions].stepFunction =
        MTLVertexStepFunctionPerVertex;

    /// 设置顶点的UV定义: 使用BufferIndexMeshGenerics(1) Buffer传输
    _vertex_desc.attributes[VertexAttributeTexcoord].format = MTLVertexFormatFloat2;
    _vertex_desc.attributes[VertexAttributeTexcoord].offset = 0;
    _vertex_desc.attributes[VertexAttributeTexcoord].bufferIndex =
        BufferIndexMeshGenerics;

    _vertex_desc.layouts[BufferIndexMeshGenerics].stride = 8;
    _vertex_desc.layouts[BufferIndexMeshGenerics].stepRate = 1;
    _vertex_desc.layouts[BufferIndexMeshGenerics].stepFunction =
        MTLVertexStepFunctionPerVertex;

    // === 加载Shader === //
    id<MTLLibrary> shader_lib = [_device newDefaultLibrary];

    id <MTLFunction> vertex_shader = [shader_lib newFunctionWithName:@"vertexShader"];
    id <MTLFunction> pixel_shader  = [shader_lib newFunctionWithName:@"fragmentShader"];

    // === 创建PipeLine === //
    MTLRenderPipelineDescriptor *pipeline_state_desc =
        [[MTLRenderPipelineDescriptor alloc] init];
    pipeline_state_desc.label = @"MyPipeline";
    pipeline_state_desc.sampleCount = view.sampleCount;
    pipeline_state_desc.vertexFunction = vertex_shader;
    pipeline_state_desc.fragmentFunction = pixel_shader;
    pipeline_state_desc.vertexDescriptor = _vertex_desc;
    pipeline_state_desc.colorAttachments[0].pixelFormat = view.colorPixelFormat;
    pipeline_state_desc.depthAttachmentPixelFormat = view.depthStencilPixelFormat;
    pipeline_state_desc.stencilAttachmentPixelFormat = view.depthStencilPixelFormat;

    NSError * error = NULL;
    _pipelineState = [_device newRenderPipelineStateWithDescriptor:pipeline_state_desc
                                                             error:&error];
    if (!_pipelineState)
    {
        NSLog(@"Failed to created pipeline state, error %@", error);
    }

    MTLDepthStencilDescriptor *depthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
    depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
    depthStateDesc.depthWriteEnabled = YES;
    _depthState = [_device newDepthStencilStateWithDescriptor:depthStateDesc];

    /// 创建UniformBuffer(三个): Model + Projection Matrix
    for(NSUInteger i = 0; i < MaxBuffersInFlight; i++)
    {
        _uniform_buffer[i] =
            [_device newBufferWithLength:sizeof(Uniforms)
                                 options:MTLResourceStorageModeShared];
        _uniform_buffer[i].label = @"UniformBuffer";
    }

    _commandQueue = [_device newCommandQueue];
}


- (void)_loadGameAssets
{
    /// Load assets into metal objects

    NSError *error;

    MTKMeshBufferAllocator *metalAllocator = [[MTKMeshBufferAllocator alloc]
                                              initWithDevice: _device];

    MDLMesh *mdlMesh = [MDLMesh newBoxWithDimensions:(vector_float3){4, 4, 4}
                                            segments:(vector_uint3){2, 2, 2}
                                        geometryType:MDLGeometryTypeTriangles
                                       inwardNormals:NO
                                           allocator:metalAllocator];

    MDLVertexDescriptor *mdlVertexDescriptor =
    MTKModelIOVertexDescriptorFromMetal(_vertex_desc);

    mdlVertexDescriptor.attributes[VertexAttributePosition].name  = MDLVertexAttributePosition;
    mdlVertexDescriptor.attributes[VertexAttributeTexcoord].name  = MDLVertexAttributeTextureCoordinate;

    mdlMesh.vertexDescriptor = mdlVertexDescriptor;

    _mesh = [[MTKMesh alloc] initWithMesh:mdlMesh
                                   device:_device
                                    error:&error];

    if(!_mesh || error)
    {
        NSLog(@"Error creating MetalKit mesh %@", error.localizedDescription);
    }

    // 加载纹理
    MTKTextureLoader* textureLoader = [[MTKTextureLoader alloc] initWithDevice:_device];

    NSDictionary *textureLoaderOptions =
    @{
      MTKTextureLoaderOptionTextureUsage       : @(MTLTextureUsageShaderRead),
      MTKTextureLoaderOptionTextureStorageMode : @(MTLStorageModePrivate)
      };

    // 同步加载一个图形: 返回 id<MTLTexture>
    _colorMap = [textureLoader newTextureWithName:@"ColorMap"   // Asset Catalog中的图形文件
                                      scaleFactor:1.0
                                           bundle:nil           // Resource Bundle
                                          options:textureLoaderOptions
                                            error:&error];
    if(!_colorMap || error)
    {
        NSLog(@"Error creating texture %@", error.localizedDescription);
    }
}


/// Redraw中调用: drawInMTKView
- (void)_updateUniforms
{
    /// 获取MTLBuffer的系统内存地址: MTLBuffer::contents
    Uniforms * const uniforms = (Uniforms*)_uniform_buffer[_uniformBufferIndex].contents;

    uniforms->projectionMatrix = _projectionMatrix;

    /// 生成本地旋转矩阵
    vector_float3 rotationAxis = {1, 1, 0};
    matrix_float4x4 modelMatrix = matrix4x4_rotation(_rotation, rotationAxis);
    /// 生成照相机矩阵
    matrix_float4x4 viewMatrix = matrix4x4_translation(0.0, 0.0, -8.0);
    /// 合成Model View矩阵
    uniforms->modelViewMatrix = matrix_multiply(viewMatrix, modelMatrix);
}


- (void)_updateGameState
{
    /// 旋转0.01弧度(0.572958度)
    _rotation += .01;
}


- (void)drawInMTKView:(nonnull MTKView *)view
{
    // === PER FRAME UPDATE === //

    /// Semaphore的计数减一: 目前有3个Buffer, 如果耗尽则永久等待DISPATCH_TIME_FOREVER
    dispatch_semaphore_wait(_inFlightSemaphore, DISPATCH_TIME_FOREVER);

    _uniformBufferIndex = (_uniformBufferIndex + 1) % MaxBuffersInFlight;

    /// 创建CommandBuffer
    id <MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    commandBuffer.label = @"MyCommand";

    /// __block: 表示Block中可以修改此变量
    __block dispatch_semaphore_t block_sema = _inFlightSemaphore;
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer)
     {
        /// CommandBuffer执行完成, 是否一个Semophore
        dispatch_semaphore_signal(block_sema);
    }];

    /// 更新Uniform + 旋转状态
    [self _updateUniforms];
    [self _updateGameState];

    /// Delay getting the currentRenderPassDescriptor until absolutely needed. This avoids
    ///   holding onto the drawable and blocking the display pipeline any longer than necessary
    /// 创建一个新的Render Pass Descriptor(随后的Render Command Encoder使用):
    MTLRenderPassDescriptor* render_pass_desc = view.currentRenderPassDescriptor;
    if(render_pass_desc)
    {
        /// Final pass rendering code here

        /// 在新创建的RenderPassDescriptor中创建RenderEncoder
        id <MTLRenderCommandEncoder> renderEncoder =
        [commandBuffer renderCommandEncoderWithDescriptor:render_pass_desc];
        renderEncoder.label = @"MyRenderEncoder";

        ///    |MTLCommandEncoder|
        ///           ^
        ///           | 继承
        /// |MTLRenderCommandEncoder|
        ///
        /// 使用MTLCommandEncoder的函数推入一个新的Debug Group到STACK
        [renderEncoder pushDebugGroup:@"DrawCommands"];

        /// 正面: 逆时针方向
        [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];

        /// CULL背面
        [renderEncoder setCullMode:MTLCullModeBack];
        [renderEncoder setRenderPipelineState:_pipelineState];
        [renderEncoder setDepthStencilState:_depthState];

        // === 设置Vertex Shader的Buffer === //
        /// 设置Uniform Buffer
        [renderEncoder setVertexBuffer:_uniform_buffer[_uniformBufferIndex]
                                offset:0 // buffer offset
                               atIndex:BufferIndexUniforms]; // 2

        /// 设置Vertex Buffer: Mesh中定义的个数
        for (NSUInteger bufferIndex = 0; bufferIndex < _mesh.vertexBuffers.count; bufferIndex++)
        {
            MTKMeshBuffer *vertexBuffer = _mesh.vertexBuffers[bufferIndex];
            if((NSNull*)vertexBuffer != [NSNull null])
            {
                [renderEncoder setVertexBuffer:vertexBuffer.buffer
                                        offset:vertexBuffer.offset
                                       atIndex:bufferIndex];
            }
        }

        /// 设置Texture
        [renderEncoder setFragmentTexture:_colorMap
                                  atIndex:TextureIndexColor]; // 0

        /// 逐个绘制Mesh中的所有Sub-Mesh
        for(MTKSubmesh *submesh in _mesh.submeshes)
        {
            [renderEncoder drawIndexedPrimitives:submesh.primitiveType
                                      indexCount:submesh.indexCount
                                       indexType:submesh.indexType
                                     indexBuffer:submesh.indexBuffer.buffer
                               indexBufferOffset:submesh.indexBuffer.offset];
        }

        /// 从STACK中弹出当前Debug Group: "DrawCommands"
        [renderEncoder popDebugGroup];
        [renderEncoder endEncoding];

        /// Present指令(在Commit前的最后一条指令)
        [commandBuffer presentDrawable:view.currentDrawable];
    }

    [commandBuffer commit];
}


- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
    /// 重新计算Aspect Ratio，已经生成投影矩阵
    float aspect = size.width / (float)size.height;
    _projectionMatrix = matrix_perspective_right_hand(65.0f * (M_PI / 180.0f), aspect, 0.1f, 100.0f);
}

@end
