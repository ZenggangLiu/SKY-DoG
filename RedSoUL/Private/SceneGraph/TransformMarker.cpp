/// System headers
#include <algorithm> /// std::find
#include <cmath>     /// std::fmod
/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Common/CompilerDefines.hpp" /// BUILD_MODE
#include "Math/MathUtilities.hpp"     /// equal
#include "Memory/BlockAllocator.hpp"
#include "SceneGraph/GameScene.hpp"
#include "SceneGraph/MarkerTypeDepot.hpp"
#include "SceneGraph/SceneObject.hpp"
#include "SceneGraph/TransformData.hpp"
/// Self header
#include "SceneGraph/TransformMarker.hpp"


// MARK: == TransformMarker ==
/// const MarkerTypeInfo TransformMarker::ms_type_info(...);
DEFINE_MARKER_TYPE_INFO(
    TransformMarker, TransformMarker::create, TransformMarker::destroy, false);

const MessageId TransformMarker::ms_message_id = SceneObject::next_marker_message_id();


MessageId
TransformMarker::message_id ()
{
    return ms_message_id;
}


const TransformMarker *
TransformMarker::father () const
{
    return m_father_transform;
}


TransformMarker *
TransformMarker::father ()
{
    return const_cast<TransformMarker*>(((const TransformMarker*)this)->father());
}


void
TransformMarker::set_father (
    TransformMarker * const father /* = nullptr */)
{
    RUNTIME_ASSERT(m_transform_type != TransformType::IS_TRANSFORM_FROZEN,
                   "Transform is frozen, we can not change the translate!!");

    /// 如果为静止物体
    if (m_transform_type == TransformType::IS_TRANSFORM_FROZEN)
    {
        return;
    }
    else
    {
        // 检测是否企图设置相同的父节点
        if (m_father_transform == father)
        {
            return;
        }
        else
        {
            /// 检查如下非法情况: 沿father节点上找当前节点(THIS)，能挡住“自己作为父节点”和“把自己的后代作为父节点”
            /// - 将自己设置为父节点
            /// - 将子节点置为父节点
            const TransformMarker * ancestor = father;
            while (ancestor)
            {
                if (ancestor == this)
                {
                    RUNTIME_ASSERT(false, "Father can not be itself or its kinder!!");
                    return;
                }
                else
                {
                    ancestor = ancestor->m_father_transform;
                }
            }

            /// 先将此节点从父节点中Detach
            if (m_father_transform)
            {
                m_father_transform->detach(*this);
            }

            /// 如果有父节点, Attach到新的父节点中
            if (father)
            {
                father->attach(*this);
            }
        }
    }
}


uint32_t
TransformMarker::kinder_count () const
{
    return (uint32_t)m_kinder_list.size();
}


const TransformMarker *
TransformMarker::kinder_at (
    const uint32_t index) const
{
    RUNTIME_ASSERT(index < kinder_count(), "Kinder index is out of range!!");

    return m_kinder_list[index];
}


TransformMarker *
TransformMarker::kinder_at (
    const uint32_t index)
{
    return const_cast<TransformMarker*>(
        ((const TransformMarker*)this)->kinder_at(index));
}


float_3
TransformMarker::local_point_to_world_space (
    const float_3 local_point) const
{
    const matrix_3x4 & world_transform = local_to_world_transform();
    return world_transform.transform_point(local_point);
}


