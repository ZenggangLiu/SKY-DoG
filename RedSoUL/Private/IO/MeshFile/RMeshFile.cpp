/// System headers
#include <cstdio>   /// std::snprintf
#include <cstdlib>  /// std::malloc, std::free
/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Common/CommonDefines.hpp"   /// MAKE_FILE_VERSION
#include "Common/CompilerDefines.hpp" /// BUILD_MODE
#include "FileSystem/FileName.hpp"
#include "FileSystem/NativeFile.hpp"
#include "FileSystem/NativeReadStream.hpp"
#include "FileSystem/NativeWriteStream.hpp"
#include "Memory/MemoryUtilities.hpp"
#include "Render/IndexedTriangle.hpp"
#include "Render/RenderMesh.hpp"
#include "Render/VertexDataType.hpp"
#include "Render/VertexLayout/Layout_Pos.hpp"
#include "Render/VertexLayout/Layout_Pos_Colr.hpp"
#include "Render/VertexLayout/Layout_Pos_Colr_Norm.hpp"
#include "Render/VertexLayout/Layout_Pos_Colr_Norm_Uv.hpp"
#include "Render/VertexLayout/Layout_Pos_Colr_Uv.hpp"
#include "Render/VertexLayout/Layout_Pos_Norm.hpp"
#include "Render/VertexLayout/Layout_Pos_Norm_Uv.hpp"
#include "Render/VertexLayout/Layout_Pos_Uv.hpp"
#include "IO/MeshFile/RMeshHead_v1.0.hpp"
/// Self header
#include "IO/MeshFile/RMeshFile.hpp"


/// 定义v1.0
static constexpr uint16_t FILE_VERSION_1_0 = MAKE_FILE_VERSION(1, 0);

/// 当前版本定义
/// NOTE: 如果添加新的版本, 需要提供对老版本的转换函数!!
static constexpr uint16_t CURRENT_VERSION = FILE_VERSION_1_0;


