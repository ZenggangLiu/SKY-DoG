/***************************************************************************************
                                                                                        
        *          .               *                              .               *     
        ███████╗██╗  ██╗██╗   ██╗        ██████╗  ██████╗  ██████╗         *            
        ██╔════╝██║ ██╔╝╚██╗ ██╔╝        ██╔══██╗██╔═══██╗██╔════╝                      
        ███████╗█████╔╝  ╚████╔╝         ██║  ██║██║   ██║██║  ███╗        .            
        ╚════██║██╔═██╗   ╚██╔╝          ██║  ██║██║   ██║██║   ██║                     
        ███████║██║  ██╗    ██║           ██████╔╝╚██████╔╝╚██████╔╝         *          
        ╚══════╝╚═╝  ╚═╝    ╚═╝           ╚═════╝  ╚═════╝  ╚═════╝                     
                                                                                        
        <~~~               .        SKY Dog Game                      ~~~>        *     
                                Real-Time | Cross-Platform           .                  
----------------------------------------------------------------------------------------
                                                                                        
                                  ,,                                                    
                  __           o-°°|\_____/)                                            
    Author:   (___()'`; Zee...  \_/|_)     )                                            
              /,    /`             \  __  /                                             
              \\"--\\              (_/ (_/                                              
    Created:  4/09/26  @  3:36 PM
    FileName: GeometryData.hpp @ RedSoUL Project
    History:
             - created by: 4/09/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// Library headers
#include "Render/RenderMeshId.hpp"
#include "SceneGraph/ObjectMarker.hpp"


class SceneObject;


class GeometryData : public ObjectMarker
{
public:
    RenderMeshIdT
    geometry_id () const;

    /// 创建一个单位Cube(带Position, Normal, UV数据)
    ///
    ///              ^ Y
    ///              |     / Z
    ///        +-----|-------+ (0.5, 0.5, 0.5)
    ///       /|     .   /  /|
    ///      / |        .  / |
    ///     +-------------+  |
    ///     |  |     |/   |  |
    ///     |  |     O----|.--------> X
    ///     |  |          |  |
    ///     |  +----------|- +
    ///     | /           | /
    ///     +-------------+
    /// (-0.5, -0.5, -0.5)
    ///
    bool
    create_cube ();

    /// 创建一个单位Beveled Cube(带Position, Normal, UV数据)
    bool
    create_beveled_cube ();

    /// 创建一个单位Icosphere(带Position, Normal, UV数据)
    bool
    create_icosphere ();

    /// 创建指定RenderMesh文件中的几何体
    ///
    /// @param[in]  abs_file_name
    ///     RenderMesh文件的绝对路径
    /// @param[in]  exp_mesh_id
    ///     希望使用的Mesh Id
    /// @return
    ///     True, 如果加载成功
    ///     False, 如果加载失败(NOTE: 不会修改内部使用的Geometry Id)
    bool
    create_geometry (
        const char * const  abs_file_name,
        const RenderMeshIdT exp_mesh_id);

private:
    friend class SceneObject;

    typedef ObjectMarker SuperT;

    /// 创建一个Beveled Cube
    ///
    /// @param[in]  marker_owner
    ///     Marker的所有者
    static
    ObjectMarker *
    create (
        SceneObject & marker_owner);

    /// 销毁函数
    ///
    /// @param[in,out] marker_object
    ///     Marker实例。设置为nullptr, 如果销毁成功
    /// @return
    ///     True,  如果销毁成功
    ///     False, 如果销毁失败
    static
    bool
    destroy (
        ObjectMarker * & marker_object);

    GeometryData (
        SceneObject &       marker_owner,
        const RenderMeshIdT geometry_id);

    ~GeometryData ();

    GeometryData (
        const GeometryData &) = delete;
    GeometryData & operator = (
        const GeometryData &) = delete;

private:
    /// 起始的点光源个数
    static constexpr uint16_t INIT_GEOMETRY_COUNT = 100;

    /// Marker类型信息
    static const MarkerTypeInfo ms_type_info;

    /// NOTE: 如果Geometry为Triangle Mesh, 此Id可能为INVALID_RENDER_MESH_ID
    RenderMeshIdT m_geometry_id;
};