float_3
TransformMarker::world_point_to_local_space (
    const float_3 world_point) const
{
    /// Local --> World的变换为:
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
    ///
    /// World --> Local的变换为: 通过对上述各个成员变换进行取逆
    ///   Inverse(TRS) Matrix  =    Inverse(Scaling)     * Inverse(Rotation) * Inverse(Translation)
    /// | e00  e01  e02  e03 |   | 1/Sx  0     0     0 |   | Ux  Uy  Uz  0 |   | 1  0  0  -Tx |
    /// | e10  e11  e12  e13 | = | 0     1/Sy  0     0 | * | Vx  Vy  Vz  0 | * | 0  1  0  -Ty |
    /// | e20  e21  e22  e23 |   | 0     0     1/Sz  0 |   | Wx  Wy  Wz  0 |   | 0  0  1  -Tz |
    ///
    /// | e00  e01  e02  e03 |   | Ux/Sx  Uy/Sx  Uz/Sx  0 |   | 1  0  0  -Tx |
    /// | e10  e11  e12  e13 | = | Vx/Sy  Vy/Sy  Vz/Sy  0 | * | 0  1  0  -Ty |
    /// | e20  e21  e22  e23 |   | Wx/Sz  Wy/Sz  Wz/Sz  0 |   | 0  0  1  -Tz |
    ///
    /// | e00  e01  e02  e03 |   | Ux/Sx  Uy/Sx  Uz/Sx  -(Ux/Sx*Tx + Uy/Sx*Ty + Uz/Sx*Tz) |
    /// | e10  e11  e12  e13 | = | Vx/Sy  Vy/Sy  Vz/Sy  -(Vx/Sy*Tx + Vy/Sy*Ty + Vz/Sy*Tz) |
    /// | e20  e21  e22  e23 |   | Wx/Sz  Wy/Sz  Wz/Sz  -(Wx/Sz*Tx + Wy/Sz*Ty + Wz/Sz*Tz) |
    ///
    /// | e00  e01  e02  e03 |   | Ux/Sx  Uy/Sx  Uz/Sx  -(U dot T)/Sx |
    /// | e10  e11  e12  e13 | = | Vx/Sy  Vy/Sy  Vz/Sy  -(V dot T)/Sy |
    /// | e20  e21  e22  e23 |   | Wx/Sz  Wy/Sz  Wz/Sz  -(W dot T)/Sz |
    ///
    /// 此物体的Local Frame为:
    /// NOTE: 上述中的T(<Tx, Ty, Tz>)为Local Frame的原点O的世界坐标
    ///   V
    ///   ^
    ///   |  / W
    ///   | /
    ///   |/
    ///   O-------> U
    /// 上述矩阵可以写为:
    /// | e00  e01  e02  e03 |   | U/Sx  -(U dot O)/Sx |
    /// | e10  e11  e12  e13 | = | V/Sy  -(V dot O)/Sy |
    /// | e20  e21  e22  e23 |   | W/Sz  -(W dot O)/Sz |
    ///
    /// 对于世界空间中的点P, 变换后的本地点P'为:
    /// P_local =            M            * P_world
    ///           | U/Sx  -(U dot O)/Sx |   | Px |   | (U dot P)/Sx - (U dot O)/Sx |
    /// P'      = | V/Sy  -(V dot O)/Sy | * | Py | = | (V dot P)/Sy - (V dot O)/Sy |
    ///           | W/Sz  -(W dot O)/Sz |   | Pz |   | (W dot P)/Sz - (W dot O)/Sz |
    ///           |  0           1      |   | 1  |
    ///
    ///      | (U dot P)/Sx - (U dot O)/Sx |   | (U dot P - U dot O)/Sx |
    /// P' = | (V dot P)/Sy - (V dot O)/Sy | = | (V dot P - V dot O)/Sy |
    ///      | (W dot P)/Sz - (W dot O)/Sz |   | (W dot P - W dot O)/Sz |
    ///
    ///      | (U dot P - U dot O)/Sx |   | [ U dot (P - O) ]/Sx |
    /// P' = | (V dot P - V dot O)/Sy | = | [ V dot (P - O) ]/Sy |
    ///      | (W dot P - W dot O)/Sz |   | [ W dot (P - O) ]/Sz |
    ///
    /// === 推导如果从世界变换计算逆变换 ===
    /// 如果世界变换矩阵为: U, V, W为单位向量, U' := Sx*U, V' := Sy*V, W' := Sz*W
    /// | e00  e01  e02  e03 |   | Sx*Ux  Sy*Vx  Sz*Wx  Ox |
    /// | e10  e11  e12  e13 | = | Sx*Uy  Sy*Vy  Sz*Wy  Oy |
    /// | e20  e21  e22  e23 |   | Sx*Uz  Sy*Vz  Sz*Wz  Oz |
    ///
    ///      | [ U dot (P - O) ]/Sx |   | [ U'/Sx dot (P - O) ]/Sx |   | [ U' dot (P - O) ]/(|U'|*|U'|) |
    /// P' = | [ V dot (P - O) ]/Sy | = | [ V'/Sy dot (P - O) ]/Sy | = | [ V' dot (P - O) ]/(|V'|*|V'|) |
    ///      | [ W dot (P - O) ]/Sz |   | [ W'/Sz dot (P - O) ]/Sz |   | [ W' dot (P - O) ]/(|W'|*|W'|) |
    ///
    /// 最终:
    ///           | [ U'_world dot (P_world - O_world) ]/(|U'_world|*|U'_world|) |
    /// P_local = | [ V'_world dot (P_world - O_world) ]/(|V'_world|*|V'_world|) |
    ///           | [ W'_world dot (P_world - O_world) ]/(|W'_world|*|W'_world|) |
    /// 世界变换矩阵为:
    /// | U'_world  V'_world  W'_world  O_world |

    /// 获取当前世界变换矩阵
    const matrix_3x4 & world_transform = local_to_world_transform();
    /// 合成O, U', V', W'
    const float_3 frame_origin =
        float_3::make(world_transform.e03, world_transform.e13, world_transform.e23);
    const float_3 scaled_right =
        float_3::make(world_transform.e00, world_transform.e10, world_transform.e20);
    const float_3 scaled_upward =
        float_3::make(world_transform.e01, world_transform.e11, world_transform.e21);
    const float_3 scaled_forward =
        float_3::make(world_transform.e02, world_transform.e12, world_transform.e22);

    /// 计算P-O
    const float_3 origin_to_point = world_point - frame_origin;

    /// 计算各个轴的长度平方
    const float right_axis_length_squared   = scaled_right.length_squared();
    const float upward_axis_length_squared  = scaled_upward.length_squared();
    const float forward_axis_length_squared = scaled_forward.length_squared();

    /// 计算P’:
    ///      | [ U' dot (P - O) ]/(|U'|^2) |
    /// P' = | [ V' dot (P - O) ]/(|V'|^2) |
    ///      | [ W' dot (P - O) ]/(|W'|^2) |
    return float_3::make(
        origin_to_point.dot(scaled_right  )/right_axis_length_squared,
        origin_to_point.dot(scaled_upward )/upward_axis_length_squared,
        origin_to_point.dot(scaled_forward)/forward_axis_length_squared);
}