/// 反序列化RenderMesh文件(v1.0版本)
///
/// 如果出现错误: buffer内保存错误信息
static
bool
deserialize_from_rmesh_file_v1_0 (
    NativeReadStream &  mesh_file,
    const RenderMeshIdT exp_mesh_id,
    AABB &              bound_box,
    uint16_t &          vertex_layout,
    uint32_t &          vertex_count,
    uint32_t &          triangle_count,
    uint8_t * &         vertex_list,
    uint32_t &          vertex_list_size,
    IndexedTriangle * & triangle_list,
    uint32_t &          triangle_list_size,
    char * const        buffer,
    const uint32_t      buffer_size)
{
#if (BUILD_MODE == DEBUG_BUILD_MODE)
    /// RMesh文件允许保存的最大顶点数
    static constexpr uint32_t MAXIMAL_VERTEX_COUNT   = 2 * 1024 * 1024;
    /// RMesh文件允许保存的最大三角面数: #Triangle = 2 * #Vertex - 4
    static constexpr uint32_t MAXIMAL_TRIANGLE_COUNT = 2 * MAXIMAL_VERTEX_COUNT;
#endif

    vertex_list_size   = 0;
    triangle_list_size = 0;

    /// 读入文件头的后续部分
    int32_t loaded_bytes =
        mesh_file.read(
            (uint8_t*)buffer, buffer_size - sizeof(SoULFileHead),
            sizeof(SoULFileHead), sizeof(RMeshHead_v1_0) - sizeof(SoULFileHead));
    if ((uint32_t)loaded_bytes == sizeof(RMeshHead_v1_0) - sizeof(SoULFileHead))
    {
        const RMeshHead_v1_0 & rmesh_file_head = *(const RMeshHead_v1_0*)buffer;
        if (rmesh_file_head.head_end_mark == RMESH_FILE_HEAD_END_MARK)
        {
            /// 判断文件中存储的MeshId是否与期待的Id相同
            const RenderMeshIdT stored_mesh_id(
                (RenderMeshIdT::HashedValueTypeT)rmesh_file_head.mesh_id);
            if (stored_mesh_id == exp_mesh_id)
            {
                RUNTIME_ASSERT(
                    rmesh_file_head.vertex_count <= MAXIMAL_VERTEX_COUNT,
                    "Vertex count is out of range: [1, %u]!!", MAXIMAL_VERTEX_COUNT);
                RUNTIME_ASSERT(
                    rmesh_file_head.triangle_count <= MAXIMAL_TRIANGLE_COUNT,
                    "Triangle count is out of range: [1, %u]!!", MAXIMAL_TRIANGLE_COUNT);
                RUNTIME_ASSERT(
                    rmesh_file_head.vertex_data_offset >= sizeof(RMeshHead_v1_0),
                    "Vertex data offset is inside file head!!");
                RUNTIME_ASSERT(
                    rmesh_file_head.index_data_offset >=
                    (rmesh_file_head.vertex_data_offset + rmesh_file_head.vertex_list_size),
                    "Triangle data offset overlaps vertex data!!");
                RUNTIME_ASSERT(
                    (rmesh_file_head.index_data_offset +
                     rmesh_file_head.triangle_list_size) <= mesh_file.file_length(),
                    "Triangle data is out of file range!!");

                /// 赋值
                bound_box.pmax = float_3::make(rmesh_file_head.maxcorner_x,
                                               rmesh_file_head.maxcorner_y,
                                               rmesh_file_head.maxcorner_z);
                bound_box.pmin = float_3::make(rmesh_file_head.mincorner_x,
                                               rmesh_file_head.mincorner_y,
                                               rmesh_file_head.mincorner_z);
                vertex_layout  = rmesh_file_head.vertex_layout;
                vertex_count   = rmesh_file_head.vertex_count;
                triangle_count = rmesh_file_head.triangle_count;

                /// 检测三角面列表大小
                const uint32_t exp_trangle_list_size =
                    (uint32_t)(sizeof(IndexedTriangle) * triangle_count);
                if (rmesh_file_head.triangle_list_size != exp_trangle_list_size)
                {
                    std::snprintf(
                        buffer, buffer_size,
                        "triangle list size not match(%u needed NOT %u)!!\n",
                        exp_trangle_list_size, rmesh_file_head.triangle_list_size);
                    return false;
                }

                /// 创建顶点数据列表
                switch (vertex_layout)
                {
                    /// 只有Pos数据
                    case Layout_Pos::LAYOUT_DECL:
                    {
                        const uint32_t exp_vertex_list_size =
                            (uint32_t)(sizeof(Layout_Pos) * vertex_count);
                        if (rmesh_file_head.vertex_list_size == exp_vertex_list_size)
                        {
                            vertex_list = (uint8_t*)MemoryUtility::aligned_alloc(
                                rmesh_file_head.vertex_list_size, alignof(Layout_Pos));
                            break;
                        }
                        else
                        {
                            std::snprintf(
                                buffer, buffer_size,
                                "vertex list size not match(%u needed NOT %u)!!\n",
                                exp_vertex_list_size, rmesh_file_head.vertex_list_size);
                            return false;
                        }
                    }

                    /// Pos数据 + Color数据
                    case Layout_Pos_Colr::LAYOUT_DECL:
                    {
                        const uint32_t exp_vertex_list_size =
                            (uint32_t)(sizeof(Layout_Pos_Colr) * vertex_count);
                        if (rmesh_file_head.vertex_list_size == exp_vertex_list_size)
                        {
                            vertex_list = (uint8_t*)MemoryUtility::aligned_alloc(
                                rmesh_file_head.vertex_list_size, alignof(Layout_Pos_Colr));
                            break;
                        }
                        else
                        {
                            std::snprintf(
                                buffer, buffer_size,
                                "vertex list size not match(%u needed NOT %u)!!\n",
                                exp_vertex_list_size, rmesh_file_head.vertex_list_size);
                            return false;
                        }
                    }

                    /// Pos数据 + Normal数据
                    case Layout_Pos_Norm::LAYOUT_DECL:
                    {
                        const uint32_t exp_vertex_list_size =
                            (uint32_t)(sizeof(Layout_Pos_Norm) * vertex_count);
                        if (rmesh_file_head.vertex_list_size == exp_vertex_list_size)
                        {
                            vertex_list = (uint8_t*)MemoryUtility::aligned_alloc(
                                rmesh_file_head.vertex_list_size, alignof(Layout_Pos_Norm));
                            break;
                        }
                        else
                        {
                            std::snprintf(
                                buffer, buffer_size,
                                "vertex list size not match(%u needed NOT %u)!!\n",
                                exp_vertex_list_size, rmesh_file_head.vertex_list_size);
                            return false;
                        }
                    }

                    /// Pos数据 + UV数据
                    case Layout_Pos_Uv::LAYOUT_DECL:
                    {
                        const uint32_t exp_vertex_list_size =
                            (uint32_t)(sizeof(Layout_Pos_Uv) * vertex_count);
                        if (rmesh_file_head.vertex_list_size == exp_vertex_list_size)
                        {
                            vertex_list = (uint8_t*)MemoryUtility::aligned_alloc(
                                rmesh_file_head.vertex_list_size, alignof(Layout_Pos_Uv));
                            break;
                        }
                        else
                        {
                            std::snprintf(
                                buffer, buffer_size,
                                "vertex list size not match(%u needed NOT %u)!!\n",
                                exp_vertex_list_size, rmesh_file_head.vertex_list_size);
                            return false;
                        }
                    }

                    /// Pos数据 + Color数据 + Normal数据
                    case Layout_Pos_Colr_Norm::LAYOUT_DECL:
                    {
                        const uint32_t exp_vertex_list_size =
                            (uint32_t)(sizeof(Layout_Pos_Colr_Norm) * vertex_count);
                        if (rmesh_file_head.vertex_list_size == exp_vertex_list_size)
                        {
                            vertex_list = (uint8_t*)MemoryUtility::aligned_alloc(
                                rmesh_file_head.vertex_list_size, alignof(Layout_Pos_Colr_Norm));
                            break;
                        }
                        else
                        {
                            std::snprintf(
                                buffer, buffer_size,
                                "vertex list size not match(%u needed NOT %u)!!\n",
                                exp_vertex_list_size, rmesh_file_head.vertex_list_size);
                            return false;
                        }
                    }

                    /// Pos数据 + Color数据 + UV数据
                    case Layout_Pos_Colr_Uv::LAYOUT_DECL:
                    {
                        const uint32_t exp_vertex_list_size =
                            (uint32_t)(sizeof(Layout_Pos_Colr_Uv) * vertex_count);
                        if (rmesh_file_head.vertex_list_size == exp_vertex_list_size)
                        {
                            vertex_list = (uint8_t*)MemoryUtility::aligned_alloc(
                                rmesh_file_head.vertex_list_size, alignof(Layout_Pos_Colr_Uv));
                            break;
                        }
                        else
                        {
                            std::snprintf(
                                buffer, buffer_size,
                                "vertex list size not match(%u needed NOT %u)!!\n",
                                exp_vertex_list_size, rmesh_file_head.vertex_list_size);
                            return false;
                        }
                    }

                    /// Pos数据 + Normal数据 + UV数据
                    case Layout_Pos_Norm_Uv::LAYOUT_DECL:
                    {
                        const uint32_t exp_vertex_list_size =
                            (uint32_t)(sizeof(Layout_Pos_Norm_Uv) * vertex_count);
                        if (rmesh_file_head.vertex_list_size == exp_vertex_list_size)
                        {
                            vertex_list = (uint8_t*)MemoryUtility::aligned_alloc(
                                rmesh_file_head.vertex_list_size, alignof(Layout_Pos_Norm_Uv));
                            break;
                        }
                        else
                        {
                            std::snprintf(
                                buffer, buffer_size,
                                "vertex list size not match(%u needed NOT %u)!!\n",
                                exp_vertex_list_size, rmesh_file_head.vertex_list_size);
                            return false;
                        }
                    }

                    /// Pos数据 + Color数据 + Normal数据 + UV数据
                    case Layout_Pos_Colr_Norm_Uv::LAYOUT_DECL:
                    {
                        const uint32_t exp_vertex_list_size =
                            (uint32_t)(sizeof(Layout_Pos_Colr_Norm_Uv) * vertex_count);
                        if (rmesh_file_head.vertex_list_size == exp_vertex_list_size)
                        {
                            vertex_list = (uint8_t*)MemoryUtility::aligned_alloc(
                                rmesh_file_head.vertex_list_size, alignof(Layout_Pos_Colr_Norm_Uv));
                            break;
                        }
                        else
                        {
                            std::snprintf(
                                buffer, buffer_size,
                                "vertex list size not match(%u needed NOT %u)!!\n",
                                exp_vertex_list_size, rmesh_file_head.vertex_list_size);
                            return false;
                        }
                    }

                    default:
                    {
                        RUNTIME_ASSERT(false, "Unknown Vertex Layout type!!");
                        std::snprintf(
                            buffer, buffer_size, "Unknown Vertex Layout type!\n");
                        return false;
                    }
                }

                /// 读入顶点数据
                if (vertex_list)
                {
                    /// 尝试SEEK至Vertex数据保存处
                    if (mesh_file.seek(
                        (int32_t)rmesh_file_head.vertex_data_offset,
                        SeekMode::FILE_BEGIN_SEEK_MODE) == false)
                    {
                        MemoryUtility::aligned_free(vertex_list);
                        std::snprintf(
                            buffer, buffer_size,
                            "can not seek to the vertex data list!!\n");
                        return false;
                    }

                    loaded_bytes = mesh_file.read(
                        vertex_list, rmesh_file_head.vertex_list_size,
                        0, rmesh_file_head.vertex_list_size);
                    if ((uint32_t)loaded_bytes == rmesh_file_head.vertex_list_size)
                    {
                        /// 创建三角面列表
                        triangle_list = (IndexedTriangle*)std::malloc(
                            rmesh_file_head.triangle_list_size);

                        /// 读入索引数据
                        if (triangle_list)
                        {
                            /// 尝试SEEK至Triangle Index数据保存处
                            if (mesh_file.seek(
                                (int32_t)rmesh_file_head.index_data_offset,
                                SeekMode::FILE_BEGIN_SEEK_MODE) == false)
                            {
                                MemoryUtility::aligned_free(vertex_list);
                                std::free(triangle_list);
                                std::snprintf(
                                    buffer, buffer_size,
                                    "can not seek to the triangle data list!!\n");
                                return false;
                            }

                            loaded_bytes = mesh_file.read(
                                (uint8_t*)triangle_list, rmesh_file_head.triangle_list_size,
                                0, rmesh_file_head.triangle_list_size);
                            if ((uint32_t)loaded_bytes == rmesh_file_head.triangle_list_size)
                            {
                                vertex_list_size   = rmesh_file_head.vertex_list_size;
                                triangle_list_size = rmesh_file_head.triangle_list_size;
                                return true;
                            }
                            else
                            {
                                MemoryUtility::aligned_free(vertex_list);
                                std::free(triangle_list);
                                std::snprintf(
                                    buffer, buffer_size,
                                    "can not load the triangle data list!!\n");
                                return false;
                            }
                        }
                        else
                        {
                            MemoryUtility::aligned_free(vertex_list);
                            std::snprintf(
                                buffer, buffer_size,
                                "can not allocate memory for the triangle data list!!\n");
                            return false;
                        }
                    }
                    else
                    {
                        MemoryUtility::aligned_free(vertex_list);
                        std::snprintf(
                            buffer, buffer_size, "can not load the vertex data list!!\n");
                        return false;
                    }
                }
                else
                {
                    std::snprintf(
                        buffer, buffer_size,
                        "can not allocate memory for the vertex data list!!\n");
                    return false;
                }
            }
            else
            {
                std::snprintf(
                    buffer, buffer_size,
                    "stored mesh id does not match the expecting one!!\n");
                return false;
            }
        }
        else
        {
            std::snprintf(
                buffer, buffer_size,
                "render mesh file head end mark does not match!!\n");
            return false;
        }
    }
    else
    {
        std::snprintf(
            buffer, buffer_size,
            "can not load the Render Mesh file head version 1.0\n");
        return false;
    }
}


