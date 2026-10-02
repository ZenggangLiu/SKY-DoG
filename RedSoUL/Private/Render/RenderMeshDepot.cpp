/// System headers
#include <cstdlib>  /// std::free
#include <cstdio>   /// std::snprintf
/// Library headers
#include "Collision/AABB.hpp"
#include "Assert/RuntimeAssert.hpp"
#include "Common/CommonDefines.hpp" /// GENERATE_BUILTIN_NAME
#include "IO/MeshFile/PlyFile.hpp"
#include "IO/MeshFile/RMeshFile.hpp"
#include "Memory/MemoryUtilities.hpp"
#include "Render/IndexedTriangle.hpp"
#include "Render/RenderMesh.hpp"
#include "Render/VertexLayout/Layout_Pos.hpp"
#include "Render/VertexLayout/Layout_Pos_Colr.hpp"
#include "Render/VertexLayout/Layout_Pos_Colr_Norm.hpp"
#include "Render/VertexLayout/Layout_Pos_Colr_Norm_Uv.hpp"
#include "Render/VertexLayout/Layout_Pos_Colr_Uv.hpp"
#include "Render/VertexLayout/Layout_Pos_Norm.hpp"
#include "Render/VertexLayout/Layout_Pos_Norm_Uv.hpp"
#include "Render/VertexLayout/Layout_Pos_Uv.hpp"
/// Self header
#include "Render/RenderMeshDepot.hpp"


RenderMeshDepot &
RenderMeshDepot::ref ()
{
    static RenderMeshDepot s_instance;
    return s_instance;
}


const RenderMesh *
RenderMeshDepot::mesh_data (
    const RenderMeshIdT mesh_id) const
{
    RUNTIME_ASSERT(mesh_id != INVALID_RENDER_MESH_ID, "Mesh Id is invalid!!");

    ConstRenderMeshIteratorT cached_data = m_mesh_table.find(mesh_id);
    if (cached_data == m_mesh_table.end())
    {
        return nullptr;
    }
    else
    {
        return &cached_data->second;
    }
}


RenderMeshIdT
RenderMeshDepot::create_unit_square ()
{
    typedef struct Layout_Pos VertexLayoutT;

    /// 计算 Mesh Id
    static constexpr RenderMeshIdT MESH_ID =
        STATIC_RENDER_MESH_HASH(GENERATE_BUILTIN_NAME("UNIT_SQUARE"));

    /// 包围盒
    static constexpr AABB MESH_BOUND_BOX
    {
        { -0.5f, -0.001f, -0.5f },
        { +0.5f, +0.001f, +0.5f }
    };

    ///             ^ Z
    ///             |
    ///             |
    ///     1-------+-------3 (+0.5, 0, +0.5)
    ///     | *     |       |
    ///     |   *   |  II   |
    ///     |     * |       |
    ///     |       o-------+-------> X
    ///     |         *     |
    ///     |    I      *   |
    ///     |             * |
    ///     0---------------2
    /// (-0.5, 0, -0.5)
    ///
    /// 三角面I:  <0, 1, 2>
    /// 三角面II: <2 ,1, 3>

    /// 顶点列表
    static constexpr VertexLayoutT VERTEX_LIST[] =
    {
        /// 0
        {
            { -0.5f, +0.0f, -0.5f },
        },
        /// 1
        {
            { -0.5f, +0.0f, +0.5f },
        },
        /// 2
        {
            { +0.5f, +0.0f, -0.5f },
        },
        /// 3
        {
            { +0.5f, +0.0f, +0.5f },
        },
    };

    /// 三角面列表
    static const IndexedTriangle TRIANGLE_LIST[] =
    {
        { 0, 1, 2 },
        { 2, 1, 3 },
    };

    /// #vertex
    static constexpr uint16_t VERTEX_COUNT   = ARRAY_LENGTH(VERTEX_LIST);
    /// #triangle
    static constexpr uint16_t TRIANGLE_COUNT = ARRAY_LENGTH(TRIANGLE_LIST);

    cache_mesh(
        MESH_ID, MESH_BOUND_BOX,
        VertexLayoutT::LAYOUT_DECL, VERTEX_COUNT, TRIANGLE_COUNT, false,
        (const uint8_t*)VERTEX_LIST, (uint32_t)sizeof(VERTEX_LIST),
        TRIANGLE_LIST, (uint32_t)sizeof(TRIANGLE_LIST));
    return MESH_ID;
}


RenderMeshIdT
RenderMeshDepot::create_unit_square_uv ()
{
    typedef struct Layout_Pos_Uv VertexLayoutT;

    /// 计算 Mesh Id
    static constexpr RenderMeshIdT MESH_ID =
        STATIC_RENDER_MESH_HASH(GENERATE_BUILTIN_NAME("UNIT_SQUARE_UV"));

    /// 包围盒
    static constexpr AABB MESH_BOUND_BOX
    {
        { -0.5f, -0.001f, -0.5f },
        { +0.5f, +0.001f, +0.5f }
    };

    ///             ^ Z
    ///             |
    ///             |
    ///     1-------+-------3 (+0.5, 0, +0.5)
    ///     | *     |       |
    ///     |   *   |  II   |
    ///     |     * |       |
    ///     |       o-------+-------> X
    ///     |         *     |
    ///     |    I      *   |
    ///     |             * |
    ///     0---------------2
    /// (-0.5, 0, -0.5)
    ///
    /// 三角面I:  <0, 1, 2>
    /// 三角面II: <2 ,1, 3>

    /// 顶点列表
    static constexpr VertexLayoutT VERTEX_LIST[] =
    {
        /// 0
        {
            { -0.5f, +0.0f, -0.5f },
            { +0.0f, +0.0f        }
        },
        /// 1
        {
            { -0.5f, +0.0f, +0.5f },
            { +0.0f, +1.0f        }
        },
        /// 2
        {
            { +0.5f, +0.0f, -0.5f },
            { +1.0f, +0.0f        }
        },
        /// 3
        {
            { +0.5f, +0.0f, +0.5f },
            { +1.0f, +1.0f        }
        }
    };

    /// 三角面列表
    static const IndexedTriangle TRIANGLE_LIST[] =
    {
        { 0, 1, 2 },
        { 2, 1, 3 }
    };

    /// #vertex
    static constexpr uint16_t VERTEX_COUNT   = ARRAY_LENGTH(VERTEX_LIST);
    /// #triangle
    static constexpr uint16_t TRIANGLE_COUNT = ARRAY_LENGTH(TRIANGLE_LIST);

    cache_mesh(
        MESH_ID, MESH_BOUND_BOX,
        VertexLayoutT::LAYOUT_DECL, VERTEX_COUNT, TRIANGLE_COUNT, false,
        (const uint8_t*)VERTEX_LIST, (uint32_t)sizeof(VERTEX_LIST),
        TRIANGLE_LIST, (uint32_t)sizeof(TRIANGLE_LIST));
    return MESH_ID;
}


