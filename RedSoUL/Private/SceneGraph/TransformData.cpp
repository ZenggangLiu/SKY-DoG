/// System headers
#include <cmath> /// std::fmod
/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Math/MathDefines.hpp"     /// DEGREE_TO_RADIAN, RADIAN_TO_DEGREE
#include "Memory/BlockAllocator.hpp"
/// Self header
#include "SceneGraph/TransformData.hpp"


TransformData *
TransformData::create ()
{
    /// 申请内存
    void * const new_transform_data =
        BlockAllocator<TransformData, INIT_TRANSFORM_COUNT>::ref().allocate();
    if (new_transform_data)
    {
        /// 构建实例
        new(new_transform_data)TransformData();
    }
    return (TransformData*)new_transform_data;
}


bool
TransformData::destroy ()
{
    /// 调用析构函数
    this->~TransformData();
    /// 释放内存
    const bool opcode =
        BlockAllocator<TransformData, INIT_TRANSFORM_COUNT>::ref().deallocate(this);
    return opcode;
}


quaternion
TransformData::rotation () const
{
    /// 先更新
    update_combined_rotation();
    return m_combined_rotation;
}


matrix_3x4
TransformData::local_transform (
    const TransformType transform_type) const
{
    /// 各种变换组合的Matrix: '-'表示无, 'x'表示有
    /// Position       Rotatation   Scaling
    ///     -              -           -     <- ID
    ///     x              -           -     <- T
    ///     -              x           -     <- R
    ///     -              -           x     <- S
    ///     x              x           -     <- TR
    ///     x              -           x     <- TS
    ///     x              x           x     <- TRS
    ///     -              x           x     <- RS

    switch ((uint16_t)transform_type)
    {
        /// IDENTITY(ID)
        case (uint16_t)TransformType::IDENTITY_TRANSFROM:
        {
            return matrix_3x4::IDENTITY;
        }

        /// 只有位移(T)
        case (uint16_t)TransformType::HAS_POSITION:
        {
            return matrix_3x4::make_translation(m_local_position);
        }

        /// 只有旋转(R)
        case (uint16_t)TransformType::HAS_ROTATION:
        {
            return matrix_3x4::make_rotation(rotation());
        }

        /// 只有放缩(S)
        case (uint16_t)TransformType::HAS_SCALING:
        {
            return matrix_3x4::make_scaling(m_local_scaling);
        }

        /// 只有Uniform放缩(S)
        case (uint16_t)TransformType::HAS_SCALING |
             (uint16_t)TransformType::IS_UNIFORM_SCALING:
        {
            return matrix_3x4::make_scaling(m_local_scaling.x);
        }

        /// 位移 + 旋转(TR)
        case (uint16_t)TransformType::HAS_POSITION |
             (uint16_t)TransformType::HAS_ROTATION:
        {
            return matrix_3x4::make_translation_rotation(m_local_position, rotation());
        }

        /// 位移 + 放缩(TS)
        case (uint16_t)TransformType::HAS_POSITION |
             (uint16_t)TransformType::HAS_SCALING:
        {
            return matrix_3x4::make_translation_scaling(
                m_local_position, m_local_scaling);
        }

        /// 位移 + Uniform放缩(TS)
        case (uint16_t)TransformType::HAS_POSITION |
             (uint16_t)TransformType::HAS_SCALING  |
             (uint16_t)TransformType::IS_UNIFORM_SCALING:
        {
            return matrix_3x4::make_translation_scaling(
                m_local_position, m_local_scaling.x);
        }

        /// 位移 + 旋转 + 放缩(TRS)
        case (uint16_t)TransformType::HAS_POSITION |
             (uint16_t)TransformType::HAS_ROTATION |
             (uint16_t)TransformType::HAS_SCALING:
        {
            return matrix_3x4::make_translation_rotation_scaling(
                m_local_position, rotation(), m_local_scaling);
        }

        /// 位移 + 旋转 + Uniform放缩(TRS)
        case (uint16_t)TransformType::HAS_POSITION |
             (uint16_t)TransformType::HAS_ROTATION |
             (uint16_t)TransformType::HAS_SCALING  |
             (uint16_t)TransformType::IS_UNIFORM_SCALING:
        {
            return matrix_3x4::make_translation_rotation_scaling(
                m_local_position, rotation(), m_local_scaling.x);
        }

        /// 旋转 + 放缩(RS)
        case (uint16_t)TransformType::HAS_ROTATION |
             (uint16_t)TransformType::HAS_SCALING:
        {
            return matrix_3x4::make_rotation_scaling(rotation(), m_local_scaling);
        }

        /// 旋转 + Uniform放缩(RS)
        case (uint16_t)TransformType::HAS_ROTATION |
             (uint16_t)TransformType::HAS_SCALING  |
             (uint16_t)TransformType::IS_UNIFORM_SCALING:
        {
            return matrix_3x4::make_rotation_scaling(rotation(), m_local_scaling.x);
        }

        case (uint16_t)TransformType::IS_TRANSFORM_FROZEN:
        {
            return matrix_3x4::IDENTITY;
        }

        default:
        {
            RUNTIME_ASSERT(false, "Uknown Transform type!!");
            return matrix_3x4::IDENTITY;
        }
    }
}