/// 序列化为RenderMesh文件(v1.0版本)
///
/// NOTE: 如果出现错误: buffer内保存错误信息
static
bool
serialize_to_rmesh_file_v1_0 (
    const RenderMesh &  mesh_data,
    NativeWriteStream & mesh_file,
    char * const        buffer,
    const uint32_t      buffer_size)
{
    /// 顶点数据相对偏移
    static const uint32_t VERTEX_LIST_OFFSET = sizeof(RMeshHead_v1_0);

    const RMeshHead_v1_0 file_header
    {
        mesh_data.mesh_id, /// 使用RenderMesh保存的Id作为文件中的Id
        mesh_data.bound_box,
        mesh_data.vertex_layout,
        mesh_data.vertex_count,
        mesh_data.triangle_count,
        mesh_data.vertex_list_size,
        mesh_data.triangle_list_size,
        VERTEX_LIST_OFFSET,
        VERTEX_LIST_OFFSET + mesh_data.vertex_list_size
    };

    /// 输出文件头
    bool opcode = mesh_file.write(
        (const uint8_t*)&file_header, sizeof(RMeshHead_v1_0),
        0, sizeof(RMeshHead_v1_0));
    if (opcode)
    {
        /// 输出几何数据
        ///
        /// RenderMesh文件Layout:
        ///
        /// +-----------------------+
        /// | Render Mesh File Head |
        /// +-----------------------+
        /// | Vertex Data List      | <-- Layout_Pos_XYZ
        /// +-----------------------+
        /// | Triangle Data List    | <-- IndexedTriangle
        /// +-----------------------+

        /// 输出顶点数据
        opcode = mesh_file.write(
            mesh_data.vertex_list, mesh_data.vertex_list_size,
            0, mesh_data.vertex_list_size);
        if (opcode)
        {
            /// 输出索引数据
            opcode = mesh_file.write(
                (const uint8_t*)mesh_data.triangle_list,
                mesh_data.triangle_list_size, 0, mesh_data.triangle_list_size);
            if (opcode == false)
            {
                std::snprintf(
                    buffer, buffer_size, "can not store triangle data list!!\n");
            }
        }
        else
        {
            std::snprintf(buffer, buffer_size, "can not store vertex data list!!\n");
        }
    }
    else
    {
        std::snprintf(buffer, buffer_size, "can not store file head!!\n");
    }
    return opcode;
}



