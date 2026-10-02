/// System headers
#include <cmath> /// std::sqrtf
/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Math/MathUtilities.hpp"
/// Self header
#include "DataType/Quaternion.hpp"


const quaternion quaternion::IDENTITY{ 0, 0, 0, 1 };


quaternion
quaternion::make (
    const AxisType rot_axis,
    const float    rot_rads)
{
    switch (rot_axis)
    {
        case AxisType::X_AXIS_TYPE:
        {
            /// U轴为<1, 0, 0>
            return make(float_3::RIGHT, rot_rads);
        }

        case AxisType::Y_AXIS_TYPE:
        {
            /// U轴为<0, 1, 0>
            return make(float_3::UP, rot_rads);
        }

        case AxisType::Z_AXIS_TYPE:
        {
            /// U轴为<0, 0, 1>
            return make(float_3::FORWARD, rot_rads);
        }

        default:
        {
            RUNTIME_ASSERT(false, "Unknown rotate axis");
            return IDENTITY;
        }
    }
}


quaternion
quaternion::make (
    const float_3 rot_axis,
    const float   rot_rads)
{
    /// 由于单位四元数可以表示为:
    /// {
    ///     Sin(ϕ/2)*U,
    ///     Cos(ϕ/2)
    /// }
    /// - 这里U为旋转轴，ϕ为旋转角度
    ///
    /// 归一化旋转轴 U
    float_3 axis_u = rot_axis;
    axis_u.normalize();

    /// NOTE: 四元数保存的为一半的旋转角度(即: ϕ/2)
    /// 计算sin(ϕ/2), cos(ϕ/2)
    float sin_half_phi, cos_half_phi;
    MathUtility::fast_sincos(rot_rads / 2.0f, sin_half_phi, cos_half_phi);

    /// 创建单位四元数: { Sin(ϕ/2)*U, Cos(ϕ/2) }
    return quaternion
    {
        sin_half_phi * axis_u.x, sin_half_phi * axis_u.y, sin_half_phi * axis_u.z,
        cos_half_phi
    };
}


quaternion
quaternion::make (
    const float_3 v1,
    const float_3 v2)
{
    /// 公式推导参考: Real time Rendering 4th, page 83
    /// - Rotation from One Vector to Antoher
    ///
    /// S为起始向量(归一化的v1), E为终止向量(归一化的v2)
    /// ϕ为S与E之间的夹角
    ///
    /// U := Cross(S, E)
    /// ^
    /// |   / S
    /// |  /
    /// | / ϕ
    /// O---------> E
    ///
    /// 1) S将按照U的轴旋转到E
    /// 2) 如果使用单位四元数来表示此旋转, 可得到:
    ///    Q = { Sin(ϕ/2)*U, Cos(ϕ/2) }
    /// 3) 由于Cross(S, E) = Sin(ϕ)*U, Dot(S, E) = Cos(ϕ)
    /// 4) 利用半角公式:
    ///    - Cos(ϕ) = 2*Cos(ϕ/2)*Cos(ϕ/2) - 1
    ///    得到:
    ///                       1 + Cos(ϕ)
    ///    Cos(ϕ/2) =  ±sqrt(────────────)
    ///                           2
    ///    由于ϕ最大夹为π, 所以ϕ/2最大为π/2, 因此取正数
    ///    - Sin(ϕ) = 2*Sin(ϕ/2)*Cos(ϕ/2)
    ///    得到:
    ///                 Sin(ϕ)             Sin(ϕ)
    ///    Sin(ϕ/2) = ─────────── = ──────────────────────
    ///               2*Cos(ϕ/2)     sqrt[2*(1 + Cos(ϕ))]
    /// 5) 最后将Cos(ϕ/2)以sqrt[2*(1 + Cos(ϕ))]的形式表示为:
    ///                     2*(1 + Cos(ϕ))     sqrt[2*(1 + Cos(ϕ))]
    ///    Cos(ϕ/2) = sqrt(────────────────) = ─────────────────────
    ///                          2*2                    2
    ///
    /// 最终得到:
    ///              Sin(ϕ)              sqrt[2*(1 + Cos(ϕ))]
    /// Q = { ──────────────────────*U, ────────────────────── }: ϕ为S与E之间的夹角
    ///        sqrt[2*(1 + Cos(ϕ))]               2
    ///
    ///                1                           sqrt[2*(1 + Cos(ϕ))]
    ///   = { ──────────────────────*Cross(S, E), ────────────────────── }
    ///        sqrt[2*(1 + Cos(ϕ))]                         2
    ///
    const float_3 start_vec = v1.unified_vec();
    const float_3 ende_vec  = v2.unified_vec();

    /// 判断S, E是否平行(同向, 反向)
    if (start_vec.is_parallel_to(ende_vec))
    {
        /// 计算点积Cos(ϕ): Dot(S, E)
        const float cos_phi = start_vec.dot(ende_vec);

        /// Cos(ϕ) > 0, 同向, 无旋转
        /// Cos(ϕ) < 0, 反向, 按任意旋转轴转180度(即: π)
        return cos_phi > 0 ?
               IDENTITY    :
               quaternion::make(start_vec.perpendicular_vec(), ONE_PI);
    }
    else
    {
        /// 计算点积Cos(ϕ): Dot(S, E)
        const float cos_phi = start_vec.dot(ende_vec);
        /// 计算S与E的叉积Cross(S, E)
        const float_3 cross_vec = start_vec.cross(ende_vec);

        // 计算scale := sqrt[2 * (1 + Cos(ϕ))]
        const float scale = std::sqrtf(2.0f * (1.0f + cos_phi));

        ///            1                            sqrt[2*(1 + Cos(ϕ))]
        /// { ──────────────────────*Cross(S, E),  ────────────────────── }
        ///    sqrt[2*(1 + Cos(ϕ))]                          2
        return quaternion
        {
            cross_vec.x / scale, cross_vec.y / scale, cross_vec.z / scale,
            scale / 2.0f
        };
    }
}


