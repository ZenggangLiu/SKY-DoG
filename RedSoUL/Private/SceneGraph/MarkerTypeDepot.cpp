/// Library headers
#include "Assert/RuntimeAssert.hpp"
/// Self header
#include "SceneGraph/MarkerTypeDepot.hpp"


MarkerTypeDepot &
MarkerTypeDepot::ref ()
{
    static MarkerTypeDepot s_instance;
    return s_instance;
}


const MarkerTypeInfo *
MarkerTypeDepot::type_info (
    const StaticStringIdT marker_name_id) const
{
    const MarkerTypeTableT::const_iterator marker_info =
        m_type_table.find(marker_name_id);
    if (marker_info == m_type_table.end())
    {
        return nullptr;
    }
    else
    {
        return marker_info->second;
    }
}


void
MarkerTypeDepot::register_type (
    const StaticStringIdT  marker_name_id,
    const MarkerTypeInfo & marker_type_info)
{
    const auto marker_info = m_type_table.find(marker_name_id);
    if (marker_info == m_type_table.end())
    {
        m_type_table.emplace(marker_name_id, &marker_type_info);
    }
}


ObjectMarker *
MarkerTypeDepot::create_marker (
    SceneObject &         marker_owner,
    const StaticStringIdT marker_name_id)
{
    auto stored_info = m_type_table.find(marker_name_id);
    RUNTIME_ASSERT(stored_info != m_type_table.end(),
                   "Please register this marker type by calling "
                   "register_type() at first!!");

    if (stored_info != m_type_table.end())
    {
        /// 为抽象Marker: 例如CameraMarker
        if (stored_info->second->is_abstract())
        {
            RUNTIME_ASSERT(false, "Can not create abstract marker!!");
        }
        else
        {
            return stored_info->second->create_function()(marker_owner);
        }
    }
    return nullptr;
}


bool
MarkerTypeDepot::destroy_marker (
    const StaticStringIdT marker_name_id,
    ObjectMarker * &      marker_object)
{
    auto stored_info = m_type_table.find(marker_name_id);
    RUNTIME_ASSERT(stored_info != m_type_table.end(),
                   "Please register this marker type by calling "
                   "register_type() at first!!");

    if (stored_info != m_type_table.end())
    {
        /// 为抽象Marker: 例如CameraMarker
        if (stored_info->second->is_abstract())
        {
            RUNTIME_ASSERT(false, "Can not destroy abstract marker!!");
        }
        else
        {
            return stored_info->second->destroy_function()(marker_object);
        }
    }
    return false;
}