const matrix_3x4 &
TransformMarker::local_to_world_transform () const
{
    /// 先Update世界变换
    if (m_is_world_transform_dirty)
    {
        /// 此物体无空间变换
        if (m_transform_type == TransformType::IDENTITY_TRANSFROM)
        {
            /// 有父变换
            if (m_father_transform)
            {
                const matrix_3x4 & father_to_world =
                    m_father_transform->local_to_world_transform();

                m_world_transform = father_to_world;
            }
            /// 无父变换
            else
            {
                m_world_transform = matrix_3x4::IDENTITY;
            }
        }
        /// 此物体有空间变换
        else
        {
            /// 有父变换
            if (m_father_transform)
            {
                const matrix_3x4 & father_to_world =
                    m_father_transform->local_to_world_transform();
                const matrix_3x4   local_to_father =
                    m_transform_data->local_transform(m_transform_type);

                m_world_transform = father_to_world * local_to_father;
            }
            /// 无父变换
            else
            {
                const matrix_3x4 local_to_father =
                    m_transform_data->local_transform(m_transform_type);

                m_world_transform = local_to_father;
            }
        }
        /// 先复位Dirty标记: 为了避免TriggerMessage的过程中再次调用
        /// local_to_world_transform()
        m_is_world_transform_dirty = false;
        /// 发送变换更新消息
        m_marker_owner.trigger_message(ms_message_id);
    }

    return m_world_transform;
}


