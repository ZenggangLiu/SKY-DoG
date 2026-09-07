/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Memory/BlockAllocator.hpp"
#include "Render/RenderMeshDepot.hpp"
#include "SceneGraph/MarkerTypeDepot.hpp" /// DEFINE_MARKER_TYPE_INFO
#include "SceneGraph/SceneObject.hpp"
/// Self header
#include "SceneGraph/Geometry/GeometryData.hpp"


DEFINE_MARKER_TYPE_INFO(
    GeometryData, GeometryData::create, GeometryData::destroy, false);


RenderMeshIdT
GeometryData::geometry_id () const
{
    return m_geometry_id;
}


bool
GeometryData::create_cube ()
{
    m_geometry_id = RenderMeshDepot::ref().create_unit_cube();
    return true;
}


bool
GeometryData::create_beveled_cube ()
{
    m_geometry_id = RenderMeshDepot::ref().create_unit_beveled_cube();
    return true;
}


bool
GeometryData::create_icosphere ()
{
    m_geometry_id = RenderMeshDepot::ref().create_unit_icosphere();
    return true;
}


bool
GeometryData::create_geometry (
    const char * const  abs_file_name,
    const RenderMeshIdT exp_mesh_id)
{
    const RenderMeshIdT loaded_mesh_id =
        RenderMeshDepot::ref().create_from_mesh_file(abs_file_name, exp_mesh_id);
    if (loaded_mesh_id == INVALID_RENDER_MESH_ID)
    {
        return false;
    }
    else
    {
        m_geometry_id = loaded_mesh_id;
        return true;
    }
}


ObjectMarker *
GeometryData::create (
    SceneObject & marker_owner)
{
    /// 申请内存
    void* const new_marker =
        BlockAllocator<GeometryData, INIT_GEOMETRY_COUNT>::ref().allocate();
    if (new_marker)
    {
        /// 创建Beveled Cube
        const RenderMeshIdT cube_id = RenderMeshDepot::ref().create_unit_beveled_cube();
        /// 构建实例
        new(new_marker)GeometryData(marker_owner, cube_id);
    }

    return (ObjectMarker*)new_marker;
}


bool
GeometryData::destroy (
    ObjectMarker * & marker_object)
{
    RUNTIME_ASSERT(marker_object, "Marker can not be NULL!!");

    GeometryData * const geometry_data = (GeometryData*)marker_object;
    /// 调用析构函数
    geometry_data->~GeometryData();
    /// 释放内存
    const bool opcode =
        BlockAllocator<GeometryData, INIT_GEOMETRY_COUNT>::ref().deallocate(
            marker_object);
    /// 清除参考
    marker_object = nullptr;
    return opcode;
}


GeometryData::GeometryData (
    SceneObject &       marker_owner,
    const RenderMeshIdT geometry_id)
:
    SuperT(marker_owner, ms_type_info.marker_name_id()),
    m_geometry_id(geometry_id)
{

}


GeometryData::~GeometryData ()
{

}
