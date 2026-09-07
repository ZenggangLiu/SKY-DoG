/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "DataType/Matrix3x4.hpp"
#include "Memory/BlockAllocator.hpp"
#include "SceneGraph/MarkerTypeDepot.hpp"
/// Self header
#include "SceneGraph/Camera/OrthogonalCamera.hpp"


DEFINE_MARKER_TYPE_INFO(
    OrthogonalCamera, OrthogonalCamera::create, OrthogonalCamera::destroy, false);


float
OrthogonalCamera::view_volume_width () const
{
    return m_view_volume_width;
}


float
OrthogonalCamera::view_volume_height () const
{
    return m_view_volume_height;
}


float
OrthogonalCamera::near_plane_distance () const
{
    return m_near_plane_distance;
}


float
OrthogonalCamera::far_plane_distance () const
{
    return m_far_plane_distance;
}


const matrix_4x4 &
OrthogonalCamera::world_to_clip_space_transform () const
{
    /// Camera --> Clip变换为:
    ///   Projection Matrix
    /// | e00  e01  e02  e03 |   | A  0  0  0 |
    /// | e10  e11  e12  e13 | = | 0  B  0  0 |
    /// | e20  e21  e22  e23 |   | 0  0  C  D |
    /// | e30  e31  e32  e33 |   | 0  0  0  1 |
    ///
    /// 对于相机空间中的点P, 变换后的Clip空间点P'为:
    /// P_clip =       M         * P_camera =   P_clip
    ///          | A  0  0  0 |     | xe |    |  A*xe  |
    /// P'     = | 0  B  0  0 |  *  | ye |  = |  B*ye  |
    ///          | 0  0  C  D |     | ze |    | C*ze+D |
    ///          | 0  0  0  1 |     | 1  |    |   1    |
    ///
    /// 最终的投影点(NDC空间)P''与P'相同(因为Wc == 1)
    ///
    ///       |  A*xe  |
    /// P'' = |  B*ye  |
    ///       | C*ze+D |
    ///       |   1    |
    ///
    /// 为了实现: A*xe 在[-1, +1]之间,     B*ye 在[-1, +1]之间
    /// 可以得到: A 在 [-1/xe, +1/xe]之间, B 在 [-1/ye, +1/ye]之间
    ///
    /// 对于位于深度z处最右侧点P来说:
    ///                P(W*0.5, H*0.5, ze)
    /// -+------+------o-----------
    ///  |      |      |
    ///  |      |      |
    /// -+------+------o--------^--
    ///  |      |      |        n
    ///  |      |      |        e
    ///  |      |      |        a
    ///  |      |      |        r
    ///  +------C------+--------v--> x
    ///
    /// 由于A*xe = 1, B*ye = 1
    /// 变换得到:
    /// - A = 1/xe = 1/(W*0.5) = 2/W
    /// - B = 1/ye = 1/(H*0.5) = 2/H
    ///
    /// 此处W, H为: View Volume的尺寸
    /// H = W / aspect
    ///
    /// 为了实现在NDC中, Near映射到1, Far映射到0
    /// C*near + D = 1 并且 C*Far + D = 0
    /// 可以得到:
    /// C*(Far - near) = -1
    /// - C = -1/(Far - near)
    /// - D = Far/(Far - near)
    ///
    /// 最终投影矩阵:
    ///   Projection Matrix
    /// | e00  e01  e02  e03 |   | A  0  0  0 |   | 2/W  0      0         0       |
    /// | e10  e11  e12  e13 | = | 0  B  0  0 | = | 0     2*/H  0         0       |
    /// | e20  e21  e22  e23 |   | 0  0  C  D |   | 0     0     -1/(f-n)  f/(f-n) |
    /// | e30  e31  e32  e33 |   | 0  0  0  1 |   | 0     0     0         1       |
    ///
    /// World --> Clip的变换为:
    ///    Projection    *    World --> Camera
    /// | xs  0   0  0  |   | e00  e01  e02  e03 |
    /// | 0   ys  0  0  | * | e10  e11  e12  e13 |
    /// | 0   0   zs zt |   | e20  e21  e22  e23 |
    /// | 0   0   0  1  |   | 0    0    0    1   |

    ///   | xs*e00  xs*e01  xs*e02  xs*e03      |
    /// = | ys*e10  ys*e11  ys*e12  ys*e13      |
    ///   | zs*e20  zs*e21  zs*e22  zs*e23 + zt |
    ///   | 0       0       0       1           |
    ///
    /// 先Update投影变换
    if (m_is_projection_transform_dirty)
    {
        const matrix_3x4 & camera_transform = world_to_camera_transform();

        /// A = 2/W
        const float xscale = m_inverse_view_width_x2;
        /// B = 2/H
        const float yscale = m_inverse_view_height_x2;
        /// C = -1/(Far - near)
        const float zscale = -m_inverse_distance_delta;
        /// D = Far/(Far - near)
        const float ztranslate = m_far_plane_distance * m_inverse_distance_delta;

        m_project_transform[0] = camera_transform[0] * xscale;
        m_project_transform[1] = camera_transform[1] * yscale;
        m_project_transform[2] = camera_transform[2] * zscale;
        m_project_transform[3] = float_4::make(0.0f, 0.0f, 0.0f, 1.0f);
        m_project_transform.e23 += ztranslate;

        m_is_projection_transform_dirty = false;
    }

    return m_project_transform;
}


