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
    Created:  25/04/26  @  4:45 PM
    FileName: RMeshHead_v1.0.hpp @ RedSoUL Project
    History:
             - created by: 25/04/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint8_t
/// Library headers
#include "Collision/AABB.hpp"
#include "Common/CommonDefines.hpp" /// INLINE_FUNCTION, PACKED_STRUCT
#include "IO/SoULFileHead.hpp"
#include "Render/RenderMeshId.hpp"


#define RMESH_FILE_MAGIC_NUMBER  FOUR_CC_32('R', 'M', 'E', 'S')
#define RMESH_FILE_HEAD_END_MARK FOUR_CC_64('R', 'M', 'E', 'S', 'S', 'L', 'U', 'S')


/// Render Mesh文件1.0版本
///
/// +-----------------------+
/// |                       |
/// | == RenderMesh v1.0 == |
/// |                       |
/// +-----------------------+
/// | Magic Number          | <-- 四个字节(32位):"RMES"的FourCC
/// +-----------------------+
/// | Major Version         | <-- 二个字节(16位):主版本号(1)
/// |-----------------------+
/// | Minor Version         | <-- 二个字节(16位):辅版本号(0)
/// |-----------------------+
/// | File Flags            | <-- 二个字节(16位):文件标记(保留。设置为0)
/// +-----------------------+
/// | Mesh Id               | <-- 八个字节(64位):作为Key使用, 必须转换为RenderMeshIdT
/// +-----------------------+
/// | Minimal Corner(x,y,z) | <-- 十二个字节(96位)
/// +-----------------------+
/// | Maximal Corner(x,y,z) | <-- 十二个字节(96位)
/// +-----------------------+
/// | Vertex Data Layout    | <-- 两个字节(16位)
/// +-----------------------+
/// | Vertex Count          | <-- 四个字节(32位)
/// +-----------------------+
/// | Face/Triangle Count   | <-- 四个字节(32位)
/// +-----------------------+
/// | Vertex List Size      | <-- 四个字节(32位):顶点数据列表占用多少字节
/// +-----------------------+
/// | Triangle List Size    | <-- 四个字节(32位):三角面列表占用多少字节
/// +-----------------------+
/// | Vertex Data Offset    | <-- 四个字节(32位):顶点数据相对文件头位置偏移多少字节
/// +-----------------------+
/// | Triangle Data Offset  | <-- 四个字节(32位):三角面数据相对文件头位置偏移多少字节
/// +-----------------------+
/// | Header End            | <-- 八个字节(64位):"RMESSLUS"的FourCC
/// +-----------------------+
///
PACKED_STRUCT(RMeshHead_v1_0 : public SoULFileHead,
    typedef SoULFileHead SuperT;

    const uint64_t mesh_id;
    /// 包围盒的最小点
    const float    mincorner_x;
    const float    mincorner_y;
    const float    mincorner_z;
    /// 包围盒的最大点
    const float    maxcorner_x;
    const float    maxcorner_y;
    const float    maxcorner_z;
    const uint16_t vertex_layout;
    const uint32_t vertex_count;
    const uint32_t triangle_count;
    const uint32_t vertex_list_size;
    const uint32_t triangle_list_size;
    const uint32_t vertex_data_offset;
    const uint32_t index_data_offset;
    const uint64_t head_end_mark;

    INLINE_FUNCTION
    RMeshHead_v1_0 (
        const RenderMeshIdT _mesh_id,
        const AABB &        _bound_box,
        const uint16_t      _vertex_layout,
        const uint32_t      _vertex_count,
        const uint32_t      _triangle_count,
        const uint32_t      _vertex_list_size,
        const uint32_t      _triangle_list_size,
        const uint32_t      _vertex_data_offset,
        const uint32_t      _index_data_offset)
    :
        SuperT(RMESH_FILE_MAGIC_NUMBER, 1, 0, 0),
        mesh_id((uint64_t)_mesh_id.value),
        mincorner_x(_bound_box.pmin.x),
        mincorner_y(_bound_box.pmin.y),
        mincorner_z(_bound_box.pmin.z),
        maxcorner_x(_bound_box.pmax.x),
        maxcorner_y(_bound_box.pmax.y),
        maxcorner_z(_bound_box.pmax.z),
        vertex_layout(_vertex_layout),
        vertex_count(_vertex_count),
        triangle_count(_triangle_count),
        vertex_list_size(_vertex_list_size),
        triangle_list_size(_triangle_list_size),
        vertex_data_offset(_vertex_data_offset),
        index_data_offset(_index_data_offset),
        head_end_mark(RMESH_FILE_HEAD_END_MARK)
    {}
);
