/// System headers
#include <cstdint> /// uintptr_t
/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Common/CommonDefines.hpp" /// ALIGN_UP
/// Self header
#include "Render/RenderMesh.hpp"


RenderMesh::RenderMesh (
    const RenderMeshIdT           _mesh_id,
    const AABB &                  _bound_box,
    const uint16_t                _vertex_layout,
    const uint32_t                _vertex_count,
    const uint32_t                _triangle_count,
    const bool                    _is_dyn_allocated,
    const uint8_t * const         _vertex_list,
    const uint32_t                _vertex_list_size,
    const IndexedTriangle * const _triangle_list,
    const uint32_t                _triangle_list_size)
:
    vertex_list(_vertex_list),
    triangle_list(_triangle_list),
    mesh_id(_mesh_id),
    bound_box(_bound_box),
    vertex_list_size(_vertex_list_size),
    triangle_list_size(_triangle_list_size),
    vertex_count(_vertex_count),
    triangle_count(_triangle_count),
    vertex_layout(_vertex_layout),
    is_dyn_allocated(_is_dyn_allocated)
{
    RUNTIME_ASSERT(_mesh_id.is_valid(), "Mesh Id must be valid!!");
    RUNTIME_ASSERT(_vertex_count, "Zero vertex count!!");
    RUNTIME_ASSERT(_triangle_count, "Zero triangle count!!");
    RUNTIME_ASSERT(_vertex_list, "No vertex list!!");
    RUNTIME_ASSERT(_vertex_list_size, "Zero vertex list size!!");
    RUNTIME_ASSERT(_triangle_list, "No triangle index list!!");
    RUNTIME_ASSERT(_triangle_list_size, "Zero triangle index list size!!");
    RUNTIME_ASSERT(
        (uintptr_t)_vertex_list == ALIGN_UP((uintptr_t)_vertex_list, 16),
        "Wrong alignment(16 bytes aligment required)!!");
}
