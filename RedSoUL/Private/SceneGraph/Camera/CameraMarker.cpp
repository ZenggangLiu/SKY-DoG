/// Library headers
#include "Math/MathUtilities.hpp" /// equal
#include "SceneGraph/SceneObject.hpp"
#include "SceneGraph/TransformMarker.hpp"
/// Self header
#include "SceneGraph/Camera/CameraMarker.hpp"


CameraType
CameraMarker::camera_type () const
{
    return m_camera_type;
}


float_3
CameraMarker::camera_point_to_world_space (
    const float_3 camera_point) const
{
    return m_transform_marker.local_point_to_world_space(camera_point);
}


float_3
CameraMarker::world_point_to_camera_space (
    const float_3 world_point) const
{
    return m_transform_marker.world_point_to_local_space(world_point);
}


CameraMarker::CameraMarker (
    SceneObject &         marker_owner,
    const StaticStringIdT marker_name_id,
    const CameraType      camera_type)
:
    SuperT(marker_owner, marker_name_id),
    m_camera_transform(matrix_3x4::IDENTITY),
    m_project_transform(matrix_4x4::IDENTITY),
    m_transform_marker(marker_owner.transform()),
    m_is_camera_transform_dirty(false),
    m_is_projection_transform_dirty(false),
    m_camera_type(camera_type)
{
    /// 注册Transform更新监听
    m_marker_owner.register_message_observer(
        TransformMarker::message_id(), this,
        [](const MessageId message_id, ObjectMarker * const marker_object) -> void
        {
            CameraMarker * const camera_marker =
                static_cast<CameraMarker*>(marker_object);
            if (message_id == TransformMarker::message_id())
            {
                camera_marker->on_world_transform_updated();
            }
        });
}


CameraMarker::~CameraMarker ()
{
    /// 注销Transform更新监听
    m_marker_owner.remove_message_observer(TransformMarker::message_id(), this);
}


