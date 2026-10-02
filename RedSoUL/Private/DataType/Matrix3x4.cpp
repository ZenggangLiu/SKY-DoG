/// System headers
#include <cmath> /// std::sqrtf
/// Library headers
#include "Common/CompilerDefines.hpp" /// BUILD_MODE
#include "Assert/RuntimeAssert.hpp"
#include "Math/MathUtilities.hpp"
/// Self header
#include "DataType/Matrix3x4.hpp"


const matrix_3x4 matrix_3x4::IDENTITY
{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0
};


matrix_3x4
matrix_3x4::make_translation (
    const float_3 translation)
{
    const float x = translation.x;
    const float y = translation.y;
    const float z = translation.z;
    return matrix_3x4
    {
        1, 0, 0, x,
        0, 1, 0, y,
        0, 0, 1, z
    };
}


matrix_3x4
matrix_3x4::make_rotation (
    const quaternion rotation)
{
    /// 旋转矩阵为:
    ///      | 1 - 2*y*y - 2*z*z  2*x*y - 2*z*w      2*x*z + 2*y*w      0 |
    ///  R = | 2*x*y + 2*z*w      1 - 2*x*x - 2*z*z  2*y*z - 2*x*w      0 |
    ///      | 2*x*z - 2*y*w      2*y*z + 2*x*w      1 - 2*x*x - 2*y*y  0 |
    ///
    const float xx2 = 2 * rotation.x * rotation.x;
    const float yy2 = 2 * rotation.y * rotation.y;
    const float zz2 = 2 * rotation.z * rotation.z;
    const float xy2 = 2 * rotation.x * rotation.y;
    const float xz2 = 2 * rotation.x * rotation.z;
    const float xw2 = 2 * rotation.x * rotation.w;
    const float yz2 = 2 * rotation.y * rotation.z;
    const float yw2 = 2 * rotation.y * rotation.w;
    const float zw2 = 2 * rotation.z * rotation.w;

    return matrix_3x4
    {
        1 - yy2 - zz2,  xy2 - zw2,      xz2 + yw2,      0,
        xy2 + zw2,      1 - xx2 - zz2,  yz2 - xw2,      0,
        xz2 - yw2,      yz2 + xw2,      1 - xx2 - yy2,  0
    };
}


matrix_3x4
matrix_3x4::make_scaling (
    const float scaling)
{
    const float s = scaling;
    return matrix_3x4
    {
        s, 0, 0, 0,
        0, s, 0, 0,
        0, 0, s, 0
    };
}


matrix_3x4
matrix_3x4::make_scaling (
    const float_3 scaling)
{
    const float sx = scaling.x;
    const float sy = scaling.y;
    const float sz = scaling.z;
    return matrix_3x4
    {
        sx, 0,  0,  0,
        0,  sy, 0,  0,
        0,  0,  sz, 0
    };
}


matrix_3x4
matrix_3x4::make_translation_rotation (
    const float_3    translation,
    const quaternion rotation)
{
    ///       TR Matrix        =   Translation   *      Rotation
    /// | e00  e01  e02  e03 |   | 1  0  0  Tx |   | Ux  Vx  Wx  0 |
    /// | e10  e11  e12  e13 | = | 0  1  0  Ty | * | Uy  Vy  Wy  0 |
    /// | e20  e21  e22  e23 |   | 0  0  1  Tz |   | Uz  Vz  Wz  0 |
    ///
    /// | e00  e01  e02  e03 |   | Ux  Vx  Wx  Tx |
    /// | e10  e11  e12  e13 | = | Uy  Vy  Wy  Ty |
    /// | e20  e21  e22  e23 |   | Uz  Vz  Wz  Tz |

    /// 先创建旋转矩阵
    matrix_3x4 combined_matrix = make_rotation(rotation);
    /// 存储位移
    combined_matrix.e03 = translation.x;
    combined_matrix.e13 = translation.y;
    combined_matrix.e23 = translation.z;
    return combined_matrix;
}


