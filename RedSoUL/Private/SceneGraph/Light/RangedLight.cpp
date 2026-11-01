/// Library headers
#include "Assert/RuntimeAssert.hpp"
/// Self header
#include "SceneGraph/Light/RangedLight.hpp"


float
RangedLight::range () const
{
    return m_range;
}


void
RangedLight::set_range (
    const float range)
{
    RUNTIME_ASSERT(range > 0.0f, "Light range must be valid!!");

    if (range > 0.0f)
    {
        m_range = range;
    }
}


RangedLight::RangedLight (
    SceneObject &         marker_owner,
    const StaticStringIdT marker_name_id,
    const LightType       light_type,
    const float_3         light_color,
    const float           light_intensity,
    const float           light_range,
    const bool            does_cast_shadow)
:
    SuperT(marker_owner, marker_name_id, light_type,
           light_color, light_intensity, does_cast_shadow),
    m_range(light_range)
{

}


RangedLight::~RangedLight ()
{

}


