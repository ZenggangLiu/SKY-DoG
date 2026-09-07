/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Common/CompilerDefines.hpp" /// BUILD_MODE
#include "SceneGraph/Camera/OrthogonalCamera.hpp"
#include "SceneGraph/Camera/PerspectiveCamera.hpp"
#include "SceneGraph/MarkerTypeDepot.hpp"
#include "SceneGraph/TransformMarker.hpp"
#include "Text/StaticString.hpp"
/// Self header
#include "SceneGraph/SceneObject.hpp"


SceneObject::MessageTableT SceneObject::ms_message_table;


MessageId
SceneObject::next_marker_message_id ()
{
    static MessageId current_message_id{ 1 };

    return MessageId{ current_message_id.value++ };
}


const GameScene &
SceneObject::owner_scene () const
{
    return m_owner_scene;
}


GameScene &
SceneObject::owner_scene ()
{
    return const_cast<GameScene&>(((const SceneObject*)this)->owner_scene());
}


const TransformMarker &
SceneObject::transform () const
{
    const TransformMarker * const transform_marker = find_marker<TransformMarker>();
    RUNTIME_ASSERT(transform_marker, "SceneObject has no TransformMarker!!");

    return *transform_marker;
}


TransformMarker &
SceneObject::transform ()
{
    return const_cast<TransformMarker&>(((const SceneObject*)this)->transform());
}


OrthogonalCamera *
SceneObject::add_orthogonal_camera (
    const float view_width,
    const float aspect_ratio,
    const float near_plane_dist,
    const float far_plane_dist)
{
    OrthogonalCamera * const camera_marker = add_marker<OrthogonalCamera>();
    if (camera_marker)
    {
        camera_marker->set_view_volume_width(view_width);
        camera_marker->set_aspect_ratio(aspect_ratio);
        camera_marker->set_near_plane_distance(near_plane_dist);
        camera_marker->set_far_plane_distance(far_plane_dist);
    }

    return camera_marker;
}


PerspectiveCamera *
SceneObject::add_perspective_camera (
    const float fov_degrees,
    const float aspect_ratio,
    const float near_plane_dist)
{
    PerspectiveCamera * const camera_marker = add_marker<PerspectiveCamera>();
    if (camera_marker)
    {
        camera_marker->set_field_of_view(fov_degrees);
        camera_marker->set_aspect_ratio(aspect_ratio);
        camera_marker->set_near_plane_distance(near_plane_dist);
    }

    return camera_marker;
}


void
SceneObject::register_message_observer (
    const MessageId       message_id,
    ObjectMarker * const  marker_objc,
    const MessageFunction message_func)
{
    /// 创建Message Key
    const MessageKey msg_key(this, message_id);
    /// 查找是否有监听器注册在此Id上
    const MessageTableT::iterator observer_list = ms_message_table.find(msg_key);
    if (observer_list == ms_message_table.end())
    {
        MessageObserverInfoListT new_list;
        new_list.push_back(MessageObserverInfo(marker_objc, message_func));
        ms_message_table.emplace(msg_key, new_list);
    }
    /// 有监听器注册在此Id上: 确保监听器不可以重复注册
    else
    {
        MessageObserverInfoListT & registered_observers = observer_list->second;
#if (BUILD_MODE == DEBUG_BUILD_MODE)
        for (const auto & observer : registered_observers)
        {
            RUNTIME_ASSERT(observer.marker_objc != marker_objc,
                           "Double message observer registration!!");
        }
#endif
        registered_observers.push_back(MessageObserverInfo(marker_objc, message_func));
    }
}