matrix_3x4
matrix_3x4::make_translation_scaling (
    const float_3 translation,
    const float   scaling)
{
    ///       TS Matrix        =   Translation   *    Scaling
    /// | e00  e01  e02  e03 |   | 1  0  0  Tx |   | S  0  0  0 |
    /// | e10  e11  e12  e13 | = | 0  1  0  Ty | * | 0  S  0  0 |
    /// | e20  e21  e22  e23 |   | 0  0  1  Tz |   | 0  0  S  0 |
    ///
    /// | e00  e01  e02  e03 |   | S  0  0  Tx |
    /// | e10  e11  e12  e13 | = | 0  S  0  Ty |
    /// | e20  e21  e22  e23 |   | 0  0  S  Tz |

    const float   s = scaling;
    const float_3 t = translation;
    return matrix_3x4
    {
        s, 0, 0, t.x,
        0, s, 0, t.y,
        0, 0, s, t.z
    };
}


matrix_3x4
matrix_3x4::make_translation_scaling (
    const float_3 translation,
    const float_3 scaling)
{
    ///       TS Matrix        =   Translation   *      Scaling
    /// | e00  e01  e02  e03 |   | 1  0  0  Tx |   | Sx  0   0   0 |
    /// | e10  e11  e12  e13 | = | 0  1  0  Ty | * | 0   Sy  0   0 |
    /// | e20  e21  e22  e23 |   | 0  0  1  Tz |   | 0   0   Sz  0 |
    ///
    /// | e00  e01  e02  e03 |   | Sx  0   0   Tx |
    /// | e10  e11  e12  e13 | = | 0   Sy  0   Ty |
    /// | e20  e21  e22  e23 |   | 0   0   Sz  Tz |

    const float_3 s = scaling;
    const float_3 t = translation;
    return matrix_3x4
    {
        s.x, 0,   0,   t.x,
        0,   s.y, 0,   t.y,
        0,   0,   s.z, t.z
    };
}


matrix_3x4
matrix_3x4::make_rotation_scaling (
    const quaternion rotation,
    const float      scaling)
{
    ///       RS Matrix        =     Rotation      *     Scaling
    /// | e00  e01  e02  e03 |   | Ux  Vx  Wx  0 |   | s  0  0  0 |
    /// | e10  e11  e12  e13 | = | Uy  Vy  Wy  0 | * | 0  s  0  0 |
    /// | e20  e21  e22  e23 |   | Uz  Vz  Wz  0 |   | 0  0  s  0 |
    ///
    /// | e00  e01  e02  e03 |   | s*Ux  s*Vx  s*Wx  0 |
    /// | e10  e11  e12  e13 | = | s*Uy  s*Vy  s*Wy  0 |
    /// | e20  e21  e22  e23 |   | s*Uz  s*Vz  s*Wz  0 |

    /// 先创建旋转矩阵
    matrix_3x4 combined_matrix = make_rotation(rotation);

    // 将旋转矩阵每行进行放缩
    const float s = scaling;
    combined_matrix.e00 *= s; combined_matrix.e01 *= s; combined_matrix.e02 *= s;
    combined_matrix.e10 *= s; combined_matrix.e11 *= s; combined_matrix.e12 *= s;
    combined_matrix.e20 *= s; combined_matrix.e21 *= s; combined_matrix.e22 *= s;
    return combined_matrix;
}


matrix_3x4
matrix_3x4::make_rotation_scaling (
    const quaternion rotation,
    const float_3    scaling)
{
    ///       RS Matrix        =     Rotation      *      Scaling
    /// | e00  e01  e02  e03 |   | Ux  Vx  Wx  0 |   | Sx  0   0   0 |
    /// | e10  e11  e12  e13 | = | Uy  Vy  Wy  0 | * | 0   Sy  0   0 |
    /// | e20  e21  e22  e23 |   | Uz  Vz  Wz  0 |   | 0   0   Sz  0 |
    ///
    /// | e00  e01  e02  e03 |   | Sx*Ux  Sy*Vx  Sz*Wx  0 |
    /// | e10  e11  e12  e13 | = | Sx*Uy  Sy*Vy  Sz*Wy  0 |
    /// | e20  e21  e22  e23 |   | Sx*Uz  Sy*Vz  Sz*Wz  0 |

    /// 先创建旋转矩阵
    matrix_3x4 combined_matrix = make_rotation(rotation);

    // 将旋转矩阵每行进行放缩
    const float_3 s = scaling;
    combined_matrix.e00 *= s.x; combined_matrix.e01 *= s.y; combined_matrix.e02 *= s.z;
    combined_matrix.e10 *= s.x; combined_matrix.e11 *= s.y; combined_matrix.e12 *= s.z;
    combined_matrix.e20 *= s.x; combined_matrix.e21 *= s.y; combined_matrix.e22 *= s.z;
    return combined_matrix;
}


