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
    Created:  3/04/26  @  4:48 PM
    FileName: RenderMeshDepot.hpp @ RedSoUL Project
    History:
             - created by: 3/04/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint32_t,...
#include <unordered_map>
/// Library headers
#include "Render/RenderMeshId.hpp"


struct AABB;
struct IndexedTriangle;
struct RenderMesh;


/// 渲染几何体仓库
///
/// +-------+-------+-----+-------+
/// |  Id1  |  Id2  | ... |  Idn  |
/// +-------+-------+-----+-------+
/// | Mesh1 | Mesh2 | ... | MeshN |
/// +-------+-------+-----+-------+
///
class RenderMeshDepot
{
public:
    /// 获得对此Depot的参考
    static
    RenderMeshDepot &
    ref ();

    /// 获取指定Id对应的RenderMesh数据
    ///
    /// @return
    ///     合法RenderMesh指针, 如果指定Id对应Cached的数据
    ///     nullptr, 如果指定Id非法, 或者指定Id不在Cache中
    const RenderMesh *
    mesh_data (
        const RenderMeshIdT mesh_id) const;

    /// 创建一个边长为1的正方形(无纹理坐标)
    ///
    ///             ^ Y
    ///             |
    ///             |
    ///     1-------+-------3 (+0.5, +0.5, 0)
    ///     | *     |       |
    ///     |   *   |  II   |
    ///     |     * |       |
    ///     |       o-------+-------> X
    ///     |         *     |
    ///     |    I      *   |
    ///     |             * |
    ///     0---------------2
    /// (+0.5, -0.5, 0)
    ///
    RenderMeshIdT
    create_unit_square ();

    /// 创建一个边长为1的正方形(带纹理坐标)
    RenderMeshIdT
    create_unit_square_uv ();

    /// 创建一个单位Cube(只有位置数据, 无纹理坐标)
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
    RenderMeshIdT
    create_unit_cube_pos ();

    /// 创建一个单位Cube(带Position, Normal, UV数据)
    RenderMeshIdT
    create_unit_cube ();

    /// 创建一个单位Beveled Cube(带Position, Normal, UV数据)
    RenderMeshIdT
    create_unit_beveled_cube ();

    /// 创建一个单位二十面体(无纹理坐标)
    RenderMeshIdT
    create_unit_icosahedron ();

    /// 创建一个单位Icosphere(带Position, Normal, UV数据)
    RenderMeshIdT
    create_unit_icosphere ();

    /// 创建指定RenderMesh文件中的几何体
    ///
    /// @param[in]  abs_file_name
    ///     RenderMesh文件的绝对路径
    /// @param[in]  exp_mesh_id
    ///     希望使用的Mesh Id
    /// @return
    ///     合法Id, 如果加载成功
    ///     非法Id, 如果加载失败
    RenderMeshIdT
    create_from_mesh_file (
        const char * const  abs_file_name,
        const RenderMeshIdT exp_mesh_id);

    /// 将指定Id的RenderMesh的数据保存在指定的PLY文件中
    void
    write_to_ply_file (
        const char * const  abs_file_name,
        const RenderMeshIdT mesh_id) const;

    /// 将指定Id的RenderMesh的数据保存在指定的RenderMesh文件中
    void
    write_to_mesh_file (
        const char * const  abs_file_name,
        const RenderMeshIdT mesh_id) const;

    /// 清空所有缓存的几何体
    void
    clear ();


private:
     RenderMeshDepot ();
    ~RenderMeshDepot ();

    /// Cache新的RenderMesh
    ///
    /// @param[in]  is_dyn_allocated
    ///     标记顶点列表, 三角面列表是否为动态分配(True)
    /// @param[in]  vertex_list
    ///     顶点数据列表(16字节对齐)
    /// @param[in]  vertex_list_size
    ///     顶点数据列表的大小(字节个数)
    /// @param[in]  triangle_list
    ///     三角面数据列表
    /// @param[in]  triangle_list_size
    ///     三角面数据列表的大小(字节个数)
    void
    cache_mesh (
        const RenderMeshIdT           mesh_id,
        const AABB &                  bound_box,
        const uint16_t                vertex_layout,
        const uint32_t                vertex_count,
        const uint32_t                triangle_count,
        const bool                    is_dyn_allocated,
        const uint8_t * const         vertex_list,
        const uint32_t                vertex_list_size,
        const IndexedTriangle * const triangle_list,
        const uint32_t                triangle_list_size);

private:
    typedef std::unordered_map < RenderMeshIdT,
                                 RenderMesh,
                                 RenderMeshIdT::StlHasher,
                                 RenderMeshIdT::StlComparer > RenderMeshTableT;
    typedef RenderMeshTableT::const_iterator                  ConstRenderMeshIteratorT;
    typedef RenderMeshTableT::iterator                        RenderMeshIteratorT;

    RenderMeshTableT m_mesh_table;
};