quaternion
quaternion::nlerp(
    const quaternion quat0,
    const quaternion quat1,
    const float      time)
{
    /// Nlerp的特性:
    /// - 沿着最短路径插值
    /// - 合成顺序无关(Commutative): 多个Pose混合, 它的结果不依赖与他们混合的顺序
    /// - 非均匀角速度(Non-Constant Angular Velocity)
    /// - 最小转矩(Minimal Torque)
    ///
    /// 插值系数T:
    /// 0            1
    /// Q0 --------> Q1
    ///
    /// 如果时间time为0: time == 0
    if (MathUtility::equal(time, 0.0f))
    {
        return quat0;
    }
    else
    {
        /// 如果时间time为1或者超出界限: time >= 1
        if (MathUtility::equal(time, 1.0f) || time > 1.0f)
        {
            return quat1;
        }
        else
        {
            /// (1-t)*Q0 + t*Q1
            quaternion combined_quat
            {
                (1.0f - time) * quat0.x + time * quat1.x,
                (1.0f - time) * quat0.y + time * quat1.y,
                (1.0f - time) * quat0.z + time * quat1.z,
                (1.0f - time) * quat0.w + time * quat1.w
            };
            combined_quat.xyzw.normalize();
            return combined_quat;
        }
    }
}


quaternion
quaternion::slerp (
    const quaternion quat0,
    const quaternion quat1,
    const float      time)
{
    /// 插值系数T:
    /// 0            1
    /// Q0 --------> Q1
    ///
    ///         sin[(1-t) * ϕ]         sin(t * ϕ)
    /// Qt := ──────────────────*Q0 + ────────────*Q1
    ///            sin(ϕ)               sin(ϕ)
    /// 这里:
    /// ϕ为Q0与Q1之间的夹角
    ///
    /// 如果时间time为0: time == 0
    if (MathUtility::equal(time, 0.0f))
    {
        return quat0;
    }
    else
    {
        /// 如果时间time为1或者超出界限: time >= 1
        if (MathUtility::equal(time, 1.0f) || time > 1.0f)
        {
            return quat1;
        }
        else
        {
            /// 计算Dot(Q0, Q1) = Cos(ϕ)
            float cos_phi = quat0.xyzw.dot(quat1.xyzw);

            /// NOTE:
            /// 计算ϕ之前, 必须确保ϕ为锐角(Acute Angle)(即: ϕϵ[0, π/2])
            ///
            /// 如果ϕ为钝角(Obtuse Angle)(即: ϕϵ[π/2, π])
            /// 将翻转Q1(即: 使用-Q1), 以确保按照最短路径插值。
            ///
            const bool is_phi_obtuse = cos_phi < 0;
            if (is_phi_obtuse)
            {
                /// 此时cos(ϕ)为负数, 翻转它
                cos_phi = -cos_phi;
            }

            /// 计算Q0与Q1之间的夹角ϕ
            /// MathUtility::fast_acos(param)返回[0, π], 由于param确保为正
            /// 所以得到的 ϕϵ[0, π/2]
            const float phi = MathUtility::fast_acos(cos_phi);
            const float inv_sine_phi = 1.0f / MathUtility::fast_sine(phi);

            ///         sin[(1-t) * ϕ]
            /// 计算: ──────────────────
            ///            sin(ϕ)
            const float scale_quat0 =
                MathUtility::fast_sine((1.0f - time) * phi) * inv_sine_phi;
            const quaternion quat0_scaled{
                scale_quat0 * quat0.x, scale_quat0 * quat0.y,
                scale_quat0 * quat0.z, scale_quat0 * quat0.w };

            ///        sin(t * ϕ)
            /// 计算: ─────────────
            ///          sin(ϕ)
            const float scale_quat1 =
                MathUtility::fast_sine(time * phi) * inv_sine_phi;
            /// 如果ϕ为钝角, 则翻转Q1
            const quaternion quat1_scaled =
                is_phi_obtuse ?
                quaternion{
                    -scale_quat1 * quat1.x, -scale_quat1 * quat1.y,
                    -scale_quat1 * quat1.z, -scale_quat1 * quat1.w }:
                quaternion{
                     scale_quat1 * quat1.x,  scale_quat1 * quat1.y,
                     scale_quat1 * quat1.z,  scale_quat1 * quat1.w };

            quaternion combined_quat
            {
                quat0_scaled.x + quat1_scaled.x,
                quat0_scaled.y + quat1_scaled.y,
                quat0_scaled.z + quat1_scaled.z,
                quat0_scaled.w + quat1_scaled.w
            };
            combined_quat.xyzw.normalize();
            return combined_quat;
        }
    }
}