matrix_3x4
matrix_3x4::make_translation_rotation_scaling (
    const float_3    translation,
    const quaternion rotation,
    const float      scaling)
{
    ///       TRS Matrix       =   Translation   *      Rotation     *     Scaling
    /// | e00  e01  e02  e03 |   | 1  0  0  Tx |   | Ux  Vx  Wx  0 |   | s  0  0  0 |
    /// | e10  e11  e12  e13 | = | 0  1  0  Ty | * | Uy  Vy  Wy  0 | * | 0  s  0  0 |
    /// | e20  e21  e22  e23 |   | 0  0  1  Tz |   | Uz  Vz  Wz  0 |   | 0  0  s  0 |
    ///
    /// | e00  e01  e02  e03 |   | 1  0  0  Tx |   | s*Ux  s*Vx  s*Wx  0 |
    /// | e10  e11  e12  e13 | = | 0  1  0  Ty | * | s*Uy  s*Vy  s*Wy  0 |
    /// | e20  e21  e22  e23 |   | 0  0  1  Tz |   | s*Uz  s*Vz  s*Wz  0 |
    ///
    /// | e00  e01  e02  e03 |   | s*Ux  s*Vx  s*Wx  Tx |
    /// | e10  e11  e12  e13 | = | s*Uy  s*Vy  s*Wy  Ty |
    /// | e20  e21  e22  e23 |   | s*Uz  s*Vz  s*Wz  Tz |

    /// 先创建旋转/放缩矩阵
    matrix_3x4 combined_matrix = make_rotation_scaling(rotation, scaling);
    /// 存储位移
    combined_matrix.e03 = translation.x;
    combined_matrix.e13 = translation.y;
    combined_matrix.e23 = translation.z;
    return combined_matrix;
}


matrix_3x4
matrix_3x4::make_translation_rotation_scaling (
    const float_3    translation,
    const quaternion rotation,
    const float_3    scaling)
{
    ///       TRS Matrix       =   Translation   *      Rotation     *      Scaling
    /// | e00  e01  e02  e03 |   | 1  0  0  Tx |   | Ux  Vx  Wx  0 |   | Sx  0   0   0 |
    /// | e10  e11  e12  e13 | = | 0  1  0  Ty | * | Uy  Vy  Wy  0 | * | 0   Sy  0   0 |
    /// | e20  e21  e22  e23 |   | 0  0  1  Tz |   | Uz  Vz  Wz  0 |   | 0   0   Sz  0 |
    ///
    /// | e00  e01  e02  e03 |   | 1  0  0  Tx |   | Sx*Ux  Sy*Vx  Sz*Wx  0 |
    /// | e10  e11  e12  e13 | = | 0  1  0  Ty | * | Sx*Uy  Sy*Vy  Sz*Wy  0 |
    /// | e20  e21  e22  e23 |   | 0  0  1  Tz |   | Sx*Uz  Sy*Vz  Sz*Wz  0 |
    ///
    /// | e00  e01  e02  e03 |   | Sx*Ux  Sy*Vx  Sz*Wx  Tx |
    /// | e10  e11  e12  e13 | = | Sx*Uy  Sy*Vy  Sz*Wy  Ty |
    /// | e20  e21  e22  e23 |   | Sx*Uz  Sy*Vz  Sz*Wz  Tz |

    /// 先创建旋转/放缩矩阵
    matrix_3x4 combined_matrix = make_rotation_scaling(rotation, scaling);
    /// 存储位移
    combined_matrix.e03 = translation.x;
    combined_matrix.e13 = translation.y;
    combined_matrix.e23 = translation.z;
    return combined_matrix;
}


