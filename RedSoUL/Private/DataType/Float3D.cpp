/// System headers
#include <cmath> /// std::sqrtf
/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Math/MathUtilities.hpp"
#include "Math/RandomNumber.hpp"
/// Self header
#include "DataType/Float3D.hpp"


const float_3 float_3::ONE     {  1,  1,  1 };
const float_3 float_3::ZERO    {  0,  0,  0 };

const float_3 float_3::FORWARD {  0,  0,  1 };
const float_3 float_3::BACKWARD{  0,  0, -1 };
const float_3 float_3::LEFT    { -1,  0,  0 };
const float_3 float_3::RIGHT   {  1,  0,  0 };
const float_3 float_3::UP      {  0,  1,  0 };
const float_3 float_3::DOWN    {  0, -1,  0 };


float_3
float_3::make (
    const float x,
    const float y,
    const float z)
{
    return float_3{ x, y, z };
}


float_3
float_3::make_random_unit_vec ()
{
    static RandomNumber generator;

    const float x = generator.next_float();
    const float y = generator.next_float();
    const float z = generator.next_float();

    return float_3{ x, y, z }.unified_vec();
}


bool
float_3::operator== (
    const float_3 vec) const
{
    return MathUtility::equal(x, vec.x) &&
           MathUtility::equal(y, vec.y) &&
           MathUtility::equal(z, vec.z);
}


bool
float_3::operator!= (
    const float_3 vec) const
{
    return !operator==(vec);
}


float
float_3::operator[] (
    const uint8_t idx) const
{
    RUNTIME_ASSERT(idx >= 0 && idx < DIMENSION, "Index is out of range");
    return e[idx];
}


float &
float_3::operator[] (
    const uint8_t idx)
{
    RUNTIME_ASSERT(idx >= 0 && idx < DIMENSION, "Index is out of range");
    return e[idx];
}


float
float_3::length () const
{
    return std::sqrtf(length_squared());
}


float
float_3::length_squared () const
{
    /// 计算Inner Product: Dot(THIS, THIS)
    return dot(*this);
}


bool
float_3::same_direction_as (
    const float_3 vec) const
{
    /// 如果与vec3d的Dot Product为正数，则同向
    /// 如果与vec3d的Dot Product为负数，则已向
    const float dot_product = dot(vec);
    return dot_product > 0;
}


bool
float_3::is_perpendicular_to (
    const float_3 vec) const
{
    /// 如果与vec的Dot Product近似为零，则垂直
    const float dot_product = dot(vec);
    return MathUtility::equal(dot_product, 0.0f);
}


bool
float_3::is_parallel_to (
    const float_3 vec) const
{
    /// 如果与vec的Cross Product的长度近似为零，则平行
    const float_3 cross_vec = cross(vec);
    return MathUtility::equal(cross_vec.length_squared(), 0.0f);
}


float_3
float_3::operator+ (
    const float_3 vec) const
{
    return float_3{ x + vec.x, y + vec.y, z + vec.z };
}


float_3
float_3::operator- (
    const float_3 vec) const
{
    return float_3{ x - vec.x, y - vec.y, z - vec.z };
}


float_3
float_3::operator* (
    const float scale) const
{
    return float_3{ x * scale, y * scale, z * scale };
}


float_3
float_3::operator/ (
    const float scale) const
{
    /// 1/scale
    /// NOTE: 如果scale为零， 1/scale为零
    const float inversed_scale = MathUtility::equal(scale, 0.0f) ? 0.0f : 1.0f / scale;
    return float_3{ inversed_scale * x, inversed_scale * y, inversed_scale * z };
}

float
float_3::dot (
    const float_3 vec) const
{
    /// v1 dot v2 = |v1| * |v2| * cos(ϕ), ϕ为v1, v2之间的夹角
    /// |v1|为v1向量的长度
    /// Dot Product的属性:
    /// - (u dot v) = (v dot u)
    /// - (s*u dot v) = s*(u dot v)
    /// - (u dot (v + w)) = (u dot v) + (u dot w)
    return (x * vec.x) + (y * vec.y) + (z * vec.z);
}


float_3
float_3::cross (
    const float_3 vec) const
{
    /// |v1 x v2| = |v1| * |v2| * sin(ϕ), ϕ为v1, v2之间的夹角
    /// |v1|为v1向量的长度
    ///
    /// 对于一个以v1, v2为边的四边形:
    ///
    ///      /
    ///     +------------+
    /// v1 /|           /
    ///   /ϕ| h        /
    ///  +--+---------+-->
    ///        v2
    /// 它的面积为: |v2| * h。此处h := |v1|*sin(ϕ)
    /// 因此这个四边形的面积为|v1 x v2|: 即: v1与v2的Cross Product的绝对值
    const float cross_x = (y * vec.z) - (z * vec.y);
    const float cross_y = (z * vec.x) - (x * vec.z);
    const float cross_z = (x * vec.y) - (y * vec.x);

    return float_3{ cross_x, cross_y, cross_z };
}


float_3
float_3::reflected_vec (
    const float_3 normal) const
{
    ///   THIS  N   R
    ///      \  |  /
    ///       \ | /
    ///        \|/
    ///   ------O------
    /// 反射向量R := 2 * Dot(THIS, N) * N - THIS
    ///
    /// 对法线向量归一化
    float_3 used_normal = normal;
    used_normal.normalize();

    /// 计算Dot(THIS, N)
    const float this_dot_n = dot(used_normal);

    return used_normal * (2 * this_dot_n) - *this;
}


float_3
float_3::unified_vec () const
{
    float_3 normalized_vec = *this;
    normalized_vec.normalize();
    return normalized_vec;
}


float_3
float_3::perpendicular_vec ()const
{
    /// NOTE: 有无穷多个向量与当前向量垂直,
    /// 我们尽量: 使Cross(Perp, THIS)指向上方
    ///
    /// 先计算一个不平行的参考轴
    const float_3 ref_vec = (std::fabsf(y) < 0.99f)
                          ? float_3{ 0, 1, 0 }
                          : float_3{ 1, 0, 0 };
    /// Cross(THIS, Ref)
    const float_3 perp_vec = cross(ref_vec);
    const float sign = (cross(perp_vec).y >= 0.0f) ? 1.0f : -1.0f;
    return perp_vec * sign;
}


void
float_3::normalize ()
{
    const float vec_length = length();

    /// 1/length
    /// NOTE: 如果向量长度为零, 1/length为零
    const float inversed_length =
        MathUtility::equal(vec_length, 0) ? 0.0f : 1.0f / vec_length;
    x *= inversed_length;
    y *= inversed_length;
    z *= inversed_length;
}
