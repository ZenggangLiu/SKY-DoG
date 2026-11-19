#include <metal_stdlib>
#include <simd/simd.h>
#include "MetalShaderTypes.h"


using namespace metal;


struct SoULVertexInput
{
    float3 position [[attribute(SoULMetalVertexAttributePosition)]];
    float2 texcoord [[attribute(SoULMetalVertexAttributeTexcoord)]];
};


struct SoULVertexOutput
{
    float4 position [[position]];
    float2 texcoord;
};


vertex
SoULVertexOutput
soul_vertex_shader (
    const SoULVertexInput        input    [[stage_in]],
    constant SoULMetalUniforms & uniforms [[buffer(SoULMetalBufferIndexUniforms)]])
{
    SoULVertexOutput output;
    output.position = uniforms.projection_transform *
                      uniforms.model_view_transform *
                      float4(input.position, 1.0f);
    output.texcoord = input.texcoord;
    return output;
}


fragment
float4
soul_fragment_shader (
    SoULVertexOutput input         [[stage_in]],
    texture2d<float> color_texture [[texture(SoULMetalTextureIndexColor)]])
{
    constexpr sampler color_sampler(
        mip_filter::linear,
        mag_filter::linear,
        min_filter::linear);
    return color_texture.sample(color_sampler, input.texcoord);
}