float_3
TransformMarker::world_position() const
{
    /// 获得世界变换
    const matrix_3x4 & world_transform = local_to_world_transform();

    /// 世界变换矩阵为:
    ///    X    Y    Z    O
    /// | e00  e01  e02  e03 |
    /// | e10  e11  e12  e13 |
    /// | e20  e21  e22  e23 |
    ///
    /// Y(上|👆)
    /// ↑
    /// |   /Z(前)
    /// |  /
    /// | /
    /// o------→ X(右|👉)
    /// X列为世界右方, 即, 第一列
    /// Y列为世界上方, 即, 第二列
    /// Z列为世界朝向, 即, 第三列
    /// O列为世界位移, 即, 第四列
    ///
    return float_3::make(world_transform.e03, world_transform.e13, world_transform.e23);
}


float_3
TransformMarker::world_rightward_vector () const
{
    /// 获得世界变换
    const matrix_3x4 & world_transform = local_to_world_transform();

    /// 世界变换矩阵为:
    ///    X    Y    Z    O
    /// | e00  e01  e02  e03 |
    /// | e10  e11  e12  e13 |
    /// | e20  e21  e22  e23 |
    ///
    /// Y(上|👆)
    /// ↑
    /// |   /Z(前)
    /// |  /
    /// | /
    /// o------→ X(右|👉)
    /// X列为世界右方, 即, 第一列
    /// Y列为世界上方, 即, 第二列
    /// Z列为世界朝向, 即, 第三列
    /// O列为世界位移, 即, 第四列
    ///
    float_3 rightward_vector =
        float_3::make(world_transform.e00, world_transform.e10, world_transform.e20);
    rightward_vector.normalize();
    return rightward_vector;
}


float_3
TransformMarker::world_upward_vector () const
{
    /// 获得世界变换
    const matrix_3x4 & world_transform = local_to_world_transform();

    /// 世界变换矩阵为:
    ///    X    Y    Z    O
    /// | e00  e01  e02  e03 |
    /// | e10  e11  e12  e13 |
    /// | e20  e21  e22  e23 |
    ///
    /// Y(上|👆)
    /// ↑
    /// |   /Z(前)
    /// |  /
    /// | /
    /// o------→ X(右|👉)
    /// X列为世界右方, 即, 第一列
    /// Y列为世界上方, 即, 第二列
    /// Z列为世界朝向, 即, 第三列
    /// O列为世界位移, 即, 第四列
    ///
    float_3 upward_vector =
        float_3::make(world_transform.e01, world_transform.e11, world_transform.e21);
    upward_vector.normalize();
    return upward_vector;
}


float_3
TransformMarker::world_forward_vector () const
{
    /// 获得世界变换
    const matrix_3x4 & world_transform = local_to_world_transform();

    /// 世界变换矩阵为:
    ///    X    Y    Z    O
    /// | e00  e01  e02  e03 |
    /// | e10  e11  e12  e13 |
    /// | e20  e21  e22  e23 |
    ///
    /// Y(上|👆)
    /// ↑
    /// |   /Z(前)
    /// |  /
    /// | /
    /// o------→ X(右|👉)
    /// X列为世界右方, 即, 第一列
    /// Y列为世界上方, 即, 第二列
    /// Z列为世界朝向, 即, 第三列
    /// O列为世界位移, 即, 第四列
    ///
    float_3 forward_vector =
        float_3::make(world_transform.e02, world_transform.e12, world_transform.e22);
    forward_vector.normalize();
    return forward_vector;
}


float_3
TransformMarker::local_position () const
{
    const bool has_position =
        ((uint32_t)m_transform_type & (uint32_t)TransformType::HAS_POSITION) != 0;
    return has_position ? m_transform_data->position() : float_3::ZERO;
}


float
TransformMarker::local_pitch () const
{
    const bool has_rotation =
        ((uint32_t)m_transform_type & (uint32_t)TransformType::HAS_ROTATION) != 0;
    return has_rotation ? m_transform_data->pitch() : 0.0f;
}