bool
quaternion::operator== (
    const quaternion quat) const
{
    const float_4 SELF { x, y, z, w };
    const float_4 OTHER{ quat.x, quat.y, quat.z, quat.w };

    /// 由于Q, -Q表示同一个旋转, 所以我们进行Dot Product计算，
    /// 再检查cos(ϕ)是否近似为1, 其中ϕ为它们的夹角。
    return MathUtility::equal(std::fabsf(SELF.dot(OTHER)), 1);
}


bool
quaternion::operator!= (
    const quaternion quat) const
{
    return !operator==(quat);
}


float
quaternion::operator[] (
    const uint8_t idx) const
{
    RUNTIME_ASSERT(idx >= 0 && idx < 4, "Index is out of range");
    return e[idx];
}


float &
quaternion::operator[] (
    const uint8_t idx)
{
    RUNTIME_ASSERT(idx >= 0 && idx < 4, "Index is out of range");
    return e[idx];
}


quaternion
quaternion::operator* (
    const quaternion rot) const
{
    /// 乘积操作定义为: NOTE: 此处的四元数为任意长度!!
    ///  C := Q * R = { Q.v, Q.w } * { R.v, R.w }
    ///             = { <Q.x*I, Q.y*J, Qz*K>, Q.w } * { <R.x*I, R.y*J, R.z*K>, R.w }
    /// 这里:
    ///  - <I, J, K>为虚数单位(Imaginary Units)。并且:
    ///    1) I*I == J*J == K*K = -1
    ///    2) I*J == K, J*K == I, K*I == J
    ///  - 因此:Q * R 即{ <Q.x*I, Q.y*J, Qz*K>, Q.w } * { <R.x*I, R.y*J, R.z*K>, R.w }
    ///    导出为:
    ///    Q.x*R.x*II + Q.x*R.y*IJ + Q.x*R.z*IK + Q.x*R.w*I +
    ///    Q.y*R.x*JI + Q.y*R.y*JJ + Q.y*R.z*JK + Q.y*R.w*J +
    ///    Q.z*R.x*KI + Q.z*R.y*KJ + Q.z*R.z*KK + Q.z*R.w*K +
    ///    Q.w*R.x*I  + Q.w*R.y*J  + Q.w*R.z*K  + Q.w*R.w
    ///    合并得到:
    ///    -Q.x*R.x   + Q.x*R.y*K - Q.x*R.z*J + Q.x*R.w*I +
    ///    -Q.y*R.x*K - Q.y*R.y   + Q.y*R.z*I + Q.y*R.w*J +
    ///     Q.z*R.x*J - Q.z*R.y*I - Q.z*R.z   + Q.z*R.w*K +
    ///     Q.w*R.x*I + Q.w*R.y*J + Q.w*R.z*K + Q.w*R.w
    ///    虚数单位I对应的项为:  Q.x*R.w*I + Q.y*R.z*I - Q.z*R.y*I + Q.w*R.x*I
    ///    虚数单位J对应的项为: -Q.x*R.z*J + Q.y*R.w*J + Q.z*R.x*J + Q.w*R.y*J
    ///    虚数单位K对应的项为:  Q.x*R.y*K - Q.y*R.x*K + Q.z*R.w*K + Q.w*R.z*K
    ///    实部为: -Q.x*R.x - Q.y*R.y - Q.z*R.z + Q.w*R.w
    /// 整理顺序, 改为向量操作为:
    ///  - C.v := (Q.v cross R.v) + R.w*Q.v + Q.w*R.v
    ///           (Q.y*R.z - Q.z*R.y + Q.x*R.w + Q.w*R.x)*I
    ///         = (Q.z*R.x - Q.x*R.z + Q.y*R.w + Q.w*R.y)*J
    ///           (Q.x*R.y - Q.y*R.x + Q.z*R.w + Q.w*R.z)*K
    ///
    ///  - C.w := Q.w*R.w - (Q.v dot R.v)
    ///         = Q.w*R.w - Q.x*R.x - Q.y*R.y - Q.z*R.z
    ///
    /// 几个特殊的四元数乘积:
    /// - Q * S := { <Q.x*I, Q.y*J, Qz*K>, Q.w } * { <0*I, 0*J, 0*K, >, s }.即Q放缩S倍
    ///   导出为:
    ///   Q.x*0*II + Q.x*0*IJ + Q.x*0*IK + Q.x*s*I +
    ///   Q.y*0*JI + Q.y*0*JJ + Q.y*0*JK + Q.y*s*J +
    ///   Q.z*0*KI + Q.z*0*KJ + Q.z*0*KK + Q.z*s*K +
    ///   Q.w*0*I  + Q.w*0*J  + Q.w*0*K  + Q.w*s
    ///   为:{ < s*Q.x*I, s*Q.y*J, s*Qz*K >, s*Q.w }
    ///   即: Q * S = 每个成员放缩S倍
    /// - Q * Q* := { <Q.x*I, Q.y*J, Qz*K>, Q.w } * { <-Q.x*I, -Q.y*J, -Qz*K>, Q.w }
    ///   导出为:
    ///   -Q.x*Q.x*II - Q.x*Q.y*IJ - Q.x*Q.z*IK + Q.x*Q.w*I +
    ///   -Q.y*Q.x*JI - Q.y*Q.y*JJ - Q.y*Q.z*JK + Q.y*Q.w*J +
    ///   -Q.z*Q.x*KI - Q.z*Q.y*KJ - Q.z*Q.z*KK + Q.z*Q.w*K +
    ///   -Q.w*Q.x*I  - Q.w*Q.y*J  - Q.w*Q.z*K  + Q.w*Q.w
    ///   虚数单位I对应的项为:  Q.x*Q.w*I - Q.y*Q.z*I + Q.z*Q.y*I - Q.w*Q.x*I = 0*I
    ///   虚数单位J对应的项为:  Q.x*Q.z*J + Q.y*Q.w*J - Q.z*Q.x*J - Q.w*Q.y*J = 0*J
    ///   虚数单位K对应的项为: -Q.x*Q.y*K + Q.y*Q.x*K + Q.z*Q.w*K - Q.w*Q.z*K = 0*K
    ///   实部为: Q.x*Q.x  Q.y*Q.y  Q.z*Q.z + Q.w*Q.w
    ///   为: { <0*I, 0*J, 0*K, >, |Q|*|Q| }
    ///   即: Q*Q* = |Q|^2, Norm的平方
    ///
    ///        (Q.y*R.z - Q.z*R.y + Q.x*R.w + Q.w*R.x)*I
    /// C.v := (Q.z*R.x - Q.x*R.z + Q.y*R.w + Q.w*R.y)*J
    ///        (Q.x*R.y - Q.y*R.x + Q.z*R.w + Q.w*R.z)*K
    /// C.w :=  Q.w*R.w - Q.x*R.x - Q.y*R.y - Q.z*R.z
    ///
    quaternion combined_quat
    {
        y*rot.z - z*rot.y + x*rot.w + w*rot.x,
        z*rot.x - x*rot.z + y*rot.w + w*rot.y,
        x*rot.y - y*rot.x + z*rot.w + w*rot.z,
        w*rot.w - x*rot.x - y*rot.y - z*rot.z
    };
    combined_quat.xyzw.normalize();
    return combined_quat;
}