RenderMeshIdT
RenderMeshDepot::create_unit_cube_pos ()
{
    typedef struct Layout_Pos VertexLayoutT;

    /// 计算 Mesh Id
    static constexpr RenderMeshIdT MESH_ID =
        STATIC_RENDER_MESH_HASH(GENERATE_BUILTIN_NAME("UNIT_CUBE_POS"));

    /// 包围盒
    static constexpr AABB MESH_BOUND_BOX
    {
        { -0.5f, -0.5f, -0.5f },
        { +0.5f, +0.5f, +0.5f }
    };

    /// 按照如下设定创建方块Cube:
    ///
    ///              ^ Y
    ///              |     / Z
    ///       6+-----|-------+7
    ///       /|     .   /  /|
    ///      / |        .  / |
    ///    2+-------------+3 |
    ///     |  |     |/   |  |
    ///     |  |     O----|.------> X
    ///     |  |          |  |
    ///     | 4+----------|--+5
    ///     | /           | /
    ///    0+-------------+1
    ///
    /// 顶点列表
    static constexpr VertexLayoutT VERTEX_LIST[] =
    {
        /// 0
        {
            { -0.50f, -0.50f, -0.50f }
        },
        /// 1
        {
            { +0.50f, -0.50f, -0.50f }
        },
        /// 2
        {
            { -0.50f, +0.50f, -0.50f }
        },
        /// 3
        {
            { +0.50f, +0.50f, -0.50f }
        },
        /// 4
        {
            { -0.50f, -0.50f, +0.50f }
        },
        /// 5
        {
            { +0.50f, -0.50f, +0.50f }
        },
        /// 6
        {
            { -0.50f, +0.50f, +0.50f }
        },
        /// 7
        {
            { +0.50f, +0.50f, +0.50f }
        }
    };

    /// 三角面列表: Face数据以-Z, +Z开头
    static const IndexedTriangle TRIANGLE_LIST[] =
    {
        /// -Z平面:
        /// 2+-------------+3
        ///  | *           |
        ///  |   *         |
        ///  |     *       |
        ///  |       *     |
        ///  |         *   |
        ///  |           * |
        /// 0+-------------+1
        /// <0, 2, 1>
        { 0, 2, 1 },
        /// <1, 2, 3>
        { 1, 2, 3 },

        /// -X平面:
        /// 6+-------------+2
        ///  |           * |
        ///  |         *   |
        ///  |       *     |
        ///  |     *       |
        ///  |   *         |
        ///  | *           |
        /// 4+-------------+0
        /// <0, 4, 2>
        { 0, 4, 2 },
        /// <2, 4, 6>
        { 2, 4, 6 },

        /// +X平面:
        /// 3+-------------+7
        ///  |           * |
        ///  |         *   |
        ///  |       *     |
        ///  |     *       |
        ///  |   *         |
        ///  | *           |
        /// 1+-------------+5
        /// <5, 1, 7>
        { 5, 1, 7 },
        /// <7, 1, 3>
        { 7, 1, 3  },

        /// +Y平面:
        /// 6+-------------+7
        ///  |           * |
        ///  |         *   |
        ///  |       *     |
        ///  |     *       |
        ///  |   *         |
        ///  | *           |
        /// 2+-------------+3
        /// <3, 2, 7>
        { 3, 2, 7 },
        /// <7, 2, 6>
        { 7, 2, 6 },

        /// -Y平面:
        /// 0+-------------+1
        ///  |           * |
        ///  |         *   |
        ///  |       *     |
        ///  |     *       |
        ///  |   *         |
        ///  | *           |
        /// 4+-------------+5
        /// <5, 4, 1>
        { 5, 4, 1 },
        /// <1, 4, 0>
        { 1, 4, 0 },

        /// +Z平面:
        /// 7+-------------+6
        ///  | *           |
        ///  |   *         |
        ///  |     *       |
        ///  |       *     |
        ///  |         *   |
        ///  |           * |
        /// 5+-------------+4
        /// <5, 7, 4>
        { 5, 7, 4 },
        /// <4, 7, 6>
        { 4, 7, 6 }
    };

    /// #vertex
    static constexpr uint16_t VERTEX_COUNT   = ARRAY_LENGTH(VERTEX_LIST);
    /// #triangle
    static constexpr uint16_t TRIANGLE_COUNT = ARRAY_LENGTH(TRIANGLE_LIST);

    cache_mesh(
        MESH_ID, MESH_BOUND_BOX,
        VertexLayoutT::LAYOUT_DECL, VERTEX_COUNT, TRIANGLE_COUNT, false,
        (const uint8_t*)VERTEX_LIST, (uint32_t)sizeof(VERTEX_LIST),
        TRIANGLE_LIST, (uint32_t)sizeof(TRIANGLE_LIST));
    return MESH_ID;
}


RenderMeshIdT
RenderMeshDepot::create_unit_cube ()
{
    typedef struct Layout_Pos_Norm_Uv VertexLayoutT;

    /// 计算 Mesh Id
    static constexpr RenderMeshIdT MESH_ID =
        STATIC_RENDER_MESH_HASH(GENERATE_BUILTIN_NAME("UNIT_CUBE"));

    /// 包围盒
    static constexpr AABB MESH_BOUND_BOX
    {
        { -0.5f, -0.5f, -0.5f },
        { +0.5f, +0.5f, +0.5f }
    };

    /// 按照如下设定创建方块Cube:
    ///
    ///              ^ Y
    ///              |     / Z
    ///       6+-----|-------+7
    ///       /|     .   /  /|
    ///      / |        .  / |
    ///    2+-------------+3 |
    ///     |  |     |/   |  |
    ///     |  |     O----|.------> X
    ///     |  |          |  |
    ///     | 4+----------|--+5
    ///     | /           | /
    ///    0+-------------+1
    ///
    /// 每个Face独立使用完整UV空间[0, 1]x[0, 1]
    static constexpr VertexLayoutT VERTEX_LIST[] =
    {
        /// -Z平面
        {
            { -0.50f, -0.50f, -0.50f }, /// Position
            { +0.00f, +0.00f, -1.00f }, /// Normal
            { +0.00f, +0.00f         }  /// UV
        },
        {
            { +0.50f, -0.50f, -0.50f }, /// Position
            { +0.00f, +0.00f, -1.00f }, /// Normal
            { +1.00f, +0.00f         }  /// UV
        },
        {
            { -0.50f, +0.50f, -0.50f }, /// Position
            { +0.00f, +0.00f, -1.00f }, /// Normal
            { +0.00f, +1.00f         }  /// UV
        },
        {
            { +0.50f, +0.50f, -0.50f }, /// Position
            { +0.00f, +0.00f, -1.00f }, /// Normal
            { +1.00f, +1.00f         }  /// UV
        },

        /// -X平面
        {
            { -0.50f, -0.50f, -0.50f }, /// Position
            { -1.00f, +0.00f, +0.00f }, /// Normal
            { +0.00f, +0.00f         }  /// UV
        },
        {
            { -0.50f, -0.50f, +0.50f }, /// Position
            { -1.00f, +0.00f, +0.00f }, /// Normal
            { +1.00f, +0.00f         }  /// UV
        },
        {
            { -0.50f, +0.50f, -0.50f }, /// Position
            { -1.00f, +0.00f, +0.00f }, /// Normal
            { +0.00f, +1.00f         }  /// UV
        },
        {
            { -0.50f, +0.50f, +0.50f }, /// Position
            { -1.00f, +0.00f, +0.00f }, /// Normal
            { +1.00f, +1.00f         }  /// UV
        },

        /// +X平面
        {
            { +0.50f, -0.50f, +0.50f }, /// Position
            { +1.00f, +0.00f, +0.00f }, /// Normal
            { +1.00f, +0.00f         }  /// UV
        },
        {
            { +0.50f, -0.50f, -0.50f }, /// Position
            { +1.00f, +0.00f, +0.00f }, /// Normal
            { +0.00f, +0.00f         }  /// UV
        },
        {
            { +0.50f, +0.50f, +0.50f }, /// Position
            { +1.00f, +0.00f, +0.00f }, /// Normal
            { +1.00f, +1.00f         }  /// UV
        },
        {
            { +0.50f, +0.50f, -0.50f }, /// Position
            { +1.00f, +0.00f, +0.00f }, /// Normal
            { +0.00f, +1.00f         }  /// UV
        },

        /// +Y平面
        {
            { +0.50f, +0.50f, -0.50f }, /// Position
            { +0.00f, +1.00f, +0.00f }, /// Normal
            { +1.00f, +0.00f         }  /// UV
        },
        {
            { -0.50f, +0.50f, -0.50f }, /// Position
            { +0.00f, +1.00f, +0.00f }, /// Normal
            { +0.00f, +0.00f         }  /// UV
        },
        {
            { +0.50f, +0.50f, +0.50f }, /// Position
            { +0.00f, +1.00f, +0.00f }, /// Normal
            { +1.00f, +1.00f         }  /// UV
        },
        {
            { -0.50f, +0.50f, +0.50f }, /// Position
            { +0.00f, +1.00f, +0.00f }, /// Normal
            { +0.00f, +1.00f         }  /// UV
        },

        /// -Y平面
        {
            { +0.50f, -0.50f, +0.50f }, /// Position
            { +0.00f, -1.00f, +0.00f }, /// Normal
            { +1.00f, +1.00f         }  /// UV
        },
        {
            { -0.50f, -0.50f, +0.50f }, /// Position
            { +0.00f, -1.00f, +0.00f }, /// Normal
            { +0.00f, +1.00f         }  /// UV
        },
        {
            { +0.50f, -0.50f, -0.50f }, /// Position
            { +0.00f, -1.00f, +0.00f }, /// Normal
            { +1.00f, +0.00f         }  /// UV
        },
        {
            { -0.50f, -0.50f, -0.50f }, /// Position
            { +0.00f, -1.00f, +0.00f }, /// Normal
            { +0.00f, +0.00f         }  /// UV
        },

        /// +Z平面
        {
            { +0.50f, -0.50f, +0.50f }, /// Position
            { +0.00f, +0.00f, +1.00f }, /// Normal
            { +1.00f, +0.00f         }  /// UV
        },
        {
            { +0.50f, +0.50f, +0.50f }, /// Position
            { +0.00f, +0.00f, +1.00f }, /// Normal
            { +1.00f, +1.00f         }  /// UV
        },
        {
            { -0.50f, -0.50f, +0.50f }, /// Position
            { +0.00f, +0.00f, +1.00f }, /// Normal
            { +0.00f, +0.00f         }  /// UV
        },
        {
            { -0.50f, +0.50f, +0.50f }, /// Position
            { +0.00f, +0.00f, +1.00f }, /// Normal
            { +0.00f, +1.00f         }  /// UV
        }
    };

    /// 三角面列表
    static const IndexedTriangle TRIANGLE_LIST[] =
    {
        /// -Z平面:
        { 0, 2, 1 },
        { 1, 2, 3 },

        /// -X平面:
        { 4, 5, 6 },
        { 6, 5, 7 },

        /// +X平面:
        { 8, 9, 10 },
        { 10, 9, 11 },

        /// +Y平面:
        { 12, 13, 14 },
        { 14, 13, 15 },

        /// -Y平面:
        { 16, 17, 18 },
        { 18, 17, 19 },

        /// +Z平面:
        { 20, 21, 22 },
        { 22, 21, 23 }
    };

    /// #vertex
    static constexpr uint16_t VERTEX_COUNT   = ARRAY_LENGTH(VERTEX_LIST);
    /// #triangle
    static constexpr uint16_t TRIANGLE_COUNT = ARRAY_LENGTH(TRIANGLE_LIST);

    cache_mesh(
        MESH_ID, MESH_BOUND_BOX,
        VertexLayoutT::LAYOUT_DECL, VERTEX_COUNT, TRIANGLE_COUNT, false,
        (const uint8_t*)VERTEX_LIST, (uint32_t)sizeof(VERTEX_LIST),
        TRIANGLE_LIST, (uint32_t)sizeof(TRIANGLE_LIST));
    return MESH_ID;
}


