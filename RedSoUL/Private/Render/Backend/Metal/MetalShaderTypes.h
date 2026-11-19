#ifndef SOUL_METAL_SHADER_TYPES_H
#define SOUL_METAL_SHADER_TYPES_H


#ifdef __METAL_VERSION__
#define NS_ENUM(_type, _name) enum _name : _type _name; enum _name : _type
#define NSInteger metal::int32_t
#else
#import <Foundation/Foundation.h>
#endif

#include <simd/simd.h>


typedef NS_ENUM(NSInteger, SoULMetalBufferIndex)
{
    SoULMetalBufferIndexMesh     = 0,
    SoULMetalBufferIndexUniforms = 2,
};


typedef NS_ENUM(NSInteger, SoULMetalVertexAttribute)
{
    SoULMetalVertexAttributePosition = 0,
    SoULMetalVertexAttributeTexcoord = 1,
};


typedef NS_ENUM(NSInteger, SoULMetalTextureIndex)
{
    SoULMetalTextureIndexColor = 0,
};


typedef struct
{
    /// matrix_float4x4: Column Major
    matrix_float4x4 projection_transform;
    matrix_float4x4 model_view_transform;
} SoULMetalUniforms;


#endif /// SOUL_METAL_SHADER_TYPES_H