void
OrthogonalCamera::set_view_volume_width (
    const float width)
{
    RUNTIME_ASSERT(width > 0.0f, "OrthogonalCamera's view width can not be ZERO!!");

    m_view_volume_width  = width;
    m_view_volume_height = width / m_aspect_ratio; /// H = W/aspect

    m_inverse_view_width_x2  = 2.0f / m_view_volume_width;
    m_inverse_view_height_x2 = 2.0f / m_view_volume_height;

    m_is_projection_transform_dirty = true;
}


void
OrthogonalCamera::set_aspect_ratio (
    const float aspect_ratio)
{
    RUNTIME_ASSERT(aspect_ratio > 0.0f, "Aspect ratio must be positive!!");

    m_aspect_ratio = aspect_ratio;

    m_view_volume_height     = m_view_volume_width / m_aspect_ratio; /// H = W/aspect
    m_inverse_view_height_x2 = 2.0f / m_view_volume_height;

    m_is_projection_transform_dirty = true;
}


void
OrthogonalCamera::set_near_plane_distance (
    const float distance)
{
    RUNTIME_ASSERT(MathUtility::not_equal(m_far_plane_distance, distance),
                   "Far plane and near plane can not be at the same distance!!");

    m_near_plane_distance    = distance;
    m_inverse_distance_delta = 1.0f / (m_far_plane_distance - m_near_plane_distance);

    m_is_projection_transform_dirty = true;
}


void
OrthogonalCamera::set_far_plane_distance (
    const float distance)
{
    RUNTIME_ASSERT(MathUtility::not_equal(m_near_plane_distance, distance),
                   "Far plane and near plane can not be at the same distance!!");

    m_far_plane_distance     = distance;
    m_inverse_distance_delta = 1.0f / (m_far_plane_distance - m_near_plane_distance);

    m_is_projection_transform_dirty = true;
}


ObjectMarker *
OrthogonalCamera::create (
    SceneObject & marker_owner)
{
    /// 申请内存
    void * const new_marker =
        BlockAllocator<OrthogonalCamera, INIT_CAMERA_COUNT>::ref().allocate();
    if (new_marker)
    {
        /// 构建实例
        new(new_marker)OrthogonalCamera(marker_owner, 5.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    }

    return (ObjectMarker*)new_marker;
}


bool
OrthogonalCamera::destroy (
    ObjectMarker * & marker_object)
{
    RUNTIME_ASSERT(marker_object, "Marker can not be NULL!!");

    OrthogonalCamera * const camera = static_cast<OrthogonalCamera*>(marker_object);
    /// 调用析构函数
    camera->~OrthogonalCamera();
    /// 释放内存
    const bool opcode =
        BlockAllocator<OrthogonalCamera, INIT_CAMERA_COUNT>::ref().deallocate(
            marker_object);
    /// 清除参考
    marker_object = nullptr;
    return opcode;
}


OrthogonalCamera::OrthogonalCamera (
    SceneObject & marker_owner,
    const float   view_width,
    const float   aspect_ratio,
    const float   near_plane_dist,
    const float   far_plane_dist)
:
    SuperT(
        marker_owner, ms_type_info.marker_name_id(), CameraType::ORTHOGONAL_CAMEAR),
    m_view_volume_width(view_width),
    m_aspect_ratio(aspect_ratio),
    m_near_plane_distance(near_plane_dist),
    m_far_plane_distance(far_plane_dist),
    m_view_volume_height(m_view_volume_width / m_aspect_ratio), /// H = W/aspect
    m_inverse_view_width_x2 (2.0f / m_view_volume_width),
    m_inverse_view_height_x2(2.0f / m_view_volume_height),
    m_inverse_distance_delta(1.0f / (m_far_plane_distance - m_near_plane_distance))
{
    RUNTIME_ASSERT(aspect_ratio > 0.0f, "Aspect ratio must be positive!!");
    RUNTIME_ASSERT(MathUtility::not_equal(far_plane_dist, near_plane_dist),
                   "Far plane and near plane can not be at the same distance!!");

    m_is_projection_transform_dirty = true;
}


OrthogonalCamera::~OrthogonalCamera ()
{

}