const matrix_3x4 &
CameraMarker::world_to_camera_transform () const
{
    if (m_is_camera_transform_dirty)
    {
        /// Local --> World的变换为:
        ///       TRS Matrix       = Translation*Rotation*Scaling
        /// | e00  e01  e02  e03 |   | Sx*Ux  Sy*Vx  Sz*Wx  Tx |
        /// | e10  e11  e12  e13 | = | Sx*Uy  Sy*Vy  Sz*Wy  Ty |
        /// | e20  e21  e22  e23 |   | Sx*Uz  Sy*Vz  Sz*Wz  Tz |
        /// 这里: U, V, W为单位向量, 且相互垂直
        ///
        /// World --> Local的变换为: 通过对上述各个成员变换进行取逆
        ///   Inverse(TRS) Matrix  = Inverse(Scale)*Inverse(Rotate)*Inverse(Translate)
        /// | e00  e01  e02  e03 |   | Ux/Sx  Uy/Sx  Uz/Sx  -(U dot T)/Sx |
        /// | e10  e11  e12  e13 | = | Vx/Sy  Vy/Sy  Vz/Sy  -(V dot T)/Sy |
        /// | e20  e21  e22  e23 |   | Wx/Sz  Wy/Sz  Wz/Sz  -(W dot T)/Sz |
        ///
        /// 由于世界变换矩阵包含Scaling, 我们令 U' := Sx*U, V' := Sy*V, W' := Sz*W
        /// | e00  e01  e02  e03 |   | U'x  V'x  W'x  Tx |
        /// | e10  e11  e12  e13 | = | U'y  V'y  W'y  Ty |
        /// | e20  e21  e22  e23 |   | U'z  V'z  W'z  Tz |
        ///
        /// 因此逆矩阵:
        /// | Ux/Sx  Uy/Sx  Uz/Sx  -(U dot T)/Sx |   | U'x/Sx^2  U'y/Sx^2  U'z/Sx^2  -(U' dot T)/Sx^2 |
        /// | Vx/Sy  Vy/Sy  Vz/Sy  -(V dot T)/Sy | = | V'x/Sy^2  V'y/Sy^2  V'z/Sy^2  -(V' dot T)/Sy^2 |
        /// | Wx/Sz  Wy/Sz  Wz/Sz  -(W dot T)/Sz |   | W'x/Sz^2  W'y/Sz^2  W'z/Sz^2  -(W' dot T)/Sz^2 |
        /// 此处:
        /// Sx^2 = dot(U', U'), Sy^2 = dot(V', V'), Sz^2 = dot(W', W')
        ///
        /// 最终逆矩阵为:
        /// | e00  e01  e02  e03 |   | U'/dot(U', U')  -(U' dot T)/dot(U', U') |
        /// | e10  e11  e12  e13 | = | V'/dot(V', V')  -(V' dot T)/dot(V', V') |
        /// | e20  e21  e22  e23 |   | W'/dot(W', W')  -(W' dot T)/dot(W', W') |
        ///
        const matrix_3x4 & world_transform = m_transform_marker.local_to_world_transform();
        /// 合成 T, U', V', W'
        const float_3 origin_translate =
            float_3::make(world_transform.e03, world_transform.e13, world_transform.e23);
        const float_3 scaled_right_axis =
            float_3::make(world_transform.e00, world_transform.e10, world_transform.e20);
        const float_3 scaled_upward_axis =
            float_3::make(world_transform.e01, world_transform.e11, world_transform.e21);
        const float_3 scaled_forward_axis =
            float_3::make(world_transform.e02, world_transform.e12, world_transform.e22);

        /// 计算 dot(U', U'), dot(V', V'), dot(W', W')
        const float dot_u_prim_u_prim = scaled_right_axis.length_squared();
        const float dot_v_prim_v_prim = scaled_upward_axis.length_squared();
        const float dot_w_prim_w_prim = scaled_forward_axis.length_squared();

        /// 计算 1/dot(U', U'), 1/dot(V', V'), 1/dot(W', W')
        const float scaled_right_axis_scale =
            MathUtility::equal(dot_u_prim_u_prim, 0.0f) ?
            1.0f :
            1.0f / dot_u_prim_u_prim;
        const float scaled_upward_axis_scale =
            MathUtility::equal(dot_v_prim_v_prim, 0.0f) ?
            1.0f :
            1.0f / dot_v_prim_v_prim;
        const float scaled_forward_axis_scale =
            MathUtility::equal(dot_w_prim_w_prim, 0.0f) ?
            1.0f :
            1.0f / dot_w_prim_w_prim;

        m_camera_transform[0] = float_4::make
        (
            scaled_right_axis.x * scaled_right_axis_scale,
            scaled_right_axis.y * scaled_right_axis_scale,
            scaled_right_axis.z * scaled_right_axis_scale,
            -origin_translate.dot(scaled_right_axis) * scaled_right_axis_scale
        );
        m_camera_transform[1] = float_4::make
        (
            scaled_upward_axis.x * scaled_upward_axis_scale,
            scaled_upward_axis.y * scaled_upward_axis_scale,
            scaled_upward_axis.z * scaled_upward_axis_scale,
            -origin_translate.dot(scaled_upward_axis) * scaled_upward_axis_scale
        );
        m_camera_transform[2] = float_4::make
        (
            scaled_forward_axis.x * scaled_forward_axis_scale,
            scaled_forward_axis.y * scaled_forward_axis_scale,
            scaled_forward_axis.z * scaled_forward_axis_scale,
            -origin_translate.dot(scaled_forward_axis) * scaled_forward_axis_scale
        );

        /// 复位Dirty标记
        m_is_camera_transform_dirty = false;
    }

    return m_camera_transform;
}


void CameraMarker::on_world_transform_updated ()
{
    m_is_camera_transform_dirty = true;
}