float
TransformMarker::local_yaw () const
{
    const bool has_rotation =
        ((uint32_t)m_transform_type & (uint32_t)TransformType::HAS_ROTATION) != 0;
    return has_rotation ? m_transform_data->yaw() : 0.0f;
}


float
TransformMarker::local_roll () const
{
    const bool has_rotation =
        ((uint32_t)m_transform_type & (uint32_t)TransformType::HAS_ROTATION) != 0;
    return has_rotation ? m_transform_data->roll() : 0.0f;
}


quaternion
TransformMarker::local_rotation () const
{
    const bool has_rotation =
        ((uint32_t)m_transform_type & (uint32_t)TransformType::HAS_ROTATION) != 0;
    if (has_rotation)
    {
        return m_transform_data->rotation();
    }
    else
    {
        return quaternion::IDENTITY;
    }
}


float_3
TransformMarker::local_scaling () const
{
    const bool has_scaling =
        ((uint32_t)m_transform_type & (uint32_t)TransformType::HAS_SCALING) != 0;
    return has_scaling ? m_transform_data->scaling() : float_3::ONE;
}


void
TransformMarker::set_world_forward_vector (
    const float_3 world_point)
{
    /// 对上方向量(Y|👆)施加的微小偏移: 为了防止观察向量与上方向量相同
    static constexpr float_3 UP_VECTOR_SHIFT{ 0.006f, 0.006f, 0.006f };

    /// 将世界空间中的观察点转换到父节点空间
    const float_3 point_in_father_space =
        (m_father_transform == nullptr)
        ? world_point
        : m_father_transform->world_point_to_local_space(world_point);

    // === 以下所有计算均在父节点空间内进行 === //
    /// 计算Forwardt向量: 原点 --> TargetPnt
    const float_3 forward_vector =
        (point_in_father_space - local_position()).unified_vec();

    /// 获取本地旋转矩阵
    const quaternion rotate_quat = m_transform_data->rotation();
    const matrix_3x4 rotate_matx = matrix_3x4::make_rotation(rotate_quat);

    /// 旋转矩阵为:
    ///    X    Y    Z
    /// | e00  e01  e02  0 |
    /// | e10  e11  e12  0 |
    /// | e20  e21  e22  0 |
    ///
    /// Y(上|👆)
    /// ↑
    /// |   /Z(前)
    /// |  /
    /// | /
    /// o------→ X(右|👉)
    ///
    const float_3 composed_up_vector =
        UP_VECTOR_SHIFT +
        float_3::make(rotate_matx.e01, rotate_matx.e11, rotate_matx.e21);

    /// 计算X(右|👉)向量
    /// X := cross(Y, Z)
    const float_3 right_vector = composed_up_vector.cross(forward_vector).unified_vec();
    /// 重新计算Y(上|👆)向量
    /// Y := cross(Z, X)
    const float_3 up_vector = forward_vector.cross(right_vector).unified_vec();

    /// 合成旋转矩阵
    const matrix_3x4 updated_rotate_matx
    {
        right_vector.x, up_vector.x, forward_vector.x, 0.0f,
        right_vector.y, up_vector.y, forward_vector.y, 0.0f,
        right_vector.z, up_vector.z, forward_vector.z, 0.0f,
    };

    /// 获取旋转矩阵对应的四元数
    const quaternion updated_rotate_quat = updated_rotate_matx.rotation();
    set_local_rotation(updated_rotate_quat);
}