bool
matrix_3x4::operator== (
    const matrix_3x4 & matx) const
{
    return rows[0] == matx.rows[0] &&
           rows[1] == matx.rows[1] &&
           rows[2] == matx.rows[2];
}


bool
matrix_3x4::operator!= (
    const matrix_3x4 & matx) const
{
    return !operator==(matx);
}


float_4
matrix_3x4::operator[] (
    const uint8_t idx) const
{
    RUNTIME_ASSERT(idx >= 0 && idx < ROW_COUNT, "Index is out of range");
    return rows[idx];
}


float_4 &
matrix_3x4::operator[] (
    const uint8_t idx)
{
    RUNTIME_ASSERT(idx >= 0 && idx < ROW_COUNT, "Index is out of range");
    return rows[idx];
}


matrix_3x4
matrix_3x4::operator* (
    const matrix_3x4 & matx) const
{
    ///         THIS           *          M
    /// | e00  e01  e02  e03 |   | e00  e01  e02  e03 |
    /// | e10  e11  e12  e13 | * | e10  e11  e12  e13 |
    /// | e20  e21  e22  e23 |   | e20  e21  e22  e23 |
    /// |  0    0    0    1  |   |  0    0    0    1  |
    ///
    const float_4 mx_col0 = float_4::make(matx.e00, matx.e10, matx.e20, 0.0f);
    const float_4 mx_col1 = float_4::make(matx.e01, matx.e11, matx.e21, 0.0f);
    const float_4 mx_col2 = float_4::make(matx.e02, matx.e12, matx.e22, 0.0f);
    const float_4 mx_col3 = float_4::make(matx.e03, matx.e13, matx.e23, 1.0f);

    const float combined_e00 = rows[0].dot(mx_col0);
    const float combined_e01 = rows[0].dot(mx_col1);
    const float combined_e02 = rows[0].dot(mx_col2);
    const float combined_e03 = rows[0].dot(mx_col3);

    const float combined_e10 = rows[1].dot(mx_col0);
    const float combined_e11 = rows[1].dot(mx_col1);
    const float combined_e12 = rows[1].dot(mx_col2);
    const float combined_e13 = rows[1].dot(mx_col3);

    const float combined_e20 = rows[2].dot(mx_col0);
    const float combined_e21 = rows[2].dot(mx_col1);
    const float combined_e22 = rows[2].dot(mx_col2);
    const float combined_e23 = rows[2].dot(mx_col3);

    return matrix_3x4
    {
        combined_e00,  combined_e01,  combined_e02,  combined_e03,
        combined_e10,  combined_e11,  combined_e12,  combined_e13,
        combined_e20,  combined_e21,  combined_e22,  combined_e23
    };
}


