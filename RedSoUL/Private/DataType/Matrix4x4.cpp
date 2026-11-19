/// System headers
#include <cstring> /// std::memcpy
/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "DataType/Matrix3x4.hpp"
#include "Math/MathUtilities.hpp"
/// Self header
#include "DataType/Matrix4x4.hpp"


const matrix_4x4 matrix_4x4::IDENTITY
{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
};


matrix_4x4
matrix_4x4::make (
    const matrix_3x4 & rts_matx)
{
    /// [ RS      | T
    ///   <0,0,0> | 1 ]
    matrix_4x4 ret_matx;

    /// 复制前三行
    std::memcpy(ret_matx.rows, rts_matx.rows, sizeof(matrix_3x4));
    ret_matx.rows[3] = float_4::make(0, 0, 0, 1);
    return ret_matx;
}


matrix_4x4
matrix_4x4::make_perspective_projection (
    const float horizon_fov_rads,
    const float aspect_ratio,
    const float near_plane)
{
    /// Camera --> Clip变换为:
    /// Projection Matrix:
    /// | e00  e01  e02  e03 |   | A  0  0  0 |
    /// | e10  e11  e12  e13 | = | 0  B  0  0 |
    /// | e20  e21  e22  e23 |   | 0  0  C  D |
    /// | e30  e31  e32  e33 |   | 0  0  1  0 |
    ///
    /// 对于相机空间中的点P, 变换后的Clip空间点P'为:
    /// P_clip =       M         * P_camera =   P_clip
    ///          | A  0  0  0 |     | xe |    |  A*xe  |
    /// P'     = | 0  B  0  0 |  *  | ye |  = |  B*ye  |
    ///          | 0  0  C  D |     | ze |    | C*ze+D |
    ///          | 0  0  1  0 |     | 1  |    |   ze   |
    ///
    /// 最终的投影点(NDC空间)P''为:
    ///              | A  0  0  0 |     | xe |    |  A*xe  |
    /// P'' = 1/ze * | 0  B  0  0 |  *  | ye |  = |  B*ye  | * 1/ze
    ///              | 0  0  C  D |     | ze |    | C*ze+D |
    ///              | 0  0  1  0 |     | 1  |    |   ze   |
    ///
    ///       | A/ze*xe |
    /// P'' = | B/ze*ye |
    ///       | C+D/ze  |
    ///       |    1    |
    /// 因此从Clip Space到NDC我们要除以ze: 即, 点P在相机空间中的z的数值
    ///
    /// 为了实现: A*xe/ze 在[-1, +1]之间,    B*ye/ze 在[-1, +1]之间
    /// 可以得到: A 在 [-ze/xe, +ze/xe]之间, B 在 [-ze/ye, +ze/ye]之间
    /// 对于位于深度z处最右侧点P来说:
    /// A*xe/ze = 1, B*ye/ze = 1
    /// A = ze/xe,   B = ze/ye
    ///
    ///                P(xe, ye, ze)
    /// -+------+------o-----------
    ///   \     |     /
    ///    \    |    /
    /// ----+---+---o-----------^--
    ///      \ FOV /            n
    ///       \ | /             e
    ///        \|/              a
    ///         v               r
    ///         C---------------v--> x
    ///
    /// 由于:
    /// ze/n = xe/(W*0.5)
    /// 得到:
    /// A = ze/xe = n/(W*0.5)
    /// B = ze/ye = n/(H*0.5)
    ///
    /// 此处W, H为: Near Plane上可视区域尺寸
    ///
    /// 由于:
    /// tangent(Fov * 0.5) = 0.5*W / near
    /// 得到:
    /// W = 2 * near * tangent(Fov * 0.5)
    /// H = W / aspect
    ///   = 2 * near * tangent(Fov * 0.5) / aspect
    ///
    /// - A = ze/xe = n/(W*0.5) = 2*n/W = near/(near * tangent(Fov * 0.5)) = 1/tangent(Fov/2)
    ///     = cotangent(Fov * 0.5)
    /// - B = ze/ye = n/(H*0.5) = 2*n/H = near/(near * tangent(Fov * 0.5) / aspect)
    ///     = cotangent(Fov * 0.5) * aspect
    ///
    /// 为了实现在NDC中, Near映射到1, Far映射到0
    /// C*near + D = near 并且 C*Far + D = 0
    /// 可以得到:
    /// C*(Far - near) = -near
    ///
    /// - C = -near/(Far - near)
    /// - D = near*Far/(Far - near)
    ///
    /// 最终投影矩阵:
    /// Projection Matrix:
    /// | e00  e01  e02  e03 |   | A  0  0  0 |
    /// | e10  e11  e12  e13 | = | 0  B  0  0 |
    /// | e20  e21  e22  e23 |   | 0  0  C  D |
    /// | e30  e31  e32  e33 |   | 0  0  1  0 |
    ///
    ///   | xscale 0      0      0      |   | 2*n/W  0          0         0         |
    /// = | 0      yscale 0      0      | = | 0      2*n/H*W/H  0         0         |
    ///   | 0      0      zscale zshift |   | 0      0          -n/(f-n)  n*f/(f-n) |
    ///   | 0      0      1      0      |   | 0      0          1         0         |
    ///
    /// 如果使用水平Fov,
    /// Projection Matrix:
    /// | cot(Fov/2) 0                  0         0         |
    /// | 0          cot(Fov/2)*aspect  0         0         |
    /// | 0          0                  -n/(f-n)  n*f/(f-n) |
    /// | 0          0                  1         0         |
    ///
    /// 对于Infinite Far Plane:
    /// Projection Matrix:
    /// | cot(Fov/2) 0                  0  0 |
    /// | 0          cot(Fov/2)*aspect  0  0 |
    /// | 0          0                  0  n |
    /// | 0          0                  1  0 |
    ///
    /// Clip空间点P'为:
    ///      |     cot(Fov/2)*xe    |
    /// P' = | cot(Fov/2)*aspect*ye |
    ///      |          n           |
    ///      |          ze          |
    ///
    float sin_half_fov, cos_half_fov;
    MathUtility::fast_sincos(horizon_fov_rads * 0.5f, sin_half_fov, cos_half_fov);

    /// cot(Fov/2)
    const float cot_half_fov = cos_half_fov / sin_half_fov;
    const float xscale = cot_half_fov;
    /// cot(Fov/2) * aspect
    const float yscale = xscale * aspect_ratio;

    /// 合成Projection Matrix:
    /// | cot(Fov/2) 0                  0  0 |
    /// | 0          cot(Fov/2)*aspect  0  0 |
    /// | 0          0                  0  n |
    /// | 0          0                  1  0 |
    return matrix_4x4
    {
        xscale,  0,       0,  0,
        0,       yscale,  0,  0,
        0,       0,       0,  near_plane,
        0,       0,       1,  0
    };
}


float_4
matrix_4x4::operator[] (
    const uint8_t idx) const
{
    RUNTIME_ASSERT(idx >= 0 && idx < ROW_COUNT, "Index is out of range");
    return rows[idx];
}


float_4 &
matrix_4x4::operator[] (
    const uint8_t idx)
{
    RUNTIME_ASSERT(idx >= 0 && idx < ROW_COUNT, "Index is out of range");
    return rows[idx];
}