RenderMeshIdT
RenderMeshDepot::create_unit_beveled_cube ()
{
    typedef struct Layout_Pos_Norm_Uv VertexLayoutT;

    /// 计算 Mesh Id
    static constexpr RenderMeshIdT MESH_ID =
        STATIC_RENDER_MESH_HASH(GENERATE_BUILTIN_NAME("UNIT_BEVELED_CUBE"));

    /// 包围盒
    static constexpr AABB MESH_BOUND_BOX
    {
        { -0.5f, -0.5f, -0.5f },
        { +0.5f, +0.5f, +0.5f }
    };

    /// 顶点列表: 低面数窄倒角Beveled Cube
    /// 倒角半径为0.04, 主平面范围为[-0.46, +0.46]
    /// 每条边使用一个倒角四边形, 通过圆角Normal让边缘显示更圆滑
    /// 每个逻辑面使用独立完整UV空间, 左下角为(0, 0)
    /// Face数据以-Z, +Z开头
    static constexpr VertexLayoutT VERTEX_LIST[] =
    {
        /// -Z main plane
        {
            { -0.460000f, -0.460000f, -0.500000f }, /// Position
            { +0.000000f, +0.000000f, -1.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, -0.460000f, -0.500000f }, /// Position
            { +0.000000f, +0.000000f, -1.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, +0.460000f, -0.500000f }, /// Position
            { +0.000000f, +0.000000f, -1.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        {
            { -0.460000f, +0.460000f, -0.500000f }, /// Position
            { +0.000000f, +0.000000f, -1.000000f }, /// Normal
            { +0.000000f, +1.000000f             }  /// UV
        },
        /// +Z main plane
        {
            { -0.460000f, -0.460000f, +0.500000f }, /// Position
            { +0.000000f, +0.000000f, +1.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, -0.460000f, +0.500000f }, /// Position
            { +0.000000f, +0.000000f, +1.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, +0.460000f, +0.500000f }, /// Position
            { +0.000000f, +0.000000f, +1.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        {
            { -0.460000f, +0.460000f, +0.500000f }, /// Position
            { +0.000000f, +0.000000f, +1.000000f }, /// Normal
            { +0.000000f, +1.000000f             }  /// UV
        },
        /// +X main plane
        {
            { +0.500000f, -0.460000f, -0.460000f }, /// Position
            { +1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.500000f, -0.460000f, +0.460000f }, /// Position
            { +1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.500000f, +0.460000f, +0.460000f }, /// Position
            { +1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        {
            { +0.500000f, +0.460000f, -0.460000f }, /// Position
            { +1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +0.000000f, +1.000000f             }  /// UV
        },
        /// -X main plane
        {
            { -0.500000f, -0.460000f, -0.460000f }, /// Position
            { -1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { -0.500000f, -0.460000f, +0.460000f }, /// Position
            { -1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { -0.500000f, +0.460000f, +0.460000f }, /// Position
            { -1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        {
            { -0.500000f, +0.460000f, -0.460000f }, /// Position
            { -1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +0.000000f, +1.000000f             }  /// UV
        },
        /// +Y main plane
        {
            { -0.460000f, +0.500000f, -0.460000f }, /// Position
            { +0.000000f, +1.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, +0.500000f, -0.460000f }, /// Position
            { +0.000000f, +1.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, +0.500000f, +0.460000f }, /// Position
            { +0.000000f, +1.000000f, +0.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        {
            { -0.460000f, +0.500000f, +0.460000f }, /// Position
            { +0.000000f, +1.000000f, +0.000000f }, /// Normal
            { +0.000000f, +1.000000f             }  /// UV
        },
        /// -Y main plane
        {
            { -0.460000f, -0.500000f, -0.460000f }, /// Position
            { +0.000000f, -1.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, -0.500000f, -0.460000f }, /// Position
            { +0.000000f, -1.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, -0.500000f, +0.460000f }, /// Position
            { +0.000000f, -1.000000f, +0.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        {
            { -0.460000f, -0.500000f, +0.460000f }, /// Position
            { +0.000000f, -1.000000f, +0.000000f }, /// Normal
            { +0.000000f, +1.000000f             }  /// UV
        },
        /// Edge -X -Z
        {
            { -0.500000f, -0.460000f, -0.460000f }, /// Position
            { -1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { -0.500000f, +0.460000f, -0.460000f }, /// Position
            { -1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { -0.460000f, +0.460000f, -0.500000f }, /// Position
            { +0.000000f, +0.000000f, -1.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        {
            { -0.460000f, -0.460000f, -0.500000f }, /// Position
            { +0.000000f, +0.000000f, -1.000000f }, /// Normal
            { +0.000000f, +1.000000f             }  /// UV
        },
        /// Edge +X -Z
        {
            { +0.500000f, -0.460000f, -0.460000f }, /// Position
            { +1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.500000f, +0.460000f, -0.460000f }, /// Position
            { +1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, +0.460000f, -0.500000f }, /// Position
            { +0.000000f, +0.000000f, -1.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        {
            { +0.460000f, -0.460000f, -0.500000f }, /// Position
            { +0.000000f, +0.000000f, -1.000000f }, /// Normal
            { +0.000000f, +1.000000f             }  /// UV
        },
        /// Edge -Y -Z
        {
            { -0.460000f, -0.500000f, -0.460000f }, /// Position
            { +0.000000f, -1.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, -0.500000f, -0.460000f }, /// Position
            { +0.000000f, -1.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, -0.460000f, -0.500000f }, /// Position
            { +0.000000f, +0.000000f, -1.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        {
            { -0.460000f, -0.460000f, -0.500000f }, /// Position
            { +0.000000f, +0.000000f, -1.000000f }, /// Normal
            { +0.000000f, +1.000000f             }  /// UV
        },
        /// Edge +Y -Z
        {
            { -0.460000f, +0.500000f, -0.460000f }, /// Position
            { +0.000000f, +1.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, +0.500000f, -0.460000f }, /// Position
            { +0.000000f, +1.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, +0.460000f, -0.500000f }, /// Position
            { +0.000000f, +0.000000f, -1.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        {
            { -0.460000f, +0.460000f, -0.500000f }, /// Position
            { +0.000000f, +0.000000f, -1.000000f }, /// Normal
            { +0.000000f, +1.000000f             }  /// UV
        },
        /// Edge -X +Z
        {
            { -0.500000f, -0.460000f, +0.460000f }, /// Position
            { -1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { -0.500000f, +0.460000f, +0.460000f }, /// Position
            { -1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { -0.460000f, +0.460000f, +0.500000f }, /// Position
            { +0.000000f, +0.000000f, +1.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        {
            { -0.460000f, -0.460000f, +0.500000f }, /// Position
            { +0.000000f, +0.000000f, +1.000000f }, /// Normal
            { +0.000000f, +1.000000f             }  /// UV
        },
        /// Edge +X +Z
        {
            { +0.500000f, -0.460000f, +0.460000f }, /// Position
            { +1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.500000f, +0.460000f, +0.460000f }, /// Position
            { +1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, +0.460000f, +0.500000f }, /// Position
            { +0.000000f, +0.000000f, +1.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        {
            { +0.460000f, -0.460000f, +0.500000f }, /// Position
            { +0.000000f, +0.000000f, +1.000000f }, /// Normal
            { +0.000000f, +1.000000f             }  /// UV
        },
        /// Edge -Y +Z
        {
            { -0.460000f, -0.500000f, +0.460000f }, /// Position
            { +0.000000f, -1.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, -0.500000f, +0.460000f }, /// Position
            { +0.000000f, -1.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, -0.460000f, +0.500000f }, /// Position
            { +0.000000f, +0.000000f, +1.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        {
            { -0.460000f, -0.460000f, +0.500000f }, /// Position
            { +0.000000f, +0.000000f, +1.000000f }, /// Normal
            { +0.000000f, +1.000000f             }  /// UV
        },
        /// Edge +Y +Z
        {
            { -0.460000f, +0.500000f, +0.460000f }, /// Position
            { +0.000000f, +1.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, +0.500000f, +0.460000f }, /// Position
            { +0.000000f, +1.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, +0.460000f, +0.500000f }, /// Position
            { +0.000000f, +0.000000f, +1.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        {
            { -0.460000f, +0.460000f, +0.500000f }, /// Position
            { +0.000000f, +0.000000f, +1.000000f }, /// Normal
            { +0.000000f, +1.000000f             }  /// UV
        },
        /// Edge -X +Y
        {
            { -0.500000f, +0.460000f, -0.460000f }, /// Position
            { -1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { -0.500000f, +0.460000f, +0.460000f }, /// Position
            { -1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { -0.460000f, +0.500000f, +0.460000f }, /// Position
            { +0.000000f, +1.000000f, +0.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        {
            { -0.460000f, +0.500000f, -0.460000f }, /// Position
            { +0.000000f, +1.000000f, +0.000000f }, /// Normal
            { +0.000000f, +1.000000f             }  /// UV
        },
        /// Edge +X +Y
        {
            { +0.500000f, +0.460000f, -0.460000f }, /// Position
            { +1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.500000f, +0.460000f, +0.460000f }, /// Position
            { +1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, +0.500000f, +0.460000f }, /// Position
            { +0.000000f, +1.000000f, +0.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        {
            { +0.460000f, +0.500000f, -0.460000f }, /// Position
            { +0.000000f, +1.000000f, +0.000000f }, /// Normal
            { +0.000000f, +1.000000f             }  /// UV
        },
        /// Edge -X -Y
        {
            { -0.500000f, -0.460000f, -0.460000f }, /// Position
            { -1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { -0.500000f, -0.460000f, +0.460000f }, /// Position
            { -1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { -0.460000f, -0.500000f, +0.460000f }, /// Position
            { +0.000000f, -1.000000f, +0.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        {
            { -0.460000f, -0.500000f, -0.460000f }, /// Position
            { +0.000000f, -1.000000f, +0.000000f }, /// Normal
            { +0.000000f, +1.000000f             }  /// UV
        },
        /// Edge +X -Y
        {
            { +0.500000f, -0.460000f, -0.460000f }, /// Position
            { +1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.500000f, -0.460000f, +0.460000f }, /// Position
            { +1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, -0.500000f, +0.460000f }, /// Position
            { +0.000000f, -1.000000f, +0.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        {
            { +0.460000f, -0.500000f, -0.460000f }, /// Position
            { +0.000000f, -1.000000f, +0.000000f }, /// Normal
            { +0.000000f, +1.000000f             }  /// UV
        },
        /// Corner -X -Y -Z
        {
            { -0.500000f, -0.460000f, -0.460000f }, /// Position
            { -1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { -0.460000f, -0.500000f, -0.460000f }, /// Position
            { +0.000000f, -1.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { -0.460000f, -0.460000f, -0.500000f }, /// Position
            { +0.000000f, +0.000000f, -1.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        /// Corner +X -Y -Z
        {
            { +0.500000f, -0.460000f, -0.460000f }, /// Position
            { +1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, -0.500000f, -0.460000f }, /// Position
            { +0.000000f, -1.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, -0.460000f, -0.500000f }, /// Position
            { +0.000000f, +0.000000f, -1.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        /// Corner -X +Y -Z
        {
            { -0.500000f, +0.460000f, -0.460000f }, /// Position
            { -1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { -0.460000f, +0.500000f, -0.460000f }, /// Position
            { +0.000000f, +1.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { -0.460000f, +0.460000f, -0.500000f }, /// Position
            { +0.000000f, +0.000000f, -1.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        /// Corner +X +Y -Z
        {
            { +0.500000f, +0.460000f, -0.460000f }, /// Position
            { +1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, +0.500000f, -0.460000f }, /// Position
            { +0.000000f, +1.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, +0.460000f, -0.500000f }, /// Position
            { +0.000000f, +0.000000f, -1.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        /// Corner -X -Y +Z
        {
            { -0.500000f, -0.460000f, +0.460000f }, /// Position
            { -1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { -0.460000f, -0.500000f, +0.460000f }, /// Position
            { +0.000000f, -1.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { -0.460000f, -0.460000f, +0.500000f }, /// Position
            { +0.000000f, +0.000000f, +1.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        /// Corner +X -Y +Z
        {
            { +0.500000f, -0.460000f, +0.460000f }, /// Position
            { +1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, -0.500000f, +0.460000f }, /// Position
            { +0.000000f, -1.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, -0.460000f, +0.500000f }, /// Position
            { +0.000000f, +0.000000f, +1.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        /// Corner -X +Y +Z
        {
            { -0.500000f, +0.460000f, +0.460000f }, /// Position
            { -1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { -0.460000f, +0.500000f, +0.460000f }, /// Position
            { +0.000000f, +1.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { -0.460000f, +0.460000f, +0.500000f }, /// Position
            { +0.000000f, +0.000000f, +1.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        },
        /// Corner +X +Y +Z
        {
            { +0.500000f, +0.460000f, +0.460000f }, /// Position
            { +1.000000f, +0.000000f, +0.000000f }, /// Normal
            { +0.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, +0.500000f, +0.460000f }, /// Position
            { +0.000000f, +1.000000f, +0.000000f }, /// Normal
            { +1.000000f, +0.000000f             }  /// UV
        },
        {
            { +0.460000f, +0.460000f, +0.500000f }, /// Position
            { +0.000000f, +0.000000f, +1.000000f }, /// Normal
            { +1.000000f, +1.000000f             }  /// UV
        }
    };

    /// 三角面列表
    static const IndexedTriangle TRIANGLE_LIST[] =
    {
        {  0,  2,  1 }, {  0,  3,  2 }, {  4,  5,  6 }, {  4,  6,  7 },
        {  8, 10,  9 }, {  8, 11, 10 }, { 12, 13, 14 }, { 12, 14, 15 },
        { 16, 18, 17 }, { 16, 19, 18 }, { 20, 21, 22 }, { 20, 22, 23 },
        { 24, 25, 26 }, { 24, 26, 27 }, { 28, 30, 29 }, { 28, 31, 30 },
        { 32, 34, 33 }, { 32, 35, 34 }, { 36, 37, 38 }, { 36, 38, 39 },
        { 40, 42, 41 }, { 40, 43, 42 }, { 44, 45, 46 }, { 44, 46, 47 },
        { 48, 49, 50 }, { 48, 50, 51 }, { 52, 54, 53 }, { 52, 55, 54 },
        { 56, 57, 58 }, { 56, 58, 59 }, { 60, 62, 61 }, { 60, 63, 62 },
        { 64, 66, 65 }, { 64, 67, 66 }, { 68, 69, 70 }, { 68, 70, 71 },
        { 72, 74, 73 }, { 75, 76, 77 }, { 78, 79, 80 }, { 81, 83, 82 },
        { 84, 85, 86 }, { 87, 89, 88 }, { 90, 92, 91 }, { 93, 94, 95 }
    };

    /// #vertex
    static constexpr uint16_t VERTEX_COUNT   = ARRAY_LENGTH(VERTEX_LIST);
    /// #triangle
    static constexpr uint16_t TRIANGLE_COUNT = ARRAY_LENGTH(TRIANGLE_LIST);

    cache_mesh(
        MESH_ID, MESH_BOUND_BOX,
        VertexLayoutT::LAYOUT_DECL, VERTEX_COUNT, TRIANGLE_COUNT, false,
        (const uint8_t*)VERTEX_LIST, (uint32_t)sizeof(VERTEX_LIST),
        TRIANGLE_LIST, (uint32_t)sizeof(TRIANGLE_LIST));
    return MESH_ID;
}


RenderMeshIdT
RenderMeshDepot::create_unit_icosahedron ()
{
    typedef struct Layout_Pos VertexLayoutT;

    /// 计算 Mesh Id
    static constexpr RenderMeshIdT MESH_ID =
        STATIC_RENDER_MESH_HASH(GENERATE_BUILTIN_NAME("UNIT_ICOSAHEDRON"));

    /// 参考:
    ///  http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html#comment-2736314361
    ///
    /// Golden Ratio(为了将顶点放置在球体上)
    /// 顶点使用{0, +/-S, +/-T}的排列组合来生成
    /// 这里:
    ///             5 - sqrt(5)
    /// - S = sqrt(─────────────) * 球半径
    ///                 10
    ///             5 + sqrt(5)
    /// - T = sqrt(─────────────) * 球半径
    ///                 10
    static constexpr float S_VALUE = 0.2628655561f;
    static constexpr float T_VALUE = 0.4253254042f;

    /// 包围盒
    static constexpr AABB MESH_BOUND_BOX
    {
        { -(T_VALUE + 0.01f), -(T_VALUE + 0.01f), -(T_VALUE + 0.01f) },
        { +(T_VALUE + 0.01f), +(T_VALUE + 0.01f), +(T_VALUE + 0.01f) }
    };

    /// 顶点列表
    static constexpr VertexLayoutT VERTEX_LIST[] =
    {
        /// YZ平面(蓝色平面): {0, +/-T, -/+S}: Y值比Z值大
        /// P0: <0, +T, -S>: 红
        {
            { 0.0f, +T_VALUE, -S_VALUE },
        },
        /// P1: <0, +T, +S>: 绿
        {
            { 0.0f, +T_VALUE, +S_VALUE },
        },
        /// P2: <0, -T, -S>: 蓝
        {
            { 0.0f, -T_VALUE, -S_VALUE },
        },
        /// P3: <0, -T, +S>: 黄
        {
            { 0.0f, -T_VALUE, +S_VALUE },
        },

        //// XY平面(绿色平面): {+/-T, -/+S, 0}: X值大于Y值
        /// P4: <+T, -S, 0>: 青
        {
            { +T_VALUE, -S_VALUE, 0.0f },
        },
        /// P5: <+T, +S, 0>: 品红
        {
            { +T_VALUE, +S_VALUE, 0.0f },
        },
        /// P6: <-T, -S, 0>: 橙
        {
            { -T_VALUE, -S_VALUE, 0.0f },
        },
        /// P7: <-T, +S, 0>: 紫
        {
            { -T_VALUE, +S_VALUE, 0.0f },
        },

        //// XZ平面(红色平面): {-/+S, 0, +/-T}: Z值大于Y值
        /// P8:  <-S, 0, +T>: 黄绿
        {
            { -S_VALUE, 0.0f, +T_VALUE },
        },
        /// P9:  <+S, 0, +T>: 天蓝
        {
            { +S_VALUE, 0.0f, +T_VALUE },
        },
        /// P10: <-S, 0, -T>: 粉红
        {
            { -S_VALUE, 0.0f, -T_VALUE },
        },
        /// P11: <+S, 0, -T>: 灰
        {
            { +S_VALUE, 0.0f, -T_VALUE },
        }
    };

    /// 三角面列表
    static const IndexedTriangle TRIANGLE_LIST[] =
    {
        /// - <0, 1, 2> --> <0, 2, 1>
        ///
        /// P0处的5个三角面
        { 0, 5,  11 },
        { 0, 1,  5  },
        { 0, 7,  1  },
        { 0, 10, 7  },
        { 0, 11, 10 },

        /// 5个相邻的三角面
        { 1,  9, 5  },
        { 5,  4, 11 },
        { 11, 2, 10 },
        { 10, 6, 7  },
        { 7,  8, 1  },

        /// P3处的5个三角面
        { 3, 4, 9 },
        { 3, 2, 4 },
        { 3, 6, 2 },
        { 3, 8, 6 },
        { 3, 9, 8 },

        /// 5个相邻的三角面
        { 4, 5,  9 },
        { 2, 11, 4 },
        { 6, 10, 2 },
        { 8, 7,  6 },
        { 9, 1,  8 }
    };

    /// #vertex
    static constexpr uint16_t VERTEX_COUNT   = ARRAY_LENGTH(VERTEX_LIST);
    /// #triangle
    static constexpr uint16_t TRIANGLE_COUNT = ARRAY_LENGTH(TRIANGLE_LIST);

    cache_mesh(
        MESH_ID, MESH_BOUND_BOX,
        VertexLayoutT::LAYOUT_DECL, VERTEX_COUNT, TRIANGLE_COUNT, false,
        (const uint8_t*)VERTEX_LIST, (uint32_t)sizeof(VERTEX_LIST),
        TRIANGLE_LIST, (uint32_t)sizeof(TRIANGLE_LIST));
    return MESH_ID;
}


RenderMeshIdT
RenderMeshDepot::create_unit_icosphere ()
{
    typedef struct Layout_Pos_Norm_Uv VertexLayoutT;

    /// 计算 Mesh Id
    static constexpr RenderMeshIdT MESH_ID =
        STATIC_RENDER_MESH_HASH(GENERATE_BUILTIN_NAME("UNIT_ICOSPHERE"));

    /// 包围盒
    static constexpr AABB MESH_BOUND_BOX
    {
        { -0.51f, -0.51f, -0.51f },
        { +0.51f, +0.51f, +0.51f }
    };

    /// 顶点列表
    static constexpr VertexLayoutT VERTEX_LIST[] =
    {
        /// Vertex 0
        {
            { +0.0000000f, -0.5000000f, +0.0000000f }, /// Position
            { +0.0000000f, -1.0000000f, +0.0000000f }, /// Normal
            { +0.1250000f, +0.0000000f              }  /// UV
        },
        /// Vertex 1
        {
            { +0.0000000f, -0.4743416f, +0.1581139f }, /// Position
            { +0.0000000f, -0.9486833f, +0.3162278f }, /// Normal
            { +0.0000000f, +0.1024164f              }  /// UV
        },
        /// Vertex 2
        {
            { -0.1581139f, -0.4743416f, +0.0000000f }, /// Position
            { -0.3162278f, -0.9486833f, +0.0000000f }, /// Normal
            { +0.2500000f, +0.1024164f              }  /// UV
        },
        /// Vertex 3
        {
            { -0.2041241f, -0.4082483f, +0.2041241f }, /// Position
            { -0.4082483f, -0.8164966f, +0.4082483f }, /// Normal
            { +0.1250000f, +0.1959133f              }  /// UV
        },
        /// Vertex 4
        {
            { +0.0000000f, -0.3535534f, +0.3535534f }, /// Position
            { +0.0000000f, -0.7071068f, +0.7071068f }, /// Normal
            { +0.0000000f, +0.2500000f              }  /// UV
        },
        /// Vertex 5
        {
            { -0.3535534f, -0.3535534f, +0.0000000f }, /// Position
            { -0.7071068f, -0.7071068f, +0.0000000f }, /// Normal
            { +0.2500000f, +0.2500000f              }  /// UV
        },
        /// Vertex 6
        {
            { -0.2041241f, -0.4082483f, -0.2041241f }, /// Position
            { -0.4082483f, -0.8164966f, -0.4082483f }, /// Normal
            { +0.3750000f, +0.1959133f              }  /// UV
        },
        /// Vertex 7
        {
            { +0.0000000f, -0.4743416f, -0.1581139f }, /// Position
            { +0.0000000f, -0.9486833f, -0.3162278f }, /// Normal
            { +0.5000000f, +0.1024164f              }  /// UV
        },
        /// Vertex 8
        {
            { +0.0000000f, -0.5000000f, +0.0000000f }, /// Position
            { +0.0000000f, -1.0000000f, +0.0000000f }, /// Normal
            { +0.3750000f, +0.0000000f              }  /// UV
        },
        /// Vertex 9
        {
            { +0.0000000f, -0.3535534f, -0.3535534f }, /// Position
            { +0.0000000f, -0.7071068f, -0.7071068f }, /// Normal
            { +0.5000000f, +0.2500000f              }  /// UV
        },
        /// Vertex 10
        {
            { +0.2041241f, -0.4082483f, -0.2041241f }, /// Position
            { +0.4082483f, -0.8164966f, -0.4082483f }, /// Normal
            { +0.6250000f, +0.1959133f              }  /// UV
        },
        /// Vertex 11
        {
            { +0.1581139f, -0.4743416f, +0.0000000f }, /// Position
            { +0.3162278f, -0.9486833f, +0.0000000f }, /// Normal
            { +0.7500000f, +0.1024164f              }  /// UV
        },
        /// Vertex 12
        {
            { +0.0000000f, -0.5000000f, +0.0000000f }, /// Position
            { +0.0000000f, -1.0000000f, +0.0000000f }, /// Normal
            { +0.6250000f, +0.0000000f              }  /// UV
        },
        /// Vertex 13
        {
            { +0.3535534f, -0.3535534f, +0.0000000f }, /// Position
            { +0.7071068f, -0.7071068f, +0.0000000f }, /// Normal
            { +0.7500000f, +0.2500000f              }  /// UV
        },
        /// Vertex 14
        {
            { +0.2041241f, -0.4082483f, +0.2041241f }, /// Position
            { +0.4082483f, -0.8164966f, +0.4082483f }, /// Normal
            { +0.8750000f, +0.1959133f              }  /// UV
        },
        /// Vertex 15
        {
            { +0.0000000f, -0.4743416f, +0.1581139f }, /// Position
            { +0.0000000f, -0.9486833f, +0.3162278f }, /// Normal
            { +1.0000000f, +0.1024164f              }  /// UV
        },
        /// Vertex 16
        {
            { +0.0000000f, -0.5000000f, +0.0000000f }, /// Position
            { +0.0000000f, -1.0000000f, +0.0000000f }, /// Normal
            { +0.8750000f, +0.0000000f              }  /// UV
        },
        /// Vertex 17
        {
            { +0.0000000f, -0.3535534f, +0.3535534f }, /// Position
            { +0.0000000f, -0.7071068f, +0.7071068f }, /// Normal
            { +1.0000000f, +0.2500000f              }  /// UV
        },
        /// Vertex 18
        {
            { -0.2041241f, -0.2041241f, +0.4082483f }, /// Position
            { -0.4082483f, -0.4082483f, +0.8164966f }, /// Normal
            { +0.0737918f, +0.3661398f              }  /// UV
        },
        /// Vertex 19
        {
            { +0.0000000f, -0.1581139f, +0.4743416f }, /// Position
            { +0.0000000f, -0.3162278f, +0.9486833f }, /// Normal
            { +0.0000000f, +0.3975836f              }  /// UV
        },
        /// Vertex 20
        {
            { -0.4082483f, -0.2041241f, -0.2041241f }, /// Position
            { -0.8164966f, -0.4082483f, -0.4082483f }, /// Normal
            { +0.3237918f, +0.3661398f              }  /// UV
        },
        /// Vertex 21
        {
            { -0.4743416f, -0.1581139f, +0.0000000f }, /// Position
            { -0.9486833f, -0.3162278f, +0.0000000f }, /// Normal
            { +0.2500000f, +0.3975836f              }  /// UV
        },
        /// Vertex 22
        {
            { -0.2041241f, -0.2041241f, -0.4082483f }, /// Position
            { -0.4082482f, -0.4082483f, -0.8164966f }, /// Normal
            { +0.4262082f, +0.3661398f              }  /// UV
        },
        /// Vertex 23
        {
            { +0.0000000f, -0.1581139f, -0.4743416f }, /// Position
            { +0.0000000f, -0.3162278f, -0.9486833f }, /// Normal
            { +0.5000000f, +0.3975836f              }  /// UV
        },
        /// Vertex 24
        {
            { +0.2041241f, -0.2041241f, -0.4082483f }, /// Position
            { +0.4082483f, -0.4082483f, -0.8164966f }, /// Normal
            { +0.5737918f, +0.3661398f              }  /// UV
        },
        /// Vertex 25
        {
            { +0.4082483f, -0.2041241f, -0.2041241f }, /// Position
            { +0.8164966f, -0.4082483f, -0.4082482f }, /// Normal
            { +0.6762083f, +0.3661398f              }  /// UV
        },
        /// Vertex 26
        {
            { +0.4743416f, -0.1581139f, +0.0000000f }, /// Position
            { +0.9486833f, -0.3162278f, +0.0000000f }, /// Normal
            { +0.7500000f, +0.3975836f              }  /// UV
        },
        /// Vertex 27
        {
            { +0.2041241f, -0.2041241f, +0.4082483f }, /// Position
            { +0.4082482f, -0.4082483f, +0.8164966f }, /// Normal
            { +0.9262082f, +0.3661398f              }  /// UV
        },
        /// Vertex 28
        {
            { +0.0000000f, -0.1581139f, +0.4743416f }, /// Position
            { +0.0000000f, -0.3162278f, +0.9486833f }, /// Normal
            { +1.0000000f, +0.3975836f              }  /// UV
        },
        /// Vertex 29
        {
            { -0.1581139f, +0.0000000f, +0.4743416f }, /// Position
            { -0.3162278f, +0.0000000f, +0.9486833f }, /// Normal
            { +0.0512082f, +0.5000000f              }  /// UV
        },
        /// Vertex 30
        {
            { +0.0000000f, +0.0000000f, +0.5000000f }, /// Position
            { +0.0000000f, +0.0000000f, +1.0000000f }, /// Normal
            { +0.0000000f, +0.5000000f              }  /// UV
        },
        /// Vertex 31
        {
            { -0.3535534f, +0.0000000f, +0.3535534f }, /// Position
            { -0.7071068f, +0.0000000f, +0.7071068f }, /// Normal
            { +0.1250000f, +0.5000000f              }  /// UV
        },
        /// Vertex 32
        {
            { -0.4743416f, +0.0000000f, -0.1581139f }, /// Position
            { -0.9486833f, +0.0000000f, -0.3162278f }, /// Normal
            { +0.3012082f, +0.5000000f              }  /// UV
        },
        /// Vertex 33
        {
            { -0.5000000f, +0.0000000f, +0.0000000f }, /// Position
            { -1.0000000f, +0.0000000f, +0.0000000f }, /// Normal
            { +0.2500000f, +0.5000000f              }  /// UV
        },
        /// Vertex 34
        {
            { -0.4743416f, +0.0000000f, +0.1581139f }, /// Position
            { -0.9486833f, +0.0000000f, +0.3162278f }, /// Normal
            { +0.1987918f, +0.5000000f              }  /// UV
        },
        /// Vertex 35
        {
            { -0.4082483f, -0.2041241f, +0.2041241f }, /// Position
            { -0.8164966f, -0.4082483f, +0.4082482f }, /// Normal
            { +0.1762082f, +0.3661398f              }  /// UV
        },
        /// Vertex 36
        {
            { +0.4743416f, +0.0000000f, -0.1581139f }, /// Position
            { +0.9486833f, +0.0000000f, -0.3162278f }, /// Normal
            { +0.6987918f, +0.5000000f              }  /// UV
        },
        /// Vertex 37
        {
            { +0.3535534f, +0.0000000f, -0.3535534f }, /// Position
            { +0.7071068f, +0.0000000f, -0.7071068f }, /// Normal
            { +0.6250000f, +0.5000000f              }  /// UV
        },
        /// Vertex 38
        {
            { +0.5000000f, +0.0000000f, +0.0000000f }, /// Position
            { +1.0000000f, +0.0000000f, +0.0000000f }, /// Normal
            { +0.7500000f, +0.5000000f              }  /// UV
        },
        /// Vertex 39
        {
            { +0.1581139f, +0.0000000f, +0.4743416f }, /// Position
            { +0.3162278f, +0.0000000f, +0.9486833f }, /// Normal
            { +0.9487918f, +0.5000000f              }  /// UV
        },
        /// Vertex 40
        {
            { +0.0000000f, +0.0000000f, +0.5000000f }, /// Position
            { +0.0000000f, +0.0000000f, +1.0000000f }, /// Normal
            { +1.0000000f, +0.5000000f              }  /// UV
        },
        /// Vertex 41
        {
            { +0.0000000f, +0.1581139f, +0.4743416f }, /// Position
            { +0.0000000f, +0.3162278f, +0.9486833f }, /// Normal
            { +0.0000000f, +0.6024164f              }  /// UV
        },
        /// Vertex 42
        {
            { -0.2041241f, +0.2041241f, +0.4082483f }, /// Position
            { -0.4082483f, +0.4082483f, +0.8164966f }, /// Normal
            { +0.0737918f, +0.6338602f              }  /// UV
        },
        /// Vertex 43
        {
            { -0.4082483f, +0.2041241f, +0.2041241f }, /// Position
            { -0.8164966f, +0.4082483f, +0.4082482f }, /// Normal
            { +0.1762082f, +0.6338602f              }  /// UV
        },
        /// Vertex 44
        {
            { -0.4743416f, +0.1581139f, +0.0000000f }, /// Position
            { -0.9486833f, +0.3162278f, +0.0000000f }, /// Normal
            { +0.2500000f, +0.6024164f              }  /// UV
        },
        /// Vertex 45
        {
            { -0.4082483f, +0.2041241f, -0.2041241f }, /// Position
            { -0.8164966f, +0.4082483f, -0.4082483f }, /// Normal
            { +0.3237918f, +0.6338602f              }  /// UV
        },
        /// Vertex 46
        {
            { -0.3535534f, +0.0000000f, -0.3535534f }, /// Position
            { -0.7071068f, +0.0000000f, -0.7071068f }, /// Normal
            { +0.3750000f, +0.5000000f              }  /// UV
        },
        /// Vertex 47
        {
            { -0.1581139f, +0.0000000f, -0.4743416f }, /// Position
            { -0.3162278f, +0.0000000f, -0.9486833f }, /// Normal
            { +0.4487918f, +0.5000000f              }  /// UV
        },
        /// Vertex 48
        {
            { +0.0000000f, +0.0000000f, -0.5000000f }, /// Position
            { +0.0000000f, +0.0000000f, -1.0000000f }, /// Normal
            { +0.5000000f, +0.5000000f              }  /// UV
        },
        /// Vertex 49
        {
            { +0.1581139f, +0.0000000f, -0.4743416f }, /// Position
            { +0.3162278f, +0.0000000f, -0.9486833f }, /// Normal
            { +0.5512082f, +0.5000000f              }  /// UV
        },
        /// Vertex 50
        {
            { -0.2041241f, +0.2041241f, -0.4082483f }, /// Position
            { -0.4082482f, +0.4082483f, -0.8164966f }, /// Normal
            { +0.4262082f, +0.6338602f              }  /// UV
        },
        /// Vertex 51
        {
            { +0.0000000f, +0.1581139f, -0.4743416f }, /// Position
            { +0.0000000f, +0.3162278f, -0.9486833f }, /// Normal
            { +0.5000000f, +0.6024164f              }  /// UV
        },
        /// Vertex 52
        {
            { +0.2041241f, +0.2041241f, -0.4082483f }, /// Position
            { +0.4082483f, +0.4082483f, -0.8164966f }, /// Normal
            { +0.5737918f, +0.6338602f              }  /// UV
        },
        /// Vertex 53
        {
            { +0.4082483f, +0.2041241f, -0.2041241f }, /// Position
            { +0.8164966f, +0.4082483f, -0.4082482f }, /// Normal
            { +0.6762083f, +0.6338602f              }  /// UV
        },
        /// Vertex 54
        {
            { +0.4743416f, +0.1581139f, +0.0000000f }, /// Position
            { +0.9486833f, +0.3162278f, +0.0000000f }, /// Normal
            { +0.7500000f, +0.6024164f              }  /// UV
        },
        /// Vertex 55
        {
            { +0.4743416f, +0.0000000f, +0.1581139f }, /// Position
            { +0.9486833f, +0.0000000f, +0.3162278f }, /// Normal
            { +0.8012082f, +0.5000000f              }  /// UV
        },
        /// Vertex 56
        {
            { +0.4082483f, -0.2041241f, +0.2041241f }, /// Position
            { +0.8164966f, -0.4082483f, +0.4082483f }, /// Normal
            { +0.8237918f, +0.3661398f              }  /// UV
        },
        /// Vertex 57
        {
            { +0.3535534f, +0.0000000f, +0.3535534f }, /// Position
            { +0.7071068f, +0.0000000f, +0.7071068f }, /// Normal
            { +0.8750000f, +0.5000000f              }  /// UV
        },
        /// Vertex 58
        {
            { +0.4082483f, +0.2041241f, +0.2041241f }, /// Position
            { +0.8164966f, +0.4082483f, +0.4082483f }, /// Normal
            { +0.8237918f, +0.6338602f              }  /// UV
        },
        /// Vertex 59
        {
            { +0.2041241f, +0.2041241f, +0.4082483f }, /// Position
            { +0.4082482f, +0.4082483f, +0.8164966f }, /// Normal
            { +0.9262082f, +0.6338602f              }  /// UV
        },
        /// Vertex 60
        {
            { +0.0000000f, +0.1581139f, +0.4743416f }, /// Position
            { +0.0000000f, +0.3162278f, +0.9486833f }, /// Normal
            { +1.0000000f, +0.6024164f              }  /// UV
        },
        /// Vertex 61
        {
            { +0.3535534f, +0.3535534f, +0.0000000f }, /// Position
            { +0.7071068f, +0.7071068f, +0.0000000f }, /// Normal
            { +0.7500000f, +0.7500000f              }  /// UV
        },
        /// Vertex 62
        {
            { +0.2041241f, +0.4082483f, +0.2041241f }, /// Position
            { +0.4082483f, +0.8164966f, +0.4082483f }, /// Normal
            { +0.8750000f, +0.8040867f              }  /// UV
        },
        /// Vertex 63
        {
            { +0.0000000f, +0.3535534f, +0.3535534f }, /// Position
            { +0.0000000f, +0.7071068f, +0.7071068f }, /// Normal
            { +1.0000000f, +0.7500000f              }  /// UV
        },
        /// Vertex 64
        {
            { +0.1581139f, +0.4743416f, +0.0000000f }, /// Position
            { +0.3162278f, +0.9486833f, +0.0000000f }, /// Normal
            { +0.7500000f, +0.8975835f              }  /// UV
        },
        /// Vertex 65
        {
            { +0.2041241f, +0.4082483f, -0.2041241f }, /// Position
            { +0.4082483f, +0.8164966f, -0.4082483f }, /// Normal
            { +0.6250000f, +0.8040867f              }  /// UV
        },
        /// Vertex 66
        {
            { +0.0000000f, +0.3535534f, -0.3535534f }, /// Position
            { +0.0000000f, +0.7071068f, -0.7071068f }, /// Normal
            { +0.5000000f, +0.7500000f              }  /// UV
        },
        /// Vertex 67
        {
            { -0.2041241f, +0.4082483f, -0.2041241f }, /// Position
            { -0.4082483f, +0.8164966f, -0.4082483f }, /// Normal
            { +0.3750000f, +0.8040867f              }  /// UV
        },
        /// Vertex 68
        {
            { -0.3535534f, +0.3535534f, +0.0000000f }, /// Position
            { -0.7071068f, +0.7071068f, +0.0000000f }, /// Normal
            { +0.2500000f, +0.7500000f              }  /// UV
        },
        /// Vertex 69
        {
            { -0.2041241f, +0.4082483f, +0.2041241f }, /// Position
            { -0.4082483f, +0.8164966f, +0.4082483f }, /// Normal
            { +0.1250000f, +0.8040867f              }  /// UV
        },
        /// Vertex 70
        {
            { +0.0000000f, +0.3535534f, +0.3535534f }, /// Position
            { +0.0000000f, +0.7071068f, +0.7071068f }, /// Normal
            { +0.0000000f, +0.7500000f              }  /// UV
        },
        /// Vertex 71
        {
            { +0.0000000f, +0.4743416f, +0.1581139f }, /// Position
            { +0.0000000f, +0.9486833f, +0.3162278f }, /// Normal
            { +0.0000000f, +0.8975835f              }  /// UV
        },
        /// Vertex 72
        {
            { -0.1581139f, +0.4743416f, +0.0000000f }, /// Position
            { -0.3162278f, +0.9486833f, +0.0000000f }, /// Normal
            { +0.2500000f, +0.8975835f              }  /// UV
        },
        /// Vertex 73
        {
            { +0.0000000f, +0.4743416f, -0.1581139f }, /// Position
            { +0.0000000f, +0.9486833f, -0.3162278f }, /// Normal
            { +0.5000000f, +0.8975835f              }  /// UV
        },
        /// Vertex 74
        {
            { +0.0000000f, +0.5000000f, +0.0000000f }, /// Position
            { +0.0000000f, +1.0000000f, +0.0000000f }, /// Normal
            { +0.1250000f, +1.0000000f              }  /// UV
        },
        /// Vertex 75
        {
            { +0.0000000f, +0.5000000f, +0.0000000f }, /// Position
            { +0.0000000f, +1.0000000f, +0.0000000f }, /// Normal
            { +0.3750000f, +1.0000000f              }  /// UV
        },
        /// Vertex 76
        {
            { +0.0000000f, +0.5000000f, +0.0000000f }, /// Position
            { +0.0000000f, +1.0000000f, +0.0000000f }, /// Normal
            { +0.6250000f, +1.0000000f              }  /// UV
        },
        /// Vertex 77
        {
            { +0.0000000f, +0.4743416f, +0.1581139f }, /// Position
            { +0.0000000f, +0.9486833f, +0.3162278f }, /// Normal
            { +1.0000000f, +0.8975835f              }  /// UV
        },
        /// Vertex 78
        {
            { +0.0000000f, +0.5000000f, +0.0000000f }, /// Position
            { +0.0000000f, +1.0000000f, +0.0000000f }, /// Normal
            { +0.8750000f, +1.0000000f              }  /// UV
        }
    };

    /// 三角面列表
    static const IndexedTriangle TRIANGLE_LIST[] =
    {
        {  0,  1,  2 }, {  1,  3,  2 }, {  1,  4,  3 }, {  2,  3,  5 },
        {  2,  5,  6 }, {  2,  6,  7 }, {  8,  2,  7 }, {  7,  6,  9 },
        {  7,  9, 10 }, {  7, 10, 11 }, { 12,  7, 11 }, { 11, 10, 13 },
        { 11, 13, 14 }, { 11, 14, 15 }, { 16, 11, 15 }, { 15, 14, 17 },
        {  4, 18,  3 }, {  4, 19, 18 }, {  5, 20,  6 }, {  5, 21, 20 },
        {  6, 20, 22 }, {  6, 22,  9 }, {  9, 22, 23 }, {  9, 23, 24 },
        {  9, 24, 10 }, { 10, 24, 25 }, { 10, 25, 13 }, { 13, 25, 26 },
        { 14, 27, 17 }, { 17, 27, 28 }, { 19, 29, 18 }, { 19, 30, 29 },
        { 18, 29, 31 }, { 21, 32, 20 }, { 21, 33, 32 }, { 21, 34, 33 },
        { 35, 31, 34 }, { 18, 31, 35 }, {  3, 18, 35 }, {  3, 35,  5 },
        {  5, 35, 21 }, { 35, 34, 21 }, { 25, 36, 26 }, { 25, 37, 36 },
        { 26, 36, 38 }, { 27, 39, 28 }, { 28, 39, 40 }, { 30, 41, 29 },
        { 41, 42, 29 }, { 29, 42, 31 }, { 42, 43, 31 }, { 31, 43, 34 },
        { 43, 44, 34 }, { 34, 44, 33 }, { 33, 44, 32 }, { 44, 45, 32 },
        { 32, 45, 46 }, { 20, 32, 46 }, { 20, 46, 22 }, { 22, 46, 47 },
        { 22, 47, 23 }, { 23, 47, 48 }, { 23, 48, 49 }, { 23, 49, 24 },
        { 24, 49, 37 }, { 24, 37, 25 }, { 45, 50, 46 }, { 46, 50, 47 },
        { 50, 51, 47 }, { 47, 51, 48 }, { 48, 51, 49 }, { 51, 52, 49 },
        { 49, 52, 37 }, { 52, 53, 37 }, { 37, 53, 36 }, { 53, 54, 36 },
        { 36, 54, 38 }, { 38, 54, 55 }, { 26, 38, 55 }, { 26, 55, 56 },
        { 13, 26, 56 }, { 13, 56, 14 }, { 14, 56, 27 }, { 56, 55, 57 },
        { 56, 57, 27 }, { 27, 57, 39 }, { 54, 58, 55 }, { 55, 58, 57 },
        { 58, 59, 57 }, { 57, 59, 39 }, { 59, 60, 39 }, { 39, 60, 40 },
        { 53, 61, 54 }, { 54, 61, 58 }, { 61, 62, 58 }, { 58, 62, 59 },
        { 62, 63, 59 }, { 59, 63, 60 }, { 61, 64, 62 }, { 65, 64, 61 },
        { 65, 61, 53 }, { 52, 65, 53 }, { 66, 65, 52 }, { 51, 66, 52 },
        { 50, 66, 51 }, { 67, 66, 50 }, { 45, 67, 50 }, { 68, 67, 45 },
        { 44, 68, 45 }, { 43, 68, 44 }, { 69, 68, 43 }, { 42, 69, 43 },
        { 70, 69, 42 }, { 41, 70, 42 }, { 70, 71, 69 }, { 71, 72, 69 },
        { 69, 72, 68 }, { 68, 72, 67 }, { 72, 73, 67 }, { 67, 73, 66 },
        { 66, 73, 65 }, { 73, 64, 65 }, { 71, 74, 72 }, { 72, 75, 73 },
        { 73, 76, 64 }, { 64, 77, 62 }, { 62, 77, 63 }, { 64, 78, 77 }
    };

    /// #vertex
    static constexpr uint16_t VERTEX_COUNT   = ARRAY_LENGTH(VERTEX_LIST);
    /// #triangle
    static constexpr uint16_t TRIANGLE_COUNT = ARRAY_LENGTH(TRIANGLE_LIST);

    cache_mesh(
        MESH_ID, MESH_BOUND_BOX,
        VertexLayoutT::LAYOUT_DECL, VERTEX_COUNT, TRIANGLE_COUNT, false,
        (const uint8_t*)VERTEX_LIST, (uint32_t)sizeof(VERTEX_LIST),
        TRIANGLE_LIST, (uint32_t)sizeof(TRIANGLE_LIST));
    return MESH_ID;
}


RenderMeshIdT
RenderMeshDepot::create_from_mesh_file (
    const char * const  abs_file_name,
    const RenderMeshIdT exp_mesh_id)
{
    /// 无此Mesh
    if (m_mesh_table.find(exp_mesh_id) == m_mesh_table.end())
    {
        AABB bound_box;
        uint16_t vertex_layout;
        uint32_t vertex_count;
        uint32_t triangle_count;
        uint8_t * vertex_list;
        uint32_t vertex_list_size;
        IndexedTriangle * triangle_list;
        uint32_t triangle_list_size;
        const bool opcode = RMeshFile::read_from(
            abs_file_name, exp_mesh_id, bound_box,
            vertex_layout, vertex_count, triangle_count,
            vertex_list, vertex_list_size, triangle_list, triangle_list_size);
        if (opcode == false)
        {
            return INVALID_RENDER_MESH_ID;
        }

        cache_mesh(exp_mesh_id, bound_box, vertex_layout,
                   vertex_count, triangle_count, true,
                   vertex_list, vertex_list_size, triangle_list, triangle_list_size);
    }

    return exp_mesh_id;
}


void
RenderMeshDepot::write_to_ply_file (
    const char * const  abs_file_name,
    const RenderMeshIdT mesh_id) const
{
    const ConstRenderMeshIteratorT render_mesh = m_mesh_table.find(mesh_id);
    if (render_mesh != m_mesh_table.end())
    {
        PlyFile::write_to(abs_file_name, render_mesh->second);
    }
}


void
RenderMeshDepot::write_to_mesh_file (
    const char * const  abs_file_name,
    const RenderMeshIdT mesh_id) const
{
    const ConstRenderMeshIteratorT render_mesh = m_mesh_table.find(mesh_id);
    if (render_mesh != m_mesh_table.end())
    {
        RMeshFile::write_to(abs_file_name, render_mesh->second);
    }
}


void
RenderMeshDepot::clear ()
{
    for (RenderMeshIteratorT mesh = m_mesh_table.begin();
         mesh != m_mesh_table.end(); ++mesh)
    {
        if (mesh->second.is_dyn_allocated)
        {
            MemoryUtility::aligned_free((void*)mesh->second.vertex_list);
            std::free((void*)mesh->second.triangle_list);
        }
    }
    m_mesh_table.clear();
}


RenderMeshDepot::RenderMeshDepot ()
{

}


RenderMeshDepot::~RenderMeshDepot ()
{
    clear();
}


void
RenderMeshDepot::cache_mesh (
    const RenderMeshIdT           mesh_id,
    const AABB &                  bound_box,
    const uint16_t                vertex_layout,
    const uint32_t                vertex_count,
    const uint32_t                triangle_count,
    const bool                    is_dyn_allocated,
    const uint8_t * const         vertex_list,
    const uint32_t                vertex_list_size,
    const IndexedTriangle * const triangle_list,
    const uint32_t                triangle_list_size)
{
    /// 无此Mesh
    if (m_mesh_table.find(mesh_id) == m_mesh_table.end())
    {
        m_mesh_table.insert(
        {
            mesh_id,
            RenderMesh(mesh_id, bound_box, vertex_layout,
                       vertex_count, triangle_count, is_dyn_allocated,
                       vertex_list, vertex_list_size,
                       triangle_list, triangle_list_size)
        });
    }
}