float_3
quaternion::operator* (
    const float_3 vec) const
{
    /// 使用如下公式计算旋转后的向量V'或者点P':
    /// - V' := Q * V * Q^-1
    ///      := Q * V * Q*
    /// Q*为Q的Conjugate, 表示反向旋转: <-Q.x, -Q.y, -Q.z, Q.w>
    /// - 将向量V表示为四元数的形式: V := { <V.x, V.y, V.z> , 0 }
    ///   将向量P表示为四元数的形式: P := { <P.x, P.y, P.z> , 1 }
    /// 下面就向量进行旋转:
    ///
    /// V' := { <xI, yJ, zK>, 0 } := Q * V * Q*
    /// C := Q * V
    ///
    ///        (Q.y*V.z - Q.z*V.y + Q.w*V.x)*I
    /// C.v := (Q.z*V.x - Q.x*V.z + Q.w*V.y)*J
    ///        (Q.x*V.y - Q.y*V.x + Q.w*V.z)*K
    /// 
    /// C.w := -Q.x*V.x - Q.y*V.y - Q.z*V.z
    ///
    /// 计算V' := C * Q*
    /// Q* := <-Q.x, -Q.y, -Q.z, Q.w>
    ///          (-C.y*Q.z + C.z*Q.y + C.x*Q.w - C.w*Q.x)*I
    /// V'.v : = (-C.z*Q.x + C.x*Q.z + C.y*Q.w - C.w*Q.y)*J
    ///          (-C.x*Q.y + C.y*Q.x + C.z*Q.w - C.w*Q.z)*K
    ///    I := (1 - 2*Q.y*Q.y - 2*Q.z*Q.z)*V.x +
    ///         (2*Q.x*Q.y - 2*Q.z*Q.w)    *V.y +
    ///         (2*Q.x*Q.z + 2*Q.y*Q.w)    *V.z
    ///    J := (2*Q.x*Q.y + 2*Q.z*Q.w)    *V.x +
    ///         (1 - 2*Q.x*Q.x - 2*Q.z*Q.z)*V.y +
    ///         (2*Q.y*Q.z - 2*Q.x*Q.w)    *V.z
    ///    K := (2*Q.x*Q.z - 2*Q.y*Q.w)    *V.x +
    ///         (2*Q.y*Q.z + 2*Q.x*Q.w)    *V.y +
    ///         (1 - 2*Q.x*Q.x - 2*Q.y*Q.y)*V.z
    /// 
    /// V'.w := C.w*Q.w + C.x*Q.x + C.y*Q.y + C.z*Q.z
    ///      := (-Q.x*V.x - Q.y*V.y - Q.z*V.z)*Q.w +
    ///         ( Q.y*V.z - Q.z*V.y + Q.w*V.x)*Q.x +
    ///         ( Q.z*V.x - Q.x*V.z + Q.w*V.y)*Q.y +
    ///         ( Q.x*V.y - Q.y*V.x + Q.w*V.z)*Q.z
    ///      := 0
    ///
    /// 所以旋转矩阵为:
    ///      | 1 - 2*y*y - 2*z*z   2*x*y - 2*z*w       2*x*z + 2*y*w     |
    ///  R = | 2*x*y + 2*z*w       1 - 2*x*x - 2*z*z   2*y*z - 2*x*w     |
    ///      | 2*x*z - 2*y*w       2*y*z + 2*x*w       1 - 2*x*x - 2*y*y |
    ///
    const float xx2 = 2 * x * x;
    const float yy2 = 2 * y * y;
    const float zz2 = 2 * z * z;
    const float xy2 = 2 * x * y;
    const float xz2 = 2 * x * z;
    const float xw2 = 2 * x * w;
    const float yz2 = 2 * y * z;
    const float yw2 = 2 * y * w;
    const float zw2 = 2 * z * w;

    const float x = vec.x;
    const float y = vec.y;
    const float z = vec.z;
    const float rot_x = (1 - yy2 - zz2)*x + (xy2 - zw2)    *y + (xz2 + yw2)    *z;
    const float rot_y = (xy2 + zw2)    *x + (1 - xx2 - zz2)*y + (yz2 - xw2)    *z;
    const float rot_z = (xz2 - yw2)    *x + (yz2 + xw2)    *y + (1 - xx2 - yy2)*z;

    return float_3::make(rot_x, rot_y, rot_z);
}