// MARK: == RenderMesh File ==
bool
RMeshFile::read_from (
    const char * const  abs_file_name,
    const RenderMeshIdT exp_mesh_id,
    AABB &              bound_box,
    uint16_t &          vertex_layout,
    uint32_t &          vertex_count,
    uint32_t &          triangle_count,
    uint8_t * &         vertex_list,
    uint32_t &          vertex_list_size,
    IndexedTriangle * & triangle_list,
    uint32_t &          triangle_list_size)
{
    RUNTIME_ASSERT( abs_file_name, "File name can not be NULL!!");
    RUNTIME_ASSERT(*abs_file_name, "File name can not be empty!!");
    RUNTIME_ASSERT(exp_mesh_id.is_valid(), "Expected mesh Id must be valid!!");

    char buffer[1024];

    /// 确保使用'.rmesh'文件扩展符
    FileName::append_file_extension(abs_file_name, "rmesh", buffer, sizeof(buffer));

    vertex_list_size   = 0;
    triangle_list_size = 0;

    /// 创建ReadOnly文件流
    NativeReadStream mesh_file(buffer);
    if (mesh_file.is_opened())
    {
        /// 读入RedSoUL文件头
        if (SoULFileHead::create_from_stream(mesh_file, *(SoULFileHead*)buffer))
        {
            const SoULFileHead & soul_file_head = *(const SoULFileHead*)buffer;
            if (soul_file_head.magic_number == RMESH_FILE_MAGIC_NUMBER)
            {
                const uint16_t file_version =
                    MAKE_FILE_VERSION(soul_file_head.major_version,
                                      soul_file_head.minor_version);
                switch (file_version)
                {
                    case FILE_VERSION_1_0:
                    {
                        const bool opcode = deserialize_from_rmesh_file_v1_0(
                            mesh_file, exp_mesh_id, bound_box,
                            vertex_layout, vertex_count, triangle_count,
                            vertex_list, vertex_list_size,
                            triangle_list, triangle_list_size,
                            buffer, (uint32_t)sizeof(buffer));
                        if (opcode == false)
                        {
                            std::printf(
                                "[ERROR]: can not read file: %s\nwith error: %s\n",
                                abs_file_name, buffer);
                        }
                        return opcode;
                    }

                    default:
                    {
                        RUNTIME_ASSERT(false, "Unknown file version!!");
                        return false;
                    }
                }
            }
            else
            {
                std::snprintf(
                    buffer, sizeof(buffer), "this file is not a Render Mesh file!!\n");
                std::printf(
                    "[ERROR]: can not read file: %s\nwith error: %s\n",
                    abs_file_name, buffer);
                return false;
            }
        }
        else
        {
            std::snprintf(buffer, sizeof(buffer), "can not read RedSoUL file head!!\n");
            std::printf(
                "[ERROR]: can not read file: %s\nwith error: %s\n",
                abs_file_name, buffer);
            return false;
        }
    }
    else
    {
        return false;
    }
}


bool
RMeshFile::write_to (
    const char * const abs_file_name,
    const RenderMesh & mesh_data)
{
    RUNTIME_ASSERT( abs_file_name, "File name can not be NULL!!");
    RUNTIME_ASSERT(*abs_file_name, "File name can not be empty!!");

    char buffer[1024];

    /// 确保使用'.rmesh'文件扩展符
    FileName::append_file_extension(abs_file_name, "rmesh", buffer, sizeof(buffer));

    /// 创建WriteOnly文件流
    NativeWriteStream mesh_file(buffer);
    if (mesh_file.is_opened())
    {
        switch (CURRENT_VERSION)
        {
            case FILE_VERSION_1_0:
            {
                const bool opcode =
                    serialize_to_rmesh_file_v1_0(mesh_data, mesh_file,
                                                 buffer, sizeof(buffer));
                if (opcode == false)
                {
                    std::printf(
                        "[ERROR]: can not store file: %s\nwith error: %s\n",
                        abs_file_name, buffer);
                }
                return opcode;
            }

            default:
            {
                RUNTIME_ASSERT(false, "Unknown file version!!");
                return false;
            }
        }
    }
    else
    {
        return false;
    }
}