quaternion
matrix_3x4::rotation () const
{
#if (BUILD_MODE == DEBUG_BUILD_MODE)
    /// 检测是否为旋转矩阵
    const float_3 column0 = float_3::make(e00, e10, e20);
    const float_3 column1 = float_3::make(e01, e11, e21);
    const float_3 column2 = float_3::make(e02, e12, e22);
    RUNTIME_ASSERT(MathUtility::equal(column0.length_squared(), 1.0f),
                   "The first column is not a unit vector!!");
    RUNTIME_ASSERT(MathUtility::equal(column1.length_squared(), 1.0f),
                   "The second column is not a unit vector!!");
    RUNTIME_ASSERT(MathUtility::equal(column2.length_squared(), 1.0f),
                   "The third column is not a unit vector!!");
    RUNTIME_ASSERT(column0.is_perpendicular_to(column1),
                   "The first column is not perpendicular to the second column!!");
    RUNTIME_ASSERT(column0.is_perpendicular_to(column2),
                   "The first column is not perpendicular to the third column!!");
    RUNTIME_ASSERT(column1.is_perpendicular_to(column2),
                   "The second column is not perpendicular to the third column!!");
#endif

    /// 四元数对应的矩阵为:
    /// | e00  e01  e02  e03 |   | 1 - 2*y*y - 2*z*z  2*x*y - 2*z*w      2*x*z + 2*y*w      0 |
    /// | e10  e11  e12  e13 | = | 2*x*y + 2*z*w      1 - 2*x*x - 2*z*z  2*y*z - 2*x*w      0 |
    /// | e20  e21  e22  e23 |   | 2*x*z - 2*y*w      2*y*z + 2*x*w      1 - 2*x*x - 2*y*y  0 |
    ///
    /// 4*x*x = +e00 - e11 - e22 + 1
    ///       = +(e00 - e11) + (1 - e22)
    /// 4*y*y = -e00 + e11 - e22 + 1
    ///       = -(e00 - e11) + (1 - e22)
    /// 4*z*z = -e00 - e11 + e22 + 1
    ///       = -(e00 + e11) + (1 + e22)
    ///
    /// 参考:
    /// - Document/Quaternion_matrix.pdf
    /// - https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
    ///
    /// 首先判断是否w足够大: e00 + e11 + e22 > 0。 这是为了防止除以过小的w。
    /// 4*w*w = e00 + e11 + e22 + 1
    ///       = 1 - 2*y*y - 2*z*z + 1 - 2*x*x - 2*z*z + 1 - 2*x*x - 2*y*y + 1
    ///       = 4 - 4*y*y - 4*z*z - 4*x*x
    ///       = 4 - 4(x*x + y*y +z*z)
    ///       = 4 - 4*(1 - w*w)
    ///       = 4 - 4 + 4*w*w
    ///       = (e00 + e11) + (e22 + 1)
    ///
    /// trace = e00 + e11 + e22
    const float trace = e00 + e11 + e22;

    /// Trace最大
    if (trace > 0)
    {
        /// 4*w*w = e00 + e11 + e22 + 1
        /// 4*w = 2 * sqrt(e00 + e11 + e22  + 1)
        const float four_w = 2.0f * std::sqrtf(trace + 1.0f);
        /// 1/(4*w)
        const float inversed_4w = MathUtility::equal(four_w, 0.0f)
                                ? 1.0f
                                : 1.0f / four_w;

        /// e21 - e12 = 2*y*z + 2*x*w - (2*y*z - 2*x*w)
        ///           = 2*y*z + 2*x*w - 2*y*z + 2*x*w
        ///           = 4*x*w
        /// x = (e21 - e12)/(4*w)
        const float x = (e21 - e12) * inversed_4w;
        /// e02 - e20 = 2*x*z + 2*y*w - (2*x*z - 2*y*w)
        ///           = 2*x*z + 2*y*w - 2*x*z + 2*y*w
        ///           = 4*y*w
        /// y = (e02 - e20)/(4*w)
        const float y = (e02 - e20) * inversed_4w;
        /// e10 - e01 = 2*x*y + 2*z*w - (2*x*y - 2*z*w)
        ///           = 2*x*y + 2*z*w - 2*x*y + 2*z*w
        ///           = 4*z*w
        /// z = (e10 - e01)/(4*w)
        const float z = (e10 - e01) * inversed_4w;
        /// w = 1/4 * 4*w = 0.25*4*w
        const float w = 0.25f * four_w;
        return quaternion{ x, y, z, w };
    }
    else
    {
        /// e00最大
        if (e00 > e11 && e00 > e22)
        {
            /// 4*x*x = e00 - e11 - e22 + 1
            /// 4*x = 2*sqrt(e00 - e11 - e22 + 1)
            const float four_x = 2.0f * std::sqrtf(e00 - e11 - e22 + 1.0f);
            /// 1/(4*x)
            const float inversed_4x = MathUtility::equal(four_x, 0.0f)
                                    ? 1.0f
                                    : 1.0f / four_x;

            /// x = 0.25*4*x
            const float x = 0.25f * four_x;
            /// e01 + e10 = 2*x*y - 2*z*w + 2*x*y + 2*z*w
            ///           = 4*x*y
            /// y = (e01 + e10)/(4*x)
            const float y = (e01 + e10) * inversed_4x;
            /// e02 + e20 = 2*x*z + 2*y*w + 2*x*z - 2*y*w
            ///           = 4*x*z
            /// z = (e02 + e20)/(4*x)
            const float z = (e02 + e20) * inversed_4x;
            /// e21 - e12 = 2*y*z + 2*x*w - (2*y*z - 2*x*w)
            ///           = 2*y*z + 2*x*w - 2*y*z + 2*x*w
            ///           = 4*x*w
            /// w = (e21 - e12)/(4*x)
            const float w = (e21 - e12) * inversed_4x;
            return quaternion{ x, y, z, w };
        }
        /// e11最大
        else if (e11 > e22)
        {
            /// quat.y最大
            /// 4*y*y = e11 -e00 - e22 + 1
            /// 4*y = 2*sqrt(e11 - e00 - e22 + 1)
            const float four_y = 2 * std::sqrtf(e11 - e00 - e22 + 1.0f);
            /// 1/(4*y)
            const float inversed_4y = MathUtility::equal(four_y, 0.0f)
                                    ? 1.0f
                                    : 1.0f / four_y;

            /// e01 + e10 = 2*x*y - 2*z*w + 2*x*y + 2*z*w
            ///           = 4*x*y
            /// x = (e01 + e10)/(4*y)
            const float x = (e01 + e10) * inversed_4y;
            /// y = 0.25*4*y
            const float y = 0.25f * four_y;
            /// e12 + e21 = 2*y*z - 2*x*w + 2*y*z + 2*x*w
            ///           = 4*y*z
            /// z = (e12 + e21)/(4*y)
            const float z = (e12 + e21) * inversed_4y;
            /// e02 - e20 = 2*x*z + 2*y*w - (2*x*z - 2*y*w)
            ///           = 2*x*z + 2*y*w - 2*x*z + 2*y*w
            ///           = 4*y*w
            /// w = (e02 - e20)/(4*y)
            const float w = (e02 - e20) * inversed_4y;
            return quaternion{ x, y, z, w };
        }
        /// e22最大
        else
        {
            /// 4*z*z = e22 - e00 - e11 + 1
            /// 4*z = 2*sqrt(e22 - e00 - e11 + 1)
            const float four_z = 2 * std::sqrtf(e22 - e00 - e11 + 1.0f);
            /// 1/(4*z)
            const float inversed_4z = MathUtility::equal(four_z, 0.0f)
                                    ? 1.0f
                                    : 1.0f / four_z;

            /// e02 + e20 = 2*x*z + 2*y*w + 2*x*z - 2*y*w
            ///           = 4*x*z
            /// x = (e02 + e20)/(4*z)
            const float x = (e02 + e20) * inversed_4z;
            /// e12 + e21 = 2*y*z - 2*x*w + 2*y*z + 2*x*w
            ///           = 4*y*z
            /// y = (e12 + e21)/(4*z)
            const float y = (e12 + e21) * inversed_4z;
            /// z = 0.25*4*z
            const float z = 0.25f * four_z;
            /// e10 - e01 = 2*x*y + 2*z*w - (2*x*y - 2*z*w)
            ///           = 2*x*y + 2*z*w - 2*x*y + 2*z*w
            ///           = 4*z*w
            /// w = (e10 - e01)/(4*z)
            const float w = (e10 - e01) * inversed_4z;
            return quaternion{ x, y, z, w };
        }
    }
}


float_3
matrix_3x4::transform_point (
    const float_3 pnt) const
{
    ///         THIS           *    P
    /// | e00  e01  e02  e03 |   | P.x |
    /// | e10  e11  e12  e13 | * | P.y |
    /// | e20  e21  e22  e23 |   | P.z |
    /// |  0    0    0    1  |   |  1  |
    ///
    /// 构建齐次坐标
    const float_4 pnt_4d = float_4::make(pnt.x, pnt.y, pnt.z, 1.0f);

    return float_3::make(
        rows[0].dot(pnt_4d), rows[1].dot(pnt_4d), rows[2].dot(pnt_4d));
}
