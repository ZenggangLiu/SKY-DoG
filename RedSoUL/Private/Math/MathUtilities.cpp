/// System headers
#include <cmath> /// std::fabs, std::floor, std::sqrt
/// Library headers
#include "Assert/RuntimeAssert.hpp"
/// Self header
#include "Math/MathUtilities.hpp"


/// 要使用快速近似, 将此宏设置为1
#define USE_FAST_APPROXIMATION  1


float
MathUtility::fast_acos (
    const float cos_num)
{
    /// 保证指定Cosine值在[-1, +1]之间
    const float clamped_num = clamp(cos_num, -1.f, +1.f);

    /// 获取Cosine值的符号。负数表示角度在[π/2, π]度之间
    const bool is_neg = clamped_num < 0;

    /// 在第一象限中计算角度[0, π/2]
    const float positive_cos = std::fabs(clamped_num);
    float angle_rads = -0.0187293f;
    angle_rads = angle_rads * positive_cos + 0.0742610f;
    angle_rads = angle_rads * positive_cos - 0.2121144f;
    angle_rads = angle_rads * positive_cos + 1.5707288f;
    angle_rads = angle_rads * std::sqrt(1.f - positive_cos);

    /// 如果在第二象限: π - angle
    return is_neg ? ONE_PI - angle_rads : angle_rads;
}