void
TransformMarker::set_local_position (
    const float_3 position)
{
    RUNTIME_ASSERT(m_transform_type != TransformType::IS_TRANSFORM_FROZEN,
                   "Transform is frozen, we can not change the translate!!");

    /// 如果为静止物体
    if (m_transform_type == TransformType::IS_TRANSFORM_FROZEN)
    {
        return;
    }
    else
    {
        /// 检查是否企图设置相同的位移
        if (m_transform_data->position() == position)
        {
            return;
        }
        else
        {
            /// 保存指定位移
            m_transform_data->set_position(position);

            /// 是否设置0位移
            if (position == float_3::ZERO)
            {
                /// 清除位移标记
                const uint32_t new_type =
                    (uint32_t)m_transform_type & ~((uint32_t)TransformType::HAS_POSITION);
                m_transform_type = (TransformType)new_type;
            }
            else
            {
                /// 添加位移标记
                const uint32_t new_type =
                    (uint32_t)m_transform_type | (uint32_t)TransformType::HAS_POSITION;
                m_transform_type = (TransformType)new_type;
            }

            /// 设置缓冲矩阵Dirty标记
            set_cache_dirty();
        }
    }
}


void
TransformMarker::set_local_pitch (
    const float angle_degs)
{
    RUNTIME_ASSERT(m_transform_type != TransformType::IS_TRANSFORM_FROZEN,
                   "Transform is frozen, we can not change the translate!!");

    /// 如果为静止物体
    if (m_transform_type == TransformType::IS_TRANSFORM_FROZEN)
    {
        return;
    }
    else
    {
        /// 检查是否企图设置相同的角度
        if (m_transform_data->pitch() == std::fmod(angle_degs, 360.0f))
        {
            return;
        }
        else
        {
            /// 设置角度
            m_transform_data->set_pitch(angle_degs);
            /// 设置旋转类型
            set_rotation_type();
            /// 设置缓冲矩阵Dirty标记
            set_cache_dirty();
        }
    }
}


void
TransformMarker::set_local_yaw (
    const float angle_degs)
{
    RUNTIME_ASSERT(m_transform_type != TransformType::IS_TRANSFORM_FROZEN,
                   "Transform is frozen, we can not change the translate!!");

    /// 如果为静止物体
    if (m_transform_type == TransformType::IS_TRANSFORM_FROZEN)
    {
        return;
    }
    else
    {
        /// 检查是否企图设置相同的角度
        if (m_transform_data->yaw() == std::fmod(angle_degs, 360.0f))
        {
            return;
        }
        else
        {
            /// 设置角度
            m_transform_data->set_yaw(angle_degs);
            /// 设置旋转类型
            set_rotation_type();
            /// 设置缓冲矩阵Dirty标记
            set_cache_dirty();
        }
    }
}


void
TransformMarker::set_local_roll (
    const float angle_degs)
{
    RUNTIME_ASSERT(m_transform_type != TransformType::IS_TRANSFORM_FROZEN,
                   "Transform is frozen, we can not change the translate!!");

    /// 如果为静止物体
    if (m_transform_type == TransformType::IS_TRANSFORM_FROZEN)
    {
        return;
    }
    else
    {
        /// 检查是否企图设置相同的角度
        if (m_transform_data->roll() == std::fmod(angle_degs, 360.0f))
        {
            return;
        }
        else
        {
            /// 设置角度
            m_transform_data->set_roll(angle_degs);
            /// 设置旋转类型
            set_rotation_type();
            /// 设置缓冲矩阵Dirty标记
            set_cache_dirty();
        }
    }
}


void
TransformMarker::set_local_rotation (
    const quaternion rotation)
{
    RUNTIME_ASSERT(m_transform_type != TransformType::IS_TRANSFORM_FROZEN,
                   "Transform is frozen, we can not change the translate!!");

    /// 如果为静止物体
    if (m_transform_type == TransformType::IS_TRANSFORM_FROZEN)
    {
        return;
    }
    else
    {
        /// 检查是否企图设置相同的角度
        if (m_transform_data->rotation() == rotation)
        {
            return;
        }
        else
        {
            /// 设置角度
            m_transform_data->set_rotation(rotation);
            /// 设置旋转类型
            set_rotation_type();
            /// 设置缓冲矩阵Dirty标记
            set_cache_dirty();
        }
    }
}