float_3
quaternion::rot_axis () const
{
    /// 由于单位四元数可以表示为:
    /// {
    ///     Sin(ϕ/2)*U,
    ///     Cos(ϕ/2)
    /// }
    /// - 这里U为旋转轴, ϕ为旋转角度
    /// - 为了得到旋转轴, 只需要将Sin(ϕ/2)*U归一化

    /// 如果无旋转: { <0, 0, 0>, 1 }, 任意旋转轴(Y轴)
    if (*this == IDENTITY)
    {
        return float_3::UP;
    }
    else
    {
        return float_3::make(x, y, z).unified_vec();
    }
}


float
quaternion::rot_rads () const
{
    /// w := Cos(ϕ/2)
    /// MathUtility::fast_acos()返回值在[0, π](即: ϕ/2 ϵ [0, π])
    /// 可得到ϕ ϵ [0, 2π]
    const float half_rot_rads = MathUtility::fast_acos(w);
    return 2.0f * half_rot_rads;
}


float_3
quaternion::euler_angles ()const
{
    /// 参考:
    /// - Documents/Matrix_to_euler_derivation.txt
    ///
    /// NOTE: 计算的角度的范围:
    /// - Pitch: [-0.5π, +0.5π]
    /// - Yaw:   [   -π,    +π]
    /// - Roll:  [   -π,    +π]
    /// 例如: 对于Pitch 180°的旋转，我们可以通过先180° Yaw，再180° Roll来得到
    ///
    /// 我们内部的旋转角合成顺序: 偏航 * 俯仰 * 翻滚: Ry * Rx * Rz
    /// Ry * Rx * Rz的结果为:
    /// | e00   e01   e02 |       | cycr + syspsr    -cysr + syspcr   sycp |
    /// | e10   e11   e12 |   =   | cpsr             cpcr             -sp  |
    /// | e20   e21   e22 |       | -sycr + cyspsr   sysr + cyspcr    cycp |
    /// 我们需要处理一个特殊情况就是如果Pitch角度为±0.5π，此时将会产生异常(Singularity)
    ///
    /// sin(89.9°)
    static constexpr float SIN_89_9_DEGREES = 0.99999847691f;

    /// 任何一个四元数Q := { <x, y, z>, w }可以表示为:
    /// | e00   e01   e02 |       | 1 - 2*y*y - 2*z*z   2*x*y - 2*z*w       2*x*z + 2*y*w     |
    /// | e10   e11   e12 |   =   | 2*x*y + 2*z*w       1 - 2*x*x - 2*z*z   2*y*z - 2*x*w     |
    /// | e20   e21   e22 |       | 2*x*z - 2*y*w       2*y*z + 2*x*w       1 - 2*x*x - 2*y*y |
    ///
    /// e12 = 2*Q.y*Q.z - 2*Q.x*Q.w
    const float e12 = 2.0f * (y*z - x*w);

    /// Pitch: (-0.5π, +0.5π]
    if (e12 < SIN_89_9_DEGREES)
    {
        /// Pitch: (-0.5π, +0.5π)
        if (e12 > -SIN_89_9_DEGREES)
        {
            /// 如果Pitch ≠±0.5π:
            /// | e00   e01   e02 |       | cycr + syspsr    -cysr + syspcr   sycp |
            /// | e10   e11   e12 |   =   | cpsr             cpcr             -sp  |
            /// | e20   e21   e22 |       | -sycr + cyspsr   sysr + cyspcr    cycp |
            /// 
            /// Pitch = asine(-e12)
            /// Yaw   = actan2(e02, e22)
            /// Roll  = actan2(e10, e11)
            ///
            const float xx = x * x;
            const float yy = y * y;
            const float zz = z * z;
            // e02 = 2*Q.x*Q.z + 2*Q.y*Q.w
            const float e02 = 2 * (x*z + y*w);
            // e10 = 2*Q.x*Q.y + 2*Q.z*Q.w
            const float e10 = 2 * (x*y + z*w);
            // e11 = 1 - 2*Q.x*Q.x - 2*Q.z*Q.z
            const float e11 = 1 - 2 * (xx + zz);
            // e22 = 1 - 2*Q.x*Q.x - 2*Q.y*Q.y
            const float e22 = 1 - 2 * (xx + yy);

            /// NOTE: 角度的排列顺序:
            /// - x轴: 俯仰/Pitch角
            /// - y轴: 偏航/Yaw  角
            /// - z轴: 翻滚/Roll 角
            return float_3::make(std::asinf(-e12),
                                 std::atan2f(e02, e22),
                                 std::atan2f(e10, e11));
        }
        // Pitch ≈ +0.5π
        else
        {
            /// 由于e12为-sin(pitch), 对于角度在[+89.9, +90]，e12的值为[-1, -0.999]
            RUNTIME_ASSERT(e12 > -1 || MathUtility::equal(e12, -1.0f), "Invalid E12");

            /// 如果Pitch = 0.5π: Sine(Pitch) = 1
            /// | e00   e01   e02 |       | Cos(y-r)    Sin(y-r)   0  |
            /// | e10   e11   e12 |   =   | 0           0          -1 |
            /// | e20   e21   e22 |       | -Sin(y-r)   Cos(y-r)   0  |
            ///
            /// Yaw  = actan2(e01, e00)
            /// Roll = 0
            ///
            /// e00 = 1 - 2*Q.y*Q.y - 2*Q.z*Q.z
            const float e00 = 1 - 2 * (y*y + z*z);
            /// e01 = 2*Q.x*Q.y - 2*Q.z*Q.w
            const float e01 = 2 * (x*y - z*w);

            /// NOTE: 角度的排列顺序:
            /// - x轴: 俯仰/Pitch角
            /// - y轴: 偏航/Yaw  角
            /// - z轴: 翻滚/Roll 角
            return float_3::make(HALF_PI, std::atan2f(e01, e00), 0.0f);
        }
    }
    // Pitch ≈ -0.5π
    else
    {
        /// 由于e12为-sin(pitch), 对于角度在[-89.9, -90]，e12的值为[0.999.., 1]
        RUNTIME_ASSERT(e12 < 1 || MathUtility::equal(e12, 1.0f),"Invalid E12");

        /// 如果Pitch = -0.5π:
        /// | e00   e01   e02 |       | Cos(y+r)    -Sin(y+r)   0 |
        /// | e10   e11   e12 |   =   | 0           0           1 |
        /// | e20   e21   e22 |       | -Sin(y+r)   -Cos(y+r)   0 |
        ///
        /// Yaw  = actan2(-e01, e00)
        /// Roll = 0
        ///
        /// e00 = 1 - 2*Q.y*Q.y - 2*Q.z*Q.z
        const float e00 = 1 - 2 * (y*y + z*z);
        /// e01 = 2*Q.x*Q.y - 2*Q.z*Q.w
        const float e01 = 2 * (x*y - z*w);

        /// NOTE: 角度的排列顺序:
        /// - x轴: 俯仰/Pitch角
        /// - y轴: 偏航/Yaw  角
        /// - z轴: 翻滚/Roll 角
        return float_3::make(-HALF_PI, std::atan2f(-e01, e00), 0.0f);
    }
}


quaternion
quaternion::inverse () const
{
    /// 对于普通四元数Q以及它的Conjugate Q*: Q* := { -Q.v, w }, 从它们的乘积可知:
    /// Q * Q* = { <0*I, 0*J, 0*K, >, |Q|*|Q| }
    ///
    /// 由于Q * Q^-1 == Identity四元数 = { <0*I, 0*J, 0*K, >, 1 }
    /// 如果将四元数Q与Conjugate Q的乘机归一化:
    /// 即: Q * Q* = { <0*I, 0*J, 0*K, >, |Q|*|Q| } / |Q|^2 = Identity四元数
    /// 所以: Q * Q^-1 == Q * Q* / |Q|^2
    /// 得到:
    /// Q^-1 := Q* / |Q|^2
    ///
    /// 对于单位四元数: 由于|Q| == 1
    /// Q^-1 == Q*
    ///
    return quaternion{ -x, -y, -z, w };
}
