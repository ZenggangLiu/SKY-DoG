/// System headers
#include <cmath> /// std::sqrtf
/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Math/MathUtilities.hpp"
/// Self header
#include "DataType/Float2D.hpp"


float_2
float_2::make (
    const float x,
    const float y)
{
    return float_2{ x, y };
}


bool
float_2::operator== (
    const float_2 vec) const
{
    return MathUtility::equal(x, vec.x) &&
           MathUtility::equal(y, vec.y);
}


bool
float_2::operator!= (
    const float_2 vec) const
{
    return !operator==(vec);
}


float
float_2::operator[] (
    const uint8_t idx) const
{
    RUNTIME_ASSERT(idx >= 0 && idx < DIMENSION, "Index is out of range");
    return e[idx];
}


float &
float_2::operator[] (
    const uint8_t idx)
{
    RUNTIME_ASSERT(idx >= 0 && idx < DIMENSION, "Index is out of range");
    return e[idx];
}


float
float_2::length () const
{
    return std::sqrtf(length_squared());
}


float
float_2::length_squared () const
{
    /// 计算Inner Product: Dot(THIS, THIS)
    return (x * x) + (y * y);
}
