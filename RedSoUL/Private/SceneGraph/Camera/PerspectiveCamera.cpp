/// System headers
#include <cmath> /// std::atan
/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "DataType/Matrix3x4.hpp"
#include "Math/MathDefines.hpp"   /// DEGREE_TO_RADIAN
#include "Math/MathUtilities.hpp" /// equal, fast_sincos
#include "Memory/BlockAllocator.hpp"
#include "SceneGraph/MarkerTypeDepot.hpp"
/// Self header
#include "SceneGraph/Camera/PerspectiveCamera.hpp"


DEFINE_MARKER_TYPE_INFO(
    PerspectiveCamera, PerspectiveCamera::create, PerspectiveCamera::destroy, false);


float
PerspectiveCamera::field_of_view () const
{
    return m_field_of_view;
}


float
PerspectiveCamera::aspect_ratio () const
{
    return m_aspect_ratio;
}


float
PerspectiveCamera::near_plane_distance () const
{
    return m_near_plane_distance;
}


const matrix_4x4 &
PerspectiveCamera::world_to_clip_space_transform () const
{
    /// Camera --> Clip变换为:
    ///   Projection Matrix
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
    ///   Projection Matrix
    /// | e00  e01  e02  e03 |   | A  0  0  0 |
    /// | e10  e11  e12  e13 | = | 0  B  0  0 |
    /// | e20  e21  e22  e23 |   | 0  0  C  D |
    /// | e30  e31  e32  e33 |   | 0  0  1  0 |
    ///
    ///   | xscale  0       0       0      |   | 2*n/W  0          0         0         |
    /// = | 0       yscale  0       0      | = | 0      2*n/H*W/H  0         0         |
    ///   | 0       0       zscale  zshift |   | 0      0          -n/(f-n)  n*f/(f-n) |
    ///   | 0       0       1       0      |   | 0      0          1         0         |
    ///
    /// 如果使用水平Fov:
    /// | cot(Fov/2) 0                  0         0         |
    /// | 0          cot(Fov/2)*aspect  0         0         |
    /// | 0          0                  -n/(f-n)  n*f/(f-n) |
    /// | 0          0                  1         0         |
    /// 
    /// 对于Infinite Far, 最终因此投影矩阵为:
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
    /// World --> Clip的变换为:
    ///    Projection    *    World --> Camera
    /// | xs  0   0  0 |   | e00  e01  e02  e03 |   | xs*e00  xs*e01  xs*e02  xs*e03 |
    /// | 0   ys  0  0 | * | e10  e11  e12  e13 | = | ys*e10  ys*e11  ys*e12  ys*e13 |
    /// | 0   0   0  n |   | e20  e21  e22  e23 |   | 0       0       0       n      |
    /// | 0   0   1  0 |   | 0    0    0    1   |   | e20     e21     e22     e23    |
    ///
    /// 先Update投影变换
    if (m_is_projection_transform_dirty)
    {
        const matrix_3x4 & camera_transform = world_to_camera_transform();

        /// A = cotangent(Fov * 0.5)
        const float xscale = m_cotangent_half_fov;
        /// B = cotangent(Fov * 0.5) * aspect
        const float yscale = xscale * m_aspect_ratio;

        m_project_transform[0] = camera_transform[0] * xscale;
        m_project_transform[1] = camera_transform[1] * yscale;
        m_project_transform[2] = float_4::make(0.0f, 0.0f, 0.0f, m_near_plane_distance);
        m_project_transform[3] = camera_transform[2];

        m_is_projection_transform_dirty = false;
    }

    return m_project_transform;
}


void
PerspectiveCamera::set_field_of_view (
    const float fov_degress)
{
    /// 由于cotangent(HFov)在HFov为0附近无穷大,
    /// 因此Fov: (0, 180]
    RUNTIME_ASSERT(MathUtility::not_equal(fov_degress, 0.0f),
                   "Fov can not be ZERO degree!!");
    RUNTIME_ASSERT(fov_degress <= 180.0f,
                   "Fov can not be greater than 180 degrees!!");

    m_field_of_view = fov_degress;

    float sin_half_fov, cos_half_fov;
    MathUtility::fast_sincos(
        DEGREE_TO_RADIAN(fov_degress * 0.5f), sin_half_fov, cos_half_fov);
    m_cotangent_half_fov = cos_half_fov / sin_half_fov;

    m_is_projection_transform_dirty = true;
}


void
PerspectiveCamera::set_aspect_ratio (
    const float aspect_ratio)
{
    RUNTIME_ASSERT(aspect_ratio > 0.0f, "Aspect ratio must be positive!!");

    m_aspect_ratio = aspect_ratio;
    m_is_projection_transform_dirty = true;
}


void
PerspectiveCamera::set_near_plane_distance (
    const float distance)
{
    m_near_plane_distance = distance;
    m_is_projection_transform_dirty = true;
}


ObjectMarker *
PerspectiveCamera::create (
    SceneObject & marker_owner)
{
    /// 申请内存
    void * const new_marker =
        BlockAllocator<PerspectiveCamera, INIT_CAMERA_COUNT>::ref().allocate();
    if (new_marker)
    {
        /// 构建实例
        new(new_marker)PerspectiveCamera(marker_owner, 90.0f, 16.0f/9.0f, 0.1f);
    }

    return (ObjectMarker*)new_marker;
}


bool
PerspectiveCamera::destroy (
    ObjectMarker * & marker_object)
{
    RUNTIME_ASSERT(marker_object, "Marker can not be NULL!!");

    PerspectiveCamera * const camera = static_cast<PerspectiveCamera*>(marker_object);
    /// 调用析构函数
    camera->~PerspectiveCamera();
    /// 释放内存
    const bool opcode =
        BlockAllocator<PerspectiveCamera, INIT_CAMERA_COUNT>::ref().deallocate(
            marker_object);
    /// 清除参考
    marker_object = nullptr;
    return opcode;
}


PerspectiveCamera::PerspectiveCamera (
    SceneObject & marker_owner,
    const float   fov_degrees,
    const float   aspect_ratio,
    const float   near_plane_dist)
:
    SuperT(
        marker_owner, ms_type_info.marker_name_id(), CameraType::PERSPECTIVE_CAMERA),
    m_aspect_ratio(aspect_ratio),
    m_near_plane_distance(near_plane_dist)
{
    set_field_of_view(fov_degrees);
}


PerspectiveCamera::~PerspectiveCamera ()
{

}
