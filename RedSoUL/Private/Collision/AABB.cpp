/// System headers
#include <algorithm> /// std::min, std::max
/// Library headers
#include "Math/MathDefines.hpp"
/// Self header
#include "Collision/AABB.hpp"


float_3
AABB::mid_point () const
{
    return (pmax + pmin) * 0.5f;
}


AxisType
AABB::longest_axis () const
{
    /// Epsilon: 1毫米
    static constexpr float SHIFT_EPSILON = 1.0f / 1000.0f;

    /// 计算对角线方向上的偏移: 为了打破x,y,z坐标相同的僵局
    const float_3 shift_vec = float_3::make_random_unit_vec() * SHIFT_EPSILON;

    /// 计算对角线向量
    const float_3 diag_vec = diagonal_vec() + shift_vec;

    return (diag_vec.x>diag_vec.y && diag_vec.x>diag_vec.z) ? AxisType::X_AXIS_TYPE:
           (diag_vec.y>diag_vec.z                         ) ? AxisType::Y_AXIS_TYPE:
                                                              AxisType::Z_AXIS_TYPE;
}


float_3
AABB::diagonal_vec () const
{
    return pmax - pmin;
}


float
AABB::area () const
{
    /// 包围盒采用如下顶点编号:
    ///
    ///              ^ Y
    ///              |     / Z
    ///      6 +-----|-------+ 7
    ///       /|     .   /  /|
    ///      / |        /  / |
    ///   2 +-------------+ 3|
    ///     |  |     |/   |  |
    ///     |  |     o----|.--------> X
    ///     |  |          |  |
    ///     |4 +----------|- + 5
    ///     | /           | /
    ///   0 +-------------+ 1
    ///
    /// 计算由顶点0到顶点7的对角线向量
    const float_3 diag_vec = diagonal_vec();

    /// 计算地面: 0451
    ///   ^ z
    ///   |
    /// 4 +------------* 5
    ///   |        *   |
    ///   |   *        |
    /// 0 *------------+-> x
    /// 对角线.x * 对角线.z
    const float area_0451 = diag_vec.x * diag_vec.z;

    /// 计算左面: 0264
    ///   ^ y
    ///   |
    /// 2 +------------* 6
    ///   |        *   |
    ///   |   *        |
    /// 0 *------------+-> z
    /// 对角线.z * 对角线.y
    const float area_0264 = diag_vec.z * diag_vec.y;

    /// 计算前面: 0231
    ///   ^ y
    ///   |
    /// 2 +------------* 3
    ///   |        *   |
    ///   |   *        |
    /// 0 *------------+-> x
    /// 对角线.x * 对角线.y
    const float area_0231 = diag_vec.x * diag_vec.y;

    return 2.0f * (area_0451 + area_0264 + area_0231);
}


