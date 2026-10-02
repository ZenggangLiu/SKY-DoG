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
    Created:  21/03/26  @  7:37 PM
    FileName: Float4D.hpp @ RedSoUL Project
    History:
             - created by: 21/03/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint32_t
#include <type_traits>
/// Library headers
#include "DataType/Float3D.hpp"


/// 四维向量
///
/// NOTE:
/// float_4定义为POD/Aggregate类型: 16位字节对齐, 以支持SIMD运算
/// - NO constructor, NO copy constructor, NO operator=(),
///   NO protect/private data, NO base class, NO virtual functions
/// - 缺省的opeartor=()将使用std::memcpy()逐字节复制:
///   例如:
///   float_4 source;
///   float_4 copy = source; <-- 使用std::memcpy()进行复制, 内部使用SSE
///
struct alignas(16) float_4
{
    /// 纬度
    static constexpr uint8_t DIMENSION = 4;

    /// 全一向量
    static const float_4 ONE;  /// [  1,  1,  1, 1 ]
    /// 全零向量
    static const float_4 ZERO; /// [  0,  0,  0, 0 ]

    union
    {
        struct
        {
            float x;
            float y;
            float z;
            float w;
        };
        float e[DIMENSION];
    };

    /// 使用指定的数值创建向量: [x, y, z, w]
    static
    float_4
    make (
        const float x,
        const float y,
        const float z,
        const float w);

    /// 创建三维点对应的齐次坐标: [x, y, z, 1]
    static
    float_4
    make_from_point (
        const float_3 pnt);

    /// 创建三维向量对应的齐次坐标: [x, y, z, 0]
    static
    float_4
    make_from_vector (
        const float_3 vec);

    /// 检测当前向量是否与另一个向量vec相同
    bool
    operator== (
        const float_4 vec) const;

    /// 检测当前向量是否与另一个向量vec不同
    bool
    operator!= (
        const float_4 vec) const;

    /// 获得指定索引成员的数值
    ///
    /// 例如:
    /// const float_4 vec4d;
    /// const float fval = vec4d[1];
    float
    operator[] (
        const uint8_t idx) const;

    /// 设定指定索引成员
    ///
    /// 例如:
    /// float_4 vec4d;
    /// vec4d[1] = 3.14f;
    float &
    operator[] (
        const uint8_t idx);

    /// 计算当前向量的长度/Norm
    float
    length () const;

    /// 计算当前向量的长度的平方/Norm^2
    float
    length_squared () const;

    /// 创建一个将当前向量均匀放大scale倍的向量
    /// c = THIS * scale
    float_4
    operator* (
        const float scale) const;

    /// 计算当前向量另一个向量vec的Dot/Inner Product
    /// - Dot(THIS, vec)
    float
    dot (
        const float_4 vec) const;

    /// 获得当前向量归一化后的向量
    float_4
    unified_vec () const;

    /// 归一化当前向量
    void
    normalize ();
};
static_assert(std::is_trivial<float_4>::value &&
              std::is_standard_layout<float_4>::value,
              "float_4 is Not a POD type!!");