void
TransformData::set_position (
    const float_3 position)
{
    RUNTIME_ASSERT(m_local_position != position, "We can not set the same position!!");

    /// 保存指定位移
    m_local_position = position;
}


void
TransformData::set_pitch (
    const float angle_degs)
{
    set_euler_angle(angle_degs, m_local_rotation.x);
}


void
TransformData::set_yaw (
    const float angle_degs)
{
    set_euler_angle(angle_degs, m_local_rotation.y);
}


void
TransformData::set_roll (
    const float angle_degs)
{
    set_euler_angle(angle_degs, m_local_rotation.z);
}


void
TransformData::set_rotation (
    const quaternion rotation)
{
    RUNTIME_ASSERT(m_combined_rotation != rotation, "We can not set the same angles!!");

    /// 保存新的角度
    m_combined_rotation = rotation;
    /// 计算Euler角, 并转换成角度
    const float_3 euler_rads = m_combined_rotation.euler_angles();
    m_local_rotation = float_3::make(
        RADIAN_TO_DEGREE(euler_rads.x),
        RADIAN_TO_DEGREE(euler_rads.y),
        RADIAN_TO_DEGREE(euler_rads.z));
    /// 设置Dirty标记
    m_is_combined_rotation_dirty = false;
}


void
TransformData::set_scaling (
    const float_3 scaling)
{
    RUNTIME_ASSERT(m_local_scaling != scaling, "We can not set the same scaling!!");

    /// 保存指定缩放
    m_local_scaling = scaling;
}


TransformData::TransformData ()
:
    m_local_position(float_3::ZERO),
    m_local_rotation(float_3::ZERO),
    m_local_scaling(float_3::ONE),
    m_combined_rotation(quaternion::IDENTITY),
    m_is_combined_rotation_dirty(false)
{

}


TransformData::~TransformData ()
{

}


void
TransformData::set_euler_angle (
    const float angle_degs,
    float &     stored_loc)
{
    RUNTIME_ASSERT(stored_loc != std::fmod(angle_degs, 360.0f),
                   "We can not set the same angle!!");

    /// 保存新的角度: 确保角度在[0, 360)之间
    stored_loc = std::fmod(angle_degs, 360.0f);
    /// 设置Dirty标记
    m_is_combined_rotation_dirty = true;
}


void
TransformData::update_combined_rotation () const
{
    if (m_is_combined_rotation_dirty)
    {
        /// 计算合成的旋转
        /// - 合成顺序: Yaw(Y)* Pitch(X)* Roll(Z)
        const quaternion pitch_rotation =
        MathUtility::equal(m_local_rotation.x, 0.0f) ?
        quaternion::IDENTITY                           :
        quaternion::make(AxisType::X_AXIS_TYPE,
                         DEGREE_TO_RADIAN(m_local_rotation.x));
        const quaternion yaw_rotation =
        MathUtility::equal(m_local_rotation.y, 0.0f) ?
        quaternion::IDENTITY                           :
        quaternion::make(AxisType::Y_AXIS_TYPE,
                         DEGREE_TO_RADIAN(m_local_rotation.y));
        const quaternion roll_rotation =
        MathUtility::equal(m_local_rotation.z, 0.0f) ?
        quaternion::IDENTITY                           :
        quaternion::make(AxisType::Z_AXIS_TYPE,
                         DEGREE_TO_RADIAN(m_local_rotation.z));

        m_combined_rotation = yaw_rotation * pitch_rotation * roll_rotation;
        m_is_combined_rotation_dirty = false;
    }
}
