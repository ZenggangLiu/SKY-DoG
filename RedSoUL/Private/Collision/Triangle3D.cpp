/// Library headers
#include "Math/MathDefines.hpp"
/// Self header
#include "Collision/Triangle3D.hpp"


Triangle3D
Triangle3D::make ()
{
    ///       y
    ///       ^
    ///       |
    ///       B (0,0.5,0)
    ///      /|\
    ///     / | \
    /// ---/--+--\-----> x
    ///   /   |   \
    ///  A---------C (0.5,-0.5,0)
    /// (-0.5,-0.5,0)
    return Triangle3D
    {
        { -0.5f, -0.5f, 0.0f },
        {  0.0f,  0.5f, 0.0f },
        {  0.5f, -0.5f, 0.0f }
    };
}


float
Triangle3D::calc_hit_time (
    const Ray3D local_ray) const
{
    ///        B
    ///        /\    / O
    ///       /  \  /
    ///      /    \/
    ///     E0    /\
    ///    /     /  \
    ///   /     * P  \
    ///  /            \
    /// A------E1------C
    ///        E1
    ///
    /// 使用重心坐标系, 交点P可以表示为:
    /// P(α, β, γ) := α*A + β*B + γ*C,
    /// - 0 < α < 1, 0 < β < 1, 0 < γ < 1, 并且α + β + γ == 1
    ///
    /// 如果只使用β，γ来表示:
    /// P(β, γ) := (1-β-γ)*A + β*B + γ*C
    ///          = A - β*A - γ*A + β*B + γ*C
    ///          = A + β*(B-A) + γ*(C-A)
    ///          = A + β*E0 + γ*E1
    /// -E0为BA边, E1为CA边: 即: E0 = (B-A), E1 = (C-A)
    ///
    /// 对于Ray(t) := O + t*D
    /// 如果有交点, 交点P可以表示为:
    /// P := O + t*D = A + β*E0 + γ*E1
    /// t*D - β*E0 - γ*E1 = A - O, 两侧乘以-1
    /// β*E0 + γ*E1 - t*D = O - A
    ///
    /// 可以使用矩阵方式表示这个等式:
    ///                [β
    /// [E0  E1  -D] *  γ    = (O - A) = S
    ///                 t ]
    ///
    /// 使用Cramer's Rule可以求解β，γ，t
    ///      Det(S, E1, -D)
    /// β = ────────────────
    ///      Det(E0, E1, -D)
    ///
    ///      Det(E0, S, -D)
    /// γ = ────────────────
    ///      Det(E0, E1, -D)
    ///
    ///      Det(E0, E1, S)
    /// t = ────────────────
    ///      Det(E0, E1, -D)
    ///
    /// 使用Determinant的计算公式以及它的属性: 两行(或者两列)对调, Determinant变反
    /// Det(A, B, C) = (A x B) dot C
    ///
    /// 因此:
    ///      Det(S, E1, -D)     -Det(S, -D, E1)    -(S x -D) dot E1
    /// β = ──────────────── = ───────────────── = ────────────────
    ///      Det(E0, E1, -D)     Det(E0, E1, -D)   (E0 x E1) dot -D
    ///
    ///      (S x D) dot E1
    ///   = ─────────────────
    ///     -(E0 x E1) dot D
    ///
    ///      Det(E0, S, -D)       -Det(S, E0, -D)     Det(S, -D, E0)     (S x -D) dot E0
    /// γ = ────────────────── = ───────────────── = ──────────────── = ──────────────────
    ///      Det(E0, E1, -D)      Det(E0, E1, -D)     Det(E0, E1, -D)    (E0 x E1) dot -D
    ///
    ///      -(S x D) dot E0
    ///   = ──────────────────
    ///      -(E0 x E1) dot D
    ///
    ///      Det(E0, E1, S)      (E0 x E1) dot S
    /// t = ──────────────── = ──────────────────
    ///      Det(E0, E1, -D)    -(E0 x E1) dot D
    ///
    /// 总结:
    ///       (E0 x E1) dot S
    /// t = ──────────────────
    ///      -(E0 x E1) dot D
    ///
    ///      (S x D) dot E1
    /// β = ─────────────────
    ///     -(E0 x E1) dot D
    ///
    ///      -(S x D) dot E0
    /// γ = ──────────────────
    ///      -(E0 x E1) dot D
    /// 
    /// 这里:
    /// - S  := O - A
    /// - E0 := B - A
    /// - E1 := C - A
    ///
    /// 要相交必须满足:
    /// - (E0 x E1) dot D < -EPSILON
    ///   由于我们使用BackFace Culling, 所以-(E0 x E1) dot D > EPSILON
    /// - 0 < (S x D) dot E1 < -(E0 x E1) dot D
    ///   由于0 < β < 1
    /// - 0 < -(S x D) dot E0 < -(E0 x E1) dot D
    ///   由于0 < γ < 1
    /// - (S x D) dot E1 - (S x D) dot E0 < -(E0 x E1) dot D
    ///   由于α + β + γ == 1, 因此β + γ < 1
    /// - t >= 0
    ///
    /// E0 := B - A 从A指向B的向量
    const float_3 e0 = b - a;
    /// E1 = C - A 从A指向C的向量
    const float_3 e1 = c - a;
    /// N := E0 x E1
    const float_3 n = e0.cross(e1);
    /// (E0 x E1) dot D
    const float n_dot_d = n.dot(local_ray.dir);

    /// 检测是否射线朝向三角面的正面(即: 射线的方向D与三角面法线N反向: Dot(N, D) < 0)
    /// 检测是否射线与三角面平行(即: 射线方向D与三角面法线N垂直: Dot(N, D) ≈ 0)
    if (n_dot_d >= -EPSILON)
    {
        return -1.0f;
    }
    else
    {
        /// S := O - A 从A指向O的向量
        const float_3 s = local_ray.org - a;
        /// M := S x D
        const float_3 m = s.cross(local_ray.dir);
        /// (S x D) dot E1
        const float m_dot_e1 = m.dot(e1);

        /// 检测β是否有效: 0 < (S x D) dot E1 < -(E0 x E1) dot D
        if (m_dot_e1 <= 0.0f || m_dot_e1 >= -n_dot_d)
        {
            return -1.0f;
        }
        else
        {
            /// (S x D) dot E0
            const float m_dot_e0 = m.dot(e0);

            /// 检测γ是否有效: 0 < -(S x D) dot E0 < -(E0 x E1) dot D
            if (-m_dot_e0 <= 0.0f || -m_dot_e0 >= -n_dot_d)
            {
                return -1.0f;
            }
            else
            {
                /// 检测β + γ < 1
                /// (S x D) dot E1 - (S x D) dot E0 < -(E0 x E1) dot D
                if ((m_dot_e1 - m_dot_e0) >= -n_dot_d)
                {
                    return -1.0f;
                }
                else
                {
                    /// 计算Hit Time
                    ///       (E0 x E1) dot S
                    /// t = ──────────────────
                    ///      -(E0 x E1) dot D
                    const float n_dot_s = n.dot(s);
                    return -n_dot_s / n_dot_d;
                }
            }
        }
    }
}
