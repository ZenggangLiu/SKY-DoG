/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Memory/BlockAllocator.hpp"
#include "SceneGraph/MarkerTypeDepot.hpp"
#include "SceneGraph/TransformMarker.hpp"
/// Self header
#include "SceneGraph/Light/PointLight.hpp"


DEFINE_MARKER_TYPE_INFO(
    PointLight, PointLight::create, PointLight::destroy, false);


ObjectMarker *
PointLight::create (
    SceneObject & marker_owner)
{
    /// 申请内存
    void* const new_marker =
        BlockAllocator<PointLight, INIT_POINT_LIGHT_COUNT>::ref().allocate();
    if (new_marker)
    {
        /// 构建实例
        new(new_marker)PointLight(marker_owner, float_3::ONE, 1.0f, 10.0f, true);
    }

    return (ObjectMarker*)new_marker;
}


bool
PointLight::destroy (
    ObjectMarker * & marker_object)
{
    RUNTIME_ASSERT(marker_object, "Marker can not be NULL!!");

    PointLight * const light = (PointLight*)marker_object;
    /// 调用析构函数
    light->~PointLight();
    /// 释放内存
    const bool opcode =
        BlockAllocator<PointLight, INIT_POINT_LIGHT_COUNT>::ref().deallocate(
            marker_object);
    /// 清除参考
    marker_object = nullptr;
    return opcode;
}


PointLight::PointLight (
    SceneObject & marker_owner,
    const float_3 light_color,
    const float   light_intensity,
    const float   light_range,
    const bool    does_cast_shadow)
:
    SuperT(marker_owner, ms_type_info.marker_name_id(), LightType::POINT_LIGHT,
           light_color, light_intensity, light_range, does_cast_shadow)
{

}


PointLight::~PointLight ()
{

}


