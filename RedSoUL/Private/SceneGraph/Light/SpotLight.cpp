/// Library headers"
#include "Assert/RuntimeAssert.hpp"
#include "Math/MathDefines.hpp" /// DEGREE_TO_RADIAN
#include "Math/MathUtilities.hpp"
#include "Memory/BlockAllocator.hpp"
#include "SceneGraph/MarkerTypeDepot.hpp"
#include "SceneGraph/TransformMarker.hpp"
/// Self header
#include "SceneGraph/Light/SpotLight.hpp"


DEFINE_MARKER_TYPE_INFO(
    Spotight, Spotight::create, Spotight::destroy, false);


float
Spotight::inner_falloff_angle () const
{
    return m_inner_angle_degs;
}


float
Spotight::outer_falloff_angle () const
{
    return m_outer_angle_degs;
}


void
Spotight::set_falloff_angles (
    const float inner_angle_degs,
    const float outer_angle_degs)
{
    RUNTIME_ASSERT(inner_angle_degs >  0.0f && inner_angle_degs < outer_angle_degs &&
                   outer_angle_degs <= 90.0f, "Falloff angle is invalid!!");

    m_inner_angle_degs = inner_angle_degs;
    m_outer_angle_degs = outer_angle_degs;

    m_inner_angle_cos = MathUtility::fast_cosine(DEGREE_TO_RADIAN(inner_angle_degs));
    m_outer_angle_cos = MathUtility::fast_cosine(DEGREE_TO_RADIAN(outer_angle_degs));
}


ObjectMarker *
Spotight::create (
    SceneObject & marker_owner)
{
    /// 申请内存
    void* const new_marker =
        BlockAllocator<Spotight, INIT_POINT_LIGHT_COUNT>::ref().allocate();
    if (new_marker)
    {
        /// 构建实例
        new(new_marker)
            Spotight(marker_owner, float_3::ONE, 1.0f, 21.8f, 30.0f, 10.0f, true);
    }

    return (ObjectMarker*)new_marker;
}


bool
Spotight::destroy (
    ObjectMarker * & marker_object)
{
    RUNTIME_ASSERT(marker_object, "Marker can not be NULL!!");

    Spotight * const light = (Spotight*)marker_object;
    /// 调用析构函数
    light->~Spotight();
    /// 释放内存
    const bool opcode =
        BlockAllocator<Spotight, INIT_POINT_LIGHT_COUNT>::ref().deallocate(
            marker_object);
    /// 清除参考
    marker_object = nullptr;
    return opcode;
}


Spotight::Spotight (
    SceneObject & marker_owner,
    const float_3 light_color,
    const float   light_intensity,
    const float   inner_angle_degs,
    const float   outer_angle_degs,
    const float   light_range,
    const bool    does_cast_shadow)
:
    SuperT(marker_owner, ms_type_info.marker_name_id(), LightType::SPOT_LIGHT,
           light_color, light_intensity, light_range, does_cast_shadow)
{
    set_falloff_angles(inner_angle_degs, outer_angle_degs);
}


Spotight::~Spotight ()
{

}