float
MathUtility::fast_cosine (
    const float rads)
{
    /// COPYED: DirectX::XMScalarCos() in DirectXMath.h
    ///

    /// 将指定角度Rads映射到在[-π, +π]之间的Y值
    /// 
    /// rads = 2 * pi * quotient + remainder
    float quotient = rads / TWO_PI;
    if (rads >= 0.0f)
    {
        quotient = (float)(int32_t)(quotient + 0.5f);
    }
    else
    {
        quotient = (float)(int32_t)(quotient - 0.5f);
    }

    float y = rads - TWO_PI * quotient;

    /// 使用sin(Y) = sin(rads), 将Y映射到[-π/2, +πi/2]
    float sign;
    if (y > HALF_PI)
    {
        y = ONE_PI - y;
        sign = -1.0f;
    }
    else if (y < -HALF_PI)
    {
        y = -ONE_PI - y;
        sign = -1.0f;
    }
    else
    {
        sign = +1.0f;
    }

    float y2 = y * y;

#if (USE_FAST_APPROXIMATION == 1)
    /// 使用6次Minimax逼近
    return sign * (((-0.0012712436f * y2 + 0.041493919f) * y2 - 0.49992746f) * y2 + 1.0f);
#else
    /// 使用10次Minimax逼近
    return sign * (((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) *
                     y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f);
#endif
}


float
MathUtility::fast_sine (
    const float rads)
{
    /// COPYED: DirectX::XMScalarSin() in DirectXMath.h
    ///

    /// 将指定角度Rads映射到在[-π, +π]之间的Y值
    /// 
    /// rads = 2 * pi * quotient + remainder
    float quotient = rads / TWO_PI;
    if (rads >= 0.0f)
    {
        quotient = (float)(int32_t)(quotient + 0.5f);
    }
    else
    {
        quotient = (float)(int32_t)(quotient - 0.5f);
    }

    float y = rads - TWO_PI * quotient;

    /// 使用sin(Y) = sin(rads), 将Y映射到[-π/2, +πi/2]
    float sign;
    if (y > HALF_PI)
    {
        y = ONE_PI - y;
        sign = -1.0f;
    }
    else if (y < -HALF_PI)
    {
        y = -ONE_PI - y;
        sign = -1.0f;
    }
    else
    {
        sign = +1.0f;
    }

    float y2 = y * y;

#if (USE_FAST_APPROXIMATION == 1)
    /// 使用7次Minimax逼近
    return (((-0.00018524670f * y2 + 0.0083139502f) * y2 - 0.16665852f) * y2 + 1.0f) * y;
#else
    /// 使用11次Minimax逼近
    return (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) *
              y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;
#endif
}


void
MathUtility::fast_sincos (
    const float rads,
    float &     sin,
    float &     cos)
{
    /// COPYED: DirectX::XMScalarSinCos() in DirectXMath.h
    ///

    /// 将指定角度Rads映射到在[-π, +π]之间的Y值
    /// 
    /// rads = 2 * pi * quotient + remainder
    float quotient = rads / TWO_PI;
    if (rads >= 0.0f)
    {
        quotient = (float)(int32_t)(quotient + 0.5f);
    }
    else
    {
        quotient = (float)(int32_t)(quotient - 0.5f);
    }

    float y = rads - TWO_PI * quotient;

    /// 使用sin(Y) = sin(rads), 将Y映射到[-π/2, +πi/2]
    float sign;
    if (y > HALF_PI)
    {
        y = ONE_PI - y;
        sign = -1.0f;
    }
    else if (y < -HALF_PI)
    {
        y = -ONE_PI - y;
        sign = -1.0f;
    }
    else
    {
        sign = +1.0f;
    }

    float y2 = y * y;

#if (USE_FAST_APPROXIMATION == 1)
    /// 使用7次Minimax逼近
    sin = (((-0.00018524670f * y2 + 0.0083139502f) * y2 - 0.16665852f) * y2 + 1.0f) * y;

    /// 使用6次Minimax逼近
    cos = sign * (((-0.0012712436f * y2 + 0.041493919f) * y2 - 0.49992746f) * y2 + 1.0f);
#else
    /// 使用11次Minimax逼近
    sin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

    /// 使用10次Minimax逼近
    cos = sign * (((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f);
#endif
}


bool
MathUtility::equal (
    const float a,
    const float b,
    const float epsilon /* = EPSILON */)
{
    /// 参考:
    /// - Real Time Collision Detection
    /// 11.3.1: Tolerance Comparison for Floating-point Values
    ///
    /// 我们使用如下方式比较两个浮点数a, b:
    /// 1) 绝对tolerance比较: abs(a - b) <= tolerence
    ///    这种比较方式需要a与b中有一定的digit匹配，才能够认为他们相等。
    ///    但是当a与b越来越大的时候，这种比较几乎相当于比较所有a与b的digits是否相同
    /// 2) 相对tolerance比较:  abs(a/b - 1) <= tolerence, where |a| <= |b|
    ///    这个比较关系可以写为: abs(a - b) <= tolerence * abs(b)
    ///    如果忽略|a| <= |b|的条件:
    ///    + 我们可以获得: abs(a - b) <= tolerence * max(abs(a), abs(b))
    ///      这个比较公式只在abs(a)以及abs(b)都大于1的情况下成立
    ///
    /// 所以: absolute(a - b) <= tolerence * max[max(abs(a), abs(b)), 1]
    /// - 当数值小于1的时候，使用绝对比较: abs(a - b) <= tolerence * 1
    /// - 当数值大于1的时候，使用相对比较: abs(a - b) <= tolerence * max(abs(a), abs(b))
    /// NOTE:
    /// 使用如下近似无法实现1: 绝对tolerance比较
    /// absolute(a - b) <= tolerence * (absolute(a) + absolute(b) + 1)
    ///
    const float _max_a_b = std::max(std::fabs(a), std::fabs(b));
    return std::fabs(a - b) <= epsilon * std::max(_max_a_b, 1.f);
}


bool
MathUtility::not_equal (
    const float a,
    const float b,
    const float epsilon /* = EPSILON */)
{
    return equal(a, b, epsilon) == false;
}


float
MathUtility::repeat (
    const float value,
    const float length)
{
    return value - std::floor(value / length) * length;
}


float
MathUtility::smooth_step (
    const float value,
    const float left_edge,
    const float right_edge)
{
    RUNTIME_ASSERT(right_edge >= left_edge,
                   "Right edge must be greater than or equal to left edge!!");

    /// 参考:https://en.wikipedia.org/wiki/Smoothstep
    /// Hermit Interpolation: S(x) = 3x^2 - 2x^3 = x * x * (3 - 2*x)
    /// - 如果x∈[0, 1], 那么S(x)∈[0, 1]
    ///
    const float x = clamp((value - left_edge) / (right_edge - left_edge), 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}
