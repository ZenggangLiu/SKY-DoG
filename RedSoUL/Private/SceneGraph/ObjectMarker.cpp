/// Library headers
#include "SceneGraph/MarkerTypeDepot.hpp"
/// Self header
#include "SceneGraph/ObjectMarker.hpp"


#if (BUILD_MODE == DEBUG_BUILD_MODE)
MarkerTypeInfo::MarkerTypeInfo (
    const char * const   marker_name,
    const CreateFuncPtr  create_func,
    const DestroyFuncPtr destroy_func,
    const bool           support_multi)
:
    m_create_func(create_func),
    m_destroy_func(destroy_func),
    m_marker_name(marker_name),
    m_support_multiple(support_multi)
{
    /// 注册此类型
    MarkerTypeDepot::ref().register_type(m_marker_name.id(), *this);
}
#else
MarkerTypeInfo::MarkerTypeInfo (
    const StaticStringIdT marker_name_id,
    const CreateFuncPtr   create_func,
    const DestroyFuncPtr  destroy_func,
    const bool            support_multi)
:
    m_create_func(create_func),
    m_destroy_func(destroy_func),
    m_marker_name_id(marker_name_id),
    m_support_multiple(support_multi)
{
    MarkerTypeDepot::ref().register_type(m_marker_name_id, *this);
}
#endif


bool
MarkerTypeInfo::isa (
    const StaticStringIdT type_name_id) const
{
#if (BUILD_MODE == DEBUG_BUILD_MODE)
    const bool is_same_type = type_name_id == m_marker_name.id();
#else
    const bool is_same_type = type_name_id == m_marker_name_id;
#endif

    /// TODO: 子类检测
    return is_same_type;
}


bool
MarkerTypeInfo::is_abstract () const
{
    return m_create_func == nullptr || m_destroy_func == nullptr;
}


bool
MarkerTypeInfo::not_support_multiple () const
{
    return support_multiple() == false;
}


bool
MarkerTypeInfo::support_multiple () const
{
    return m_support_multiple;
}


CreateFuncPtr
MarkerTypeInfo::create_function () const
{
    return m_create_func;
}


DestroyFuncPtr
MarkerTypeInfo::destroy_function () const
{
    return m_destroy_func;
}


#if (BUILD_MODE == DEBUG_BUILD_MODE)
StaticStringIdT
MarkerTypeInfo::marker_name_id() const
{
    return m_marker_name.id();
}
#else
StaticStringIdT
MarkerTypeInfo::marker_name_id () const
{
    return m_marker_name_id;
}
#endif



ObjectMarker::ObjectMarker (
    SceneObject &         marker_owner,
    const StaticStringIdT marker_name_id)
:
    m_marker_owner(marker_owner),
    m_marker_name_id(marker_name_id)
{

}


ObjectMarker::~ObjectMarker ()
{

}