void
TransformMarker::set_local_scaling (
    const float_3 scaling)
{
    RUNTIME_ASSERT(m_transform_type != TransformType::IS_TRANSFORM_FROZEN,
                   "Transform is frozen, we can not change the translate!!");

    /// 如果为静止物体
    if (m_transform_type == TransformType::IS_TRANSFORM_FROZEN)
    {
        return;
    }
    else
    {
        /// 使用的缩放系数
        const float_3 scaling_used = scaling == float_3::ZERO ? float_3::ONE : scaling;

        /// 检查是否企图设置相同的缩放
        if (m_transform_data->scaling() == scaling_used)
        {
            return;
        }
        else
        {
            /// 设置缩放
            m_transform_data->set_scaling(scaling_used);

            /// 将缩放系数ONE视为无缩放
            if (scaling_used == float_3::ONE)
            {
                /// 清除放缩标记(以及可能的Uniform放缩标记)
                const uint32_t new_type =
                    (uint32_t)m_transform_type &
                    ~((uint32_t)TransformType::HAS_SCALING |
                     (uint32_t)TransformType::IS_UNIFORM_SCALING);
                m_transform_type = (TransformType)new_type;
            }
            else
            {
                /// 添加放缩标记
                const uint32_t new_type =
                    (uint32_t)m_transform_type | (uint32_t)TransformType::HAS_SCALING;
                m_transform_type = (TransformType)new_type;

                /// 判断是否为Uniform放缩
                if (MathUtility::equal(scaling_used.x, scaling_used.y) &&
                    MathUtility::equal(scaling_used.x, scaling_used.z))
                {
                    /// 添加Uniform缩放标记
                    const uint32_t new_type =
                        (uint32_t)m_transform_type |
                        (uint32_t)TransformType::IS_UNIFORM_SCALING;
                    m_transform_type = (TransformType)new_type;
                }
                else
                {
                    /// 清除Uniform缩放标记
                    const uint32_t new_type =
                        (uint32_t)m_transform_type &
                        ~((uint32_t)TransformType::IS_UNIFORM_SCALING);
                    m_transform_type = (TransformType)new_type;
                }
            }

            /// 设置缓冲矩阵Dirty标记
            set_cache_dirty();
        }
    }
}


ObjectMarker *
TransformMarker::create (
    SceneObject & marker_owner)
{
    /// 申请内存
    void * const new_marker =
        BlockAllocator<TransformMarker, INIT_TRANSFORM_MARKER_COUNT>::ref().allocate();
    if (new_marker)
    {
        new(new_marker)TransformMarker(marker_owner, nullptr);
    }

    return (ObjectMarker*)new_marker;
}


bool
TransformMarker::destroy (
    ObjectMarker * & marker_object)
{
    RUNTIME_ASSERT(marker_object, "Marker can not be NULL!!");

    TransformMarker * const transform = static_cast<TransformMarker*>(marker_object);
    /// 调用析构函数
    transform->~TransformMarker();
    /// 释放内存
    const bool opcode =
        BlockAllocator<TransformMarker, INIT_TRANSFORM_MARKER_COUNT>::ref().deallocate(
            marker_object);
    /// 清除参考
    marker_object = nullptr;
    return opcode;
}


TransformMarker::TransformMarker (
    SceneObject &           marker_owner,
    TransformMarker * const father_transform)
:
    SuperT(marker_owner, ms_type_info.marker_name_id()),
    m_world_transform(matrix_3x4::IDENTITY),
    m_father_transform(father_transform),
    m_transform_data(TransformData::create()),
    m_transform_type(TransformType::IDENTITY_TRANSFROM),
    m_is_world_transform_dirty(father_transform != nullptr)
{
    static constexpr uint8_t INITIAL_KINDER_COUNT = 8;

    RUNTIME_ASSERT(m_transform_data, "Can not create transform data!!");

    m_kinder_list.reserve(INITIAL_KINDER_COUNT);

    /// 连接父节点
    if (father_transform)
    {
        father_transform->attach(*this);
    }
    /// 无父节点变换
    else
    {
        /// 在关卡中注册此TOPLEVEL物体
        m_marker_owner.owner_scene().register_toplevel_object(m_marker_owner);
    }
}