float
AABB::calc_hit_time (
    const Ray3D local_ray) const
{
    /// 基于Slab的碰撞检测
    /// - 射线定义为: Ray(t) = O + t*D
    /// 这里:
    /// * O为射线的起点
    /// * D为射线的方向(无归一化要求)
    /// - 任何两个包围盒中平行平面(例如X轴向)之间的空间为Slab
    /// - 平面定义为:
    ///   P(a) = (a - C) dot N = 0 --> P(a) := Dot(a, N) + K, K := -Dot(C, N)
    ///
    ///              ^ N/法线
    ///              |
    ///              |   B/辅助切线
    ///         +----|---/------+
    ///        /     |  /      /
    ///       /  *a  | /      /
    ///      /       C-------/---> T/切线
    ///     /               /
    ///    /               /
    ///   +---------------+
    /// 这里:
    /// * a为平面上任意一点
    /// * C为平面的中心点
    /// * N为平面的法线
    ///
    /// 对于Hit点: P(Hit) := (O + t*D - C) dot N = 0
    /// 对于Xmin平面而言: (O + t*D) dot (-X) - Xmin dot (-X) = 0
    /// t*Dot(D, -X) = Dot(Xmin, -X) - Dot(O, -X)
    ///       Dot(Xmin, -X) - Dot(O, -X)    -Xmin.x + O.x     Xmin.x - O.x
    /// t =  ──────────────────────────── = ────────────── = ───────────────
    ///              Dot(D, -X)                  -D.x              D.x
    ///
    ///        |         t   | *
    ///        |         1   o t1x
    ///        |         y * |
    ///    ----+---------o---+---- Ymax
    ///        | t     *     |
    ///        | 0   *       |
    ///        | y *         |
    ///    ----+-o-----------+---- Ymin
    ///    t0x o             |
    ///      * |             |
    ///    *   Xmin          Xmax
    ///  Ray
    ///
    /// 进入点的t0:
    /// t0x = (Xmin.x - O.x) / D.x
    /// t0y = (Ymin.y - O.y) / D.y
    /// t0z = (Zmin.z - O.z) / D.z
    /// 
    /// 离开点的t1:
    /// t1x = (Xmax.x - O.x) / D.x
    /// t1y = (Ymax.y - O.y) / D.y
    /// t1z = (Zmax.z - O.z) / D.z
    ///
    /// 有HIT: 则所有进入点中最大的T值 <= 所有离开点中的最小的T值:
    /// - HIT --> Max(t0x, t0y, t0z) <= Min(t1x, t1y, t1z)
    ///
    float time_enter = 0.0f;
    float time_exit  = MAXIMAL_FLOAT;

    /// X轴向的平面(Xmin, Xmax平面):
    {
        /// 计算: Ray的方向的倒数: 1/Ray.d
        /// NOTE:
        /// * 如果D.x == +0 --> 1/D.x = +INF
        /// * 如果D.x == -0 --> 1/D.x = -INF
        const float inv_dirx = 1.0f / local_ray.dir.x;

        /// t0x = (Xmin.x - O.x) / D.x
        /// NOTE: 如果Ray.dir.x < 0(即: 从右向左), 我们将翻转Xmin, Xmax平面
        const float t0x = inv_dirx > 0 ? (pmin.x - local_ray.org.x) * inv_dirx:
                                         (pmax.x - local_ray.org.x) * inv_dirx;
        /// t1x = (Xmax.x - O.x) / D.x
        const float t1x = inv_dirx > 0 ? (pmax.x - local_ray.org.x) * inv_dirx:
                                         (pmin.x - local_ray.org.x) * inv_dirx;

        time_enter = std::max(time_enter, t0x);
        time_exit  = std::min(time_exit,  t1x);

        if (time_exit < time_enter)
        {
            return -1.0f;
        }
    }

    /// Y轴向的平面(Ymin, Ymax平面):
    {
        const float inv_diry = 1.0f / local_ray.dir.y;

        /// t0y = (Ymin.y - O.y) / D.y
        const float t0y = inv_diry > 0 ? (pmin.y - local_ray.org.y) * inv_diry:
                                         (pmax.y - local_ray.org.y) * inv_diry;
        /// t1y = (Ymax.y - O.y) / D.y
        const float t1y = inv_diry > 0 ? (pmax.y - local_ray.org.y) * inv_diry:
                                         (pmin.y - local_ray.org.y) * inv_diry;

        time_enter = std::max(time_enter, t0y);
        time_exit  = std::min(time_exit,  t1y);

        if (time_exit < time_enter)
        {
            return -1.0f;
        }
    }

    /// Z轴向的平面(Zmin, Zmax平面):
    {
        const float inv_dirz = 1.0f / local_ray.dir.z;

        /// t0z = (Zmin.z - O.z) / D.z
        const float t0z = inv_dirz > 0 ? (pmin.z - local_ray.org.z) * inv_dirz:
                                         (pmax.z - local_ray.org.z) * inv_dirz;
        /// t1z = (Zmax.z - O.z) / D.z
        const float t1z = inv_dirz > 0 ? (pmax.z - local_ray.org.z) * inv_dirz:
                                         (pmin.z - local_ray.org.z) * inv_dirz;

        time_enter = std::max(time_enter, t0z);
        time_exit  = std::min(time_exit,  t1z);
        if (time_exit < time_enter)
        {
            return -1.0f;
        }
        else
        {
            return time_enter;
        }
    }
}


void
AABB::set_empty ()
{
    /// 将最小点Pmin设置的比最大点Pmax大
    pmin = float_3::make(+1.0f, +1.0f, +1.0f);
    pmax = float_3::make(-1.0f, -1.0f, -1.0f);
}


void
AABB::union_with (
    const AABB & box)
{
    /// 取最小点中的最小
    /// 取最大点中的最大
    pmin = float_3::make
    (
        std::min(pmin.x, box.pmin.x),
        std::min(pmin.y, box.pmin.y),
        std::min(pmin.z, box.pmin.z)
    );
    pmax = float_3::make
    (
        std::max(pmax.x, box.pmax.x),
        std::max(pmax.y, box.pmax.y),
        std::max(pmax.z, box.pmax.z)
    );
}
