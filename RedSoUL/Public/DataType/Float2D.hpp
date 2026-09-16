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
    Created:  28/03/26  @  5:17 PM
    FileName: Float2D.hpp @ RedSoUL Project
    History:
             - created by: 28/03/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint32_t
#include <type_traits>


/// 二维向量
///
/// NOTE:
/// float_2定义为POD/Aggregate类型：8位字节对齐
/// - NO constructor, NO copy constructor, NO operator=(),
///   NO protect/private data, NO base class, NO virtual functions
/// - 缺省的opeartor=()将使用std::memcpy()逐字节复制：
///   例如：
///   float_2 source;
///   float_2 copy = source; <-- 使用std::memcpy()进行复制, 内部使用SSE
///
struct alignas(8) float_2
{
    /// 纬度
    static constexpr uint8_t DIMENSION = 2;

    union
    {
        struct
        {
            float x;
            float y;
        };
        float e[DIMENSION];
    };

    /// 使用指定的数值创建向量: [x, y]
    static
    float_2
    make (
        const float x,
        const float y);

    /// 检测当前向量是否与另一个向量vec相同
    bool
    operator== (
        const float_2 vec) const;

    /// 检测当前向量是否与另一个向量vec不同
    bool
    operator!= (
        const float_2 vec) const;

    /// 获得指定索引成员的数值
    ///
    /// 例如:
    /// const float_2 vec2d;
    /// const float fval = vec2d[1];
    float
    operator[] (
        const uint8_t idx) const;

    /// 设定指定索引成员
    ///
    /// 例如:
    /// float_2 vec2d;
    /// vec2d[1] = 3.14f;
    float &
    operator[] (
        const uint8_t idx);

    /// 计算当前向量的长度/Norm
    float
    length () const;

    /// 计算当前向量的长度的平方/Norm^2
    float
    length_squared () const;
  };
static_assert(std::is_trivial<float_2>::value &&
              std::is_standard_layout<float_2>::value,
              "float_2 is Not a POD type!!");