TransformMarker::~TransformMarker ()
{
    /// 先销毁子节点
    while (m_kinder_list.size() > 0)
    {
        TransformMarker * const kinder = m_kinder_list.back();
        /// 断开此子节点
        detach(*kinder);
        /// 销毁子物体
        SceneObject * kinder_object = &(kinder->m_marker_owner);
        m_marker_owner.owner_scene().destroy_object(kinder_object);
    }

    /// 断开父节点
    if (m_father_transform)
    {
        m_father_transform->detach(*this);
    }
    /// 无父节点变换
    else
    {
        /// 在关卡中注销此TOPLEVEL物体
        m_marker_owner.owner_scene().unregister_toplevel_object(m_marker_owner);
    }

    /// 释放TransformData实例
    m_transform_data->destroy();
}


void
TransformMarker::attach (
    TransformMarker & new_kinder)
{
    /// 检查如下非法情况:
    /// - 将自己设置为子节点
    /// - 将父节点置为子节点
    const TransformMarker * ancestor = this;
    while (ancestor)
    {
        if (ancestor == &new_kinder)
        {
            RUNTIME_ASSERT(false, "Can not attach itself or an ancestor as kinder!!");
            return;
        }
        else
        {
            ancestor = ancestor->m_father_transform;
        }
    }

    /// 测试指定子节点是否已经Attach
    const KinderListT::iterator kinder_it =
        std::find(m_kinder_list.begin(), m_kinder_list.end(), &new_kinder);
    RUNTIME_ASSERT(kinder_it == m_kinder_list.end(),
                   "Same kinder can not be attached more than once!!");

    if (kinder_it == m_kinder_list.end())
    {
        /// 如果指定子节点为TOPLEVEL节点, 在关卡中TOPLEVEL列表中注销它
        if (new_kinder.m_father_transform == nullptr)
        {
            m_marker_owner.owner_scene().unregister_toplevel_object(new_kinder.m_marker_owner);
        }
        /// 添加指定子节点
        m_kinder_list.push_back(&new_kinder);
        /// 设置指定子节点的父节点
        new_kinder.m_father_transform = this;
        /// 设置Dirty标记
        new_kinder.set_cache_dirty();
    }
}


void
TransformMarker::detach (
    TransformMarker & old_kinder)
{
    /// 测试指定子节点是否已经Attach
    KinderListT::iterator kinder_it =
        std::find(m_kinder_list.begin(), m_kinder_list.end(), &old_kinder);
    RUNTIME_ASSERT(kinder_it != m_kinder_list.end(),
                   "This kinder is not attached at all!!");

    if (kinder_it != m_kinder_list.end())
    {
        /// 清楚指定子节点的父节点
        old_kinder.m_father_transform = nullptr;
        /// 设置Dirty标记
        old_kinder.set_cache_dirty();
        /// 移除指定子节点
        m_kinder_list.erase(kinder_it);
        /// 在关卡中TOPLEVEL列表中注册它
        m_marker_owner.owner_scene().register_toplevel_object(old_kinder.m_marker_owner);
    }
}


void
TransformMarker::set_cache_dirty ()
{
    /// 设置Kinder的Dirty标记
    for (auto kinder : m_kinder_list)
    {
        kinder->set_cache_dirty();
    }

    /// 设置自己的Dirty标记
    m_is_world_transform_dirty = true;
}


void
TransformMarker::set_rotation_type ()
{
    /// 判断是否还有旋转
    if (m_transform_data->euler_angles() == float_3::ZERO)
    {
        /// 清除旋转标记
        const uint32_t new_type =
            (uint32_t)m_transform_type & ~(uint32_t)TransformType::HAS_ROTATION;
        m_transform_type = (TransformType)new_type;
    }
    else
    {
        /// 添加旋转标记
        const uint32_t new_type =
            (uint32_t)m_transform_type | (uint32_t)TransformType::HAS_ROTATION;
        m_transform_type = (TransformType)new_type;
    }
}
