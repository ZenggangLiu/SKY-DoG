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
    Created:  15/02/26  @  4:38 PM
    FileName: MathUtilities.hpp @ RedSoUL Project
    History:
             - created by: 15/02/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <algorithm> /// std::min, std::max
#include <cstring>   /// std::memcpy
/// Library headers
#include "Common/CommonDefines.hpp" /// INLINE_FUNCTION
#include "Math/MathDefines.hpp"     /// EPSILON


// MARK: == 函数定义 ==
struct MathUtility
{
    /// 获得指定32位浮点数的Bits
    INLINE_FUNCTION
    static
    uint32_t
    bits_from_float32 (
        const float float_value);

    /// 使用Bits创建一个32位浮点数
    INLINE_FUNCTION
    static
    float
    float32_from_bits (
        const uint32_t f32_bits);

    /// 近似计算Arc-Cosine
    ///
    /// @param[in]  cos_num
    ///     Cosine的数值
    /// @return
    ///     对应的角度(以弧度衡量), 角度值在[0, π]之间
    /// NOTE: 误差为大约0.001弧度
    static
    float
    fast_acos (
        const float cos_num);

    /// 近似计算Cosine
    ///
    /// @param[in]  rads
    ///     计算的角度(以弧度衡量)
    /// NOTE: 误差小于0.00001
    static
    float
    fast_cosine (
        const float rads);

    /// 近似计算Sine
    ///
    /// @param[in]  rads
    ///     计算的角度(以弧度衡量)
    /// NOTE: 误差小于0.00001
    static
    float
    fast_sine (
        const float rads);

    /// 同时近似计算Sine与Cosine
    ///
    /// @param[in]  rads
    ///     输入角度(以弧度衡量)
    /// @param[out] sin
    ///     角度对应的Sine值
    /// @param[out] cos
    ///     角度对应的Cosine值
    /// NOTE: 误差小于0.00001
    static
    void
    fast_sincos (
        const float rads,
        float &     sin,
        float &     cos);

    /// Clamp指定的数值到指定的区间: [min, max]
    template < typename T >
    INLINE_FUNCTION
    static
    T
    clamp (
       const T val,
       const T min,
       const T max);

    /// 计算A最少是B的多少个整数倍
    /// 例如:
    /// - Math::round_up_count( 3, 2) == 2
    /// - Math::round_up_count( 5, 2) == 3
    /// - Math::round_up_count(-3, 2) == -1
    template < typename T >
    INLINE_FUNCTION
    static
    T
    round_up_count (
        const T a,
        const T b);

    /// 判断是否两个浮点数相等(近似相等)
    static
    bool
    equal (
        const float a,
        const float b,
        const float epsilon = EPSILON);

    /// 判断是否两个浮点数不等(近似不等)
    static
    bool
    not_equal (
        const float a,
        const float b,
        const float epsilon = EPSILON);

    /// 在[0, length-1]之内REPEAT指定数值value
    static
    float
    repeat (
        const float value,
        const float length);

    /// 将指定数值value映射到[0, 1]之间
    ///
    /// @return
    /// 0: 如果value <= left_edge
    /// 1: 如果value >= right_edge
    static
    float
    smooth_step (
        const float value,
        const float left_edge,
        const float right_edge);
};


// MARK: == 函数实现 ==
INLINE_FUNCTION
uint32_t
MathUtility::bits_from_float32 (
    const float float_value)
{
    /// 现代C++标准不允许:
    /// - CASTING: 从一个类型Pointer到另一个类型Pointer。例如pointer(float) to point(uint32_t)
    /// - READING: 从UNION中读取一个成员, 如果其它成员已赋值
    /// 使用std::memcpy()
    ///
    uint32_t f32_bits;
    std::memcpy(&f32_bits, &float_value, sizeof(float_value));
    return f32_bits;
}


INLINE_FUNCTION
float
MathUtility::float32_from_bits (
    const uint32_t f32_bits)
{
    /// 现代C++标准不允许:
    /// - CASTING: 从一个类型Pointer到另一个类型Pointer。例如pointer(float) to point(uint32_t)
    /// - READING: 从UNION中读取一个成员, 如果其它成员已赋值
    /// 使用std::memcpy()
    ///
    float float_value;
    std::memcpy(&float_value, &f32_bits, sizeof(f32_bits));
    return float_value;
}


template < typename T >
INLINE_FUNCTION
T
MathUtility::clamp (
    const T val,
    const T min,
    const T max)
{
    return std::min(std::max(val, min), max);
}


/// 整数类型的实现
template < typename T >
INLINE_FUNCTION
T
MathUtility::round_up_count(
    const T a,
    const T b)
{
    /// (a + b-1)/b = (a-1)/b + 1
    return (a - (T)1)/b + (T)1;
}

/// round_up_count不可用于浮点数
template <>
INLINE_FUNCTION float MathUtility::round_up_count(const float, const float);
template <>
INLINE_FUNCTION double MathUtility::round_up_count(const double, const double);