void
SceneObject::remove_message_observer (
    const MessageId      message_id,
    ObjectMarker * const marker_objc)
{
    /// 创建Message Key
    const MessageKey msg_key(this, message_id);
    /// 查找是否有注册的监听器
    const MessageTableT::iterator observer_list = ms_message_table.find(msg_key);
    if (observer_list == ms_message_table.end())
    {
        RUNTIME_ASSERT(false, "The message observer is not registered!!");
    }
    else
    {
        MessageObserverInfoListT & registered_observers = observer_list->second;
        for (MessageObserverInfoListT::iterator observer = registered_observers.begin();
             observer != registered_observers.end(); ++observer)
        {
            if (observer->marker_objc == marker_objc)
            {
                registered_observers.erase(observer);
                if (registered_observers.empty())
                {
                    ms_message_table.erase(msg_key);
                }
                return;
            }
        }
        RUNTIME_ASSERT(false, "The message observer is not registered!!");
    }
}


void
SceneObject::trigger_message (
    const MessageId message_id)
{
    /// 创建Message Key
    const MessageKey msg_key(this, message_id);
    /// 查找是否有注册的监听器
    const MessageTableT::const_iterator observer_list = ms_message_table.find(msg_key);
    if (observer_list != ms_message_table.end())
    {
        for (auto & observer : observer_list->second)
        {
            observer.message_func(message_id, observer.marker_objc);
        }
    }
}


SceneObject::SceneObject (
    GameScene * const   owner_scene,
    SceneObject * const father_object)
:
    m_owner_scene(*owner_scene),
    m_self_name_id(StaticString::get_empty_string_id()),
    m_tag_name_id(StaticString::get_empty_string_id()),
    m_layer_id(0),
    m_is_enabled(true)
{
    TransformMarker * const transform_marker = add_marker<TransformMarker>();
    if (father_object)
    {
        father_object->transform().attach(*transform_marker);
    }
}


SceneObject::~SceneObject ()
{
    /// 使用MarkerTypeDepot来销毁所有属性
    for (auto marker : m_marker_list)
    {
        MarkerTypeDepot::ref().destroy_marker(marker->m_marker_name_id, marker);
    }
    m_marker_list.clear();
}


const ObjectMarker *
SceneObject::find_marker_with_nameid (
    const StaticStringIdT name_id) const
{
    if (name_id == INVALID_STATIC_STRING_ID)
    {
        return nullptr;
    }
    else
    {
        for (const auto marker : m_marker_list)
        {
            /// 获取当前Marker的TypeInfo
            const MarkerTypeInfo * const marker_info =
                MarkerTypeDepot::ref().type_info(marker->m_marker_name_id);
            if (marker_info)
            {
                /// 当前Marker为指定类型或者其子类
                if (marker_info->isa(name_id))
                {
                    return marker;
                }
            }
            else
            {
                RUNTIME_ASSERT(false, "Current marker does not have a type info!!");
                break;
            }
        }
        return nullptr;
    }
}


ObjectMarker *
SceneObject::find_marker_with_nameid (
    const StaticStringIdT name_id)
{
    return const_cast<ObjectMarker*>(
        ((const SceneObject*)this)->find_marker_with_nameid(name_id));
}


ObjectMarker *
SceneObject::add_marker_with_nameid (
    const StaticStringIdT name_id)
{
    /// 先判断是否此类Marker已经存在
    ObjectMarker * const exist_marker = find_marker_with_nameid(name_id);
    if (exist_marker)
    {
        /// 获取对应的TypeInfo
        const MarkerTypeInfo * const marker_info =
            MarkerTypeDepot::ref().type_info(name_id);
        if (marker_info)
        {
            if (marker_info->not_support_multiple())
            {
                RUNTIME_ASSERT(false,
                               "Can not create multiple instances of this Marker!!");
                return exist_marker;
            }
        }
        else
        {
            RUNTIME_ASSERT(false, "Marker does not have a type info!!");
            return nullptr;
        }
    }

    /// 创建新的Marker实例
    ObjectMarker * const marker = MarkerTypeDepot::ref().create_marker(*this, name_id);
    RUNTIME_ASSERT(marker, "Can not create new marker!!");
    if (marker)
    {
        m_marker_list.push_back(marker);
    }
    return marker;
}
