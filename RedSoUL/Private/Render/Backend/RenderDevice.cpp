/// System headers
#include <limits> /// std::numeric_limits
#include <unordered_map>
#include <vector>
/// Library headers
#include "DataType/Float2D.hpp"
#include "DataType/Float3D.hpp"
#include "DataType/Matrix4x4.hpp"
#include "Render/Backend/DeviceBufferDesc.hpp"
#include "Render/Backend/DevicePipelineDesc.hpp"
#include "Render/Backend/DeviceTextureDesc.hpp"
#include "Render/Backend/DeviceTextureFormat.hpp"
#include "Render/Backend/DeviceTextureUtilities.hpp"
#include "Render/IndexedTriangle.hpp"
#include "Render/RenderMesh.hpp"
#include "Render/RenderMeshDepot.hpp"
#include "Render/RenderTexture.hpp"
#include "Render/RenderTextureDepot.hpp"
#include "Render/TextureDataType.hpp"
#include "Render/TextureUtilities.hpp"
#include "Render/VertexDataType.hpp"
/// Self header
#include "Render/Backend/RenderDevice.hpp"
#if defined(__APPLE__)
#include "Render/Backend/Metal/MetalRenderDevice.hpp"
#elif (OS_TYPE == OS_TYPE_WIN)
//#include "Render/DirectX/Direct3DRenderDevice.hpp"
#else
//#include "Render/Vulkan/VulkanRenderDevice.hpp"
#endif


static constexpr uint16_t INVALID_VERTEX_ATTRIBUTE_OFFSET = (uint16_t)(-1);


/// MARK: == Helpers ==
static
uint16_t
vertex_attribute_offset (
    const uint16_t vertex_layout,
    const uint16_t exp_vertex_data_type)
{
    uint16_t offset = 0;

    static constexpr uint16_t VERTEX_DATA_TYPE_LIST[] =
    {
        VertexDataType::POSITION_DATA,
        VertexDataType::COLOR_DATA,
        VertexDataType::NORMAL_DATA,
        VertexDataType::TEXCOORD_DATA,
    };
    static constexpr uint16_t VERTEX_DATA_SIZE_LIST[] =
    {
        sizeof(float_3),
        sizeof(float_3),
        sizeof(float_3),
        sizeof(float_2),
    };

    for (uint8_t i = 0; i < 4; ++i)
    {
        const uint16_t vertex_data_type = VERTEX_DATA_TYPE_LIST[i];
        if (vertex_layout & vertex_data_type)
        {
            if (vertex_data_type == exp_vertex_data_type)
            {
                return offset;
            }
            offset += VERTEX_DATA_SIZE_LIST[i];
        }
    }
    return INVALID_VERTEX_ATTRIBUTE_OFFSET;
}


struct DeviceMeshResource
{
    RenderMeshIdT     host_mesh_id;
    DeviceBufferIdT   vertex_buffer_id;
    DeviceBufferIdT   index_buffer_id;
    DevicePipelineIdT pipeline_id;
    uint32_t          index_count;
};


struct DeviceTextureResource
{
    RenderTextureIdT  host_texture_id;
    DeviceTextureIdT texture_id;
};


struct CachedDrawItem
{
    DeviceMeshIdT    mesh_id;
    DeviceTextureIdT texture_id;
    matrix_4x4       model_view_transform;
    matrix_4x4       projection_transform;
};



/// MARK: == State ==
///
/// - Host资源到Device资源的缓存
/// - 当前Frame尚未提交的Draw Item
///
template < typename DetailedRenderDevice >
struct RenderDeviceBase<DetailedRenderDevice>::State
{
    State ()
    :
        next_mesh_id(0)
    {

    }

    /// NOTE: Id只在当前RenderDevice内有效
    /// Next可分配的DeviceMesh的Id
    DeviceMeshIdT next_mesh_id;

    /// DeviceMeshId -> Device Mesh资源，保存已创建的Buffer、Pipeline及绘制信息。
    std::unordered_map< DeviceMeshIdT,
                        DeviceMeshResource,
                        DeviceMeshIdT::StlHasher,
                        DeviceMeshIdT::StlComparer > mesh_table;

    /// RenderMeshId -> DeviceMeshId，用于复用已缓存的Device Mesh。
    std::unordered_map< RenderMeshIdT,
                        DeviceMeshIdT,
                        RenderMeshIdT::StlHasher,
                        RenderMeshIdT::StlComparer > host_mesh_table;

    /// DeviceTextureId -> Device Texture资源。
    std::unordered_map< DeviceTextureIdT,
                        DeviceTextureResource,
                        DeviceTextureIdT::StlHasher,
                        DeviceTextureIdT::StlComparer > texture_table;

    /// RenderTextureId -> DeviceTextureId，用于复用已缓存的Device Texture。
    std::unordered_map< RenderTextureIdT,
                        DeviceTextureIdT,
                        RenderTextureIdT::StlHasher,
                        RenderTextureIdT::StlComparer > host_texture_table;

    /// 当前Frame的待绘制列表，draw()提交后清空。
    std::vector<CachedDrawItem> frame_draw_list;
};



/// MARK: == RenderDeviceBase ==
template < typename DetailedRenderDevice >
bool
RenderDeviceBase<DetailedRenderDevice>::prepare_draw (
    const RenderMeshIdT    host_mesh_id,
    const RenderTextureIdT host_texture_id,
    const matrix_4x4 &     model_view_transform,
    const matrix_4x4 &     projection_transform)
{
    const DeviceMeshIdT device_mesh_id = cache_mesh(host_mesh_id);
    const DeviceTextureIdT device_texture_id = cache_texture(host_texture_id);
    if (device_mesh_id.is_valid() && device_texture_id.is_valid())
    {
        m_state->frame_draw_list.push_back(
        {
            device_mesh_id,
            device_texture_id,
            model_view_transform,
            projection_transform,
        });
        return true;
    }
    else
    {
        return false;
    }
}


template < typename DetailedRenderDevice >
bool
RenderDeviceBase<DetailedRenderDevice>::draw (
    void * const native_surface)
{
    if (m_state != nullptr && detailed_device().begin_frame(native_surface))
    {
        bool draw_succeeded = true;
        for (const CachedDrawItem & draw_item : m_state->frame_draw_list)
        {
            const auto mesh_item = m_state->mesh_table.find(draw_item.mesh_id);
            const auto texture_item =
                m_state->texture_table.find(draw_item.texture_id);
            if (mesh_item != m_state->mesh_table.end() &&
                texture_item != m_state->texture_table.end())
            {
                const DeviceMeshResource & mesh = mesh_item->second;
                const DeviceDrawCall draw_call
                {
                    mesh.pipeline_id,
                    mesh.vertex_buffer_id,
                    mesh.index_buffer_id,
                    texture_item->second.texture_id,
                    mesh.index_count,
                    draw_item.model_view_transform,
                    draw_item.projection_transform,
                };
                draw_succeeded =
                   detailed_device().draw_indexed(draw_call) && draw_succeeded;
            }
            else
            {
                draw_succeeded = false;
                continue;
            }
        }

        clear_frame();
        return detailed_device().end_frame() && draw_succeeded;
    }
    else
    {
        clear_frame();
        return false;
    }
}


template < typename DetailedRenderDevice >
void
RenderDeviceBase<DetailedRenderDevice>::clear_frame ()
{
    if (m_state)
    {
        m_state->frame_draw_list.clear();
    }
}


template < typename DetailedRenderDevice >
void
RenderDeviceBase<DetailedRenderDevice>::release_resources ()
{
    if (m_state != nullptr)
    {
        for (const auto & mesh_item : m_state->mesh_table)
        {
            const DeviceMeshResource & mesh = mesh_item.second;
            detailed_device().destroy_pipeline(mesh.pipeline_id);
            detailed_device().destroy_buffer(mesh.index_buffer_id);
            detailed_device().destroy_buffer(mesh.vertex_buffer_id);
        }
        for (const auto & texture_item : m_state->texture_table)
        {
            detailed_device().destroy_texture(texture_item.second.texture_id);
        }

        m_state->mesh_table.clear();
        m_state->host_mesh_table.clear();
        m_state->texture_table.clear();
        m_state->host_texture_table.clear();
        m_state->frame_draw_list.clear();
        m_state->next_mesh_id.reset();
    }
    else
    {
        return;
    }
}


template < typename DetailedRenderDevice >
RenderDeviceBase<DetailedRenderDevice>::RenderDeviceBase ()
:
    m_state(new State())
{

}


template < typename DetailedRenderDevice >
RenderDeviceBase<DetailedRenderDevice>::~RenderDeviceBase ()
{
    delete m_state;
    m_state = nullptr;
}


template < typename DetailedRenderDevice >
DetailedRenderDevice &
RenderDeviceBase<DetailedRenderDevice>::detailed_device ()
{
    return static_cast<DetailedRenderDevice &>(*this);
}


template < typename DetailedRenderDevice >
DeviceMeshIdT
RenderDeviceBase<DetailedRenderDevice>::cache_mesh (
    const RenderMeshIdT host_mesh_id)
{
    DeviceMeshIdT device_mesh_id{}; /// 开始为非法Id

    if (host_mesh_id.is_valid() && m_state && m_state->next_mesh_id.is_valid())
    {
        const auto cached_mesh = m_state->host_mesh_table.find(host_mesh_id);
        /// 新RenderMesh
        if (cached_mesh == m_state->host_mesh_table.end())
        {
            const RenderMesh * const host_mesh =
                RenderMeshDepot::ref().mesh_data(host_mesh_id);
            if (host_mesh && host_mesh->vertex_count &&
                host_mesh->vertex_list_size % host_mesh->vertex_count == 0 &&
                host_mesh->triangle_list_size ==
                    host_mesh->triangle_count * sizeof(IndexedTriangle))
            {
                static_assert(
                    sizeof(IndexedTriangle) == sizeof(uint16_t) * 3,
                    "IndexedTriangle must contain three packed uint16_t indices!!");

                const uint32_t vertex_stride =
                    host_mesh->vertex_list_size / host_mesh->vertex_count;
                if (vertex_stride <= std::numeric_limits<uint16_t>::max())
                {
                    const uint16_t position_offset = vertex_attribute_offset(
                        host_mesh->vertex_layout, VertexDataType::POSITION_DATA);
                    const uint16_t texcoord_offset = vertex_attribute_offset(
                        host_mesh->vertex_layout, VertexDataType::TEXCOORD_DATA);
                    if (position_offset != INVALID_VERTEX_ATTRIBUTE_OFFSET &&
                        texcoord_offset != INVALID_VERTEX_ATTRIBUTE_OFFSET)
                    {
                        const DeviceBufferDesc vertex_buffer_desc
                        {
                            host_mesh->vertex_list,
                            host_mesh->vertex_list_size,
                            DeviceBufferUsage::VERTEX_BUFFER,
                        };

                        const DeviceBufferIdT vertex_buffer_id =
                            detailed_device().create_buffer(vertex_buffer_desc);
                        if (vertex_buffer_id.is_valid())
                        {
                            const DeviceBufferDesc index_buffer_desc
                            {
                                host_mesh->triangle_list,
                                host_mesh->triangle_list_size,
                                DeviceBufferUsage::INDEX_BUFFER,
                            };

                            const DeviceBufferIdT index_buffer_id =
                                detailed_device().create_buffer(index_buffer_desc);
                            if (index_buffer_id.is_valid())
                            {
                                const DevicePipelineDesc pipeline_desc
                                {
                                    "soul_vertex_shader",
                                    "soul_fragment_shader",
                                    (uint16_t)vertex_stride,
                                    position_offset,
                                    texcoord_offset,
                                };

                                const DevicePipelineIdT pipeline_id =
                                    detailed_device().create_pipeline(pipeline_desc);
                                if (pipeline_id.is_valid())
                                {
                                    device_mesh_id = m_state->next_mesh_id;
                                    m_state->next_mesh_id.next_id();
                                    m_state->mesh_table.insert(
                                    {
                                        device_mesh_id,
                                        {
                                            host_mesh_id,
                                            vertex_buffer_id,
                                            index_buffer_id,
                                            pipeline_id,
                                            host_mesh->triangle_count * 3,
                                        }
                                    });
                                    m_state->host_mesh_table.insert(
                                        { host_mesh_id, device_mesh_id });
                                }
                                else
                                {
                                    detailed_device().destroy_buffer(index_buffer_id);
                                    detailed_device().destroy_buffer(vertex_buffer_id);
                                }
                            }
                            else
                            {
                                detailed_device().destroy_buffer(vertex_buffer_id);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            device_mesh_id = cached_mesh->second;
        }
    }

    return device_mesh_id;
}


template < typename DetailedRenderDevice >
DeviceTextureIdT
RenderDeviceBase<DetailedRenderDevice>::cache_texture (
    const RenderTextureIdT host_texture_id)
{
    DeviceTextureIdT device_texture_id{};

    if (m_state && host_texture_id.is_valid())
    {
        const auto cached_texture =
            m_state->host_texture_table.find(host_texture_id);
        if (cached_texture == m_state->host_texture_table.end())
        {
            const RenderTexture * const host_texture =
                RenderTextureDepot::ref().texture_data(host_texture_id);
            if (host_texture && host_texture->texel_list &&
                host_texture->texture_width && host_texture->texture_height &&
                host_texture->mipmap_count)
            {
                const uint8_t slice_count =
                    RenderTextureDepot::ref().total_slice_count(
                        host_texture->is_cube_map);
                if (slice_count &&
                    (host_texture->is_cube_map == false ||
                     host_texture->texture_width == host_texture->texture_height))
                {
                    const uint8_t max_mipmap_count =
                        TextureUtility::maximal_mipmap_count(
                            host_texture->texture_width,
                            host_texture->texture_height);
                    if (host_texture->mipmap_count <= max_mipmap_count)
                    {
                        const DeviceTextureFormat texture_format =
                            DeviceTextureUtility::to_device_texture_format(
                                host_texture->texture_data_type,
                                host_texture->is_linear_rgb);
                        const uint32_t texel_size_in_bytes =
                            DeviceTextureUtility::texel_size_in_bytes(texture_format);
                        bool is_texture_data_size_valid = true;
                        uint64_t required_slice_data_size = 0;
                        uint32_t mipmap_width  = host_texture->texture_width;
                        uint32_t mipmap_height = host_texture->texture_height;
                        for (uint8_t mipmap_idx = 0;
                             mipmap_idx < host_texture->mipmap_count &&
                             is_texture_data_size_valid;
                             ++mipmap_idx)
                        {
                            const uint64_t mipmap_texel_count =
                                (uint64_t)mipmap_width * mipmap_height;
                            if (mipmap_texel_count <=
                                (std::numeric_limits<uint32_t>::max() /
                                 texel_size_in_bytes))
                            {
                                const uint64_t mipmap_data_size =
                                    mipmap_texel_count * texel_size_in_bytes;
                                if (required_slice_data_size <=
                                    std::numeric_limits<uint32_t>::max() -
                                        mipmap_data_size)
                                {
                                    required_slice_data_size += mipmap_data_size;

                                    mipmap_width =
                                        TextureUtility::next_mipmap_dimension(
                                            mipmap_width);
                                    mipmap_height=
                                        TextureUtility::next_mipmap_dimension(
                                            mipmap_height);
                                }
                                else
                                {
                                    is_texture_data_size_valid = false;
                                }
                            }
                            else
                            {
                                is_texture_data_size_valid = false;
                            }
                        }

                        if (is_texture_data_size_valid &&
                            required_slice_data_size ==
                                host_texture->slice_data_size &&
                            required_slice_data_size <=
                                std::numeric_limits<uint32_t>::max() /
                                    slice_count &&
                            required_slice_data_size * slice_count ==
                                host_texture->texel_list_size)
                        {
                            const DeviceTextureDesc texture_desc
                            {
                                host_texture->texel_list,
                                host_texture->texel_list_size,
                                host_texture->texture_width,
                                host_texture->texture_height,
                                host_texture->mipmap_count,
                                slice_count,
                                host_texture->is_cube_map,
                                texture_format,
                            };
                            device_texture_id =
                                detailed_device().create_texture(texture_desc);
                            if (device_texture_id.is_valid())
                            {
                                m_state->texture_table.insert(
                                {
                                    device_texture_id,
                                    { host_texture_id, device_texture_id }
                                });
                                m_state->host_texture_table.insert(
                                    { host_texture_id, device_texture_id });
                            }
                        }
                    }
                }
            }
        }
        else
        {
            device_texture_id = cached_texture->second;
        }
    }

    return device_texture_id;
}



/// MARK: == 显性实例化 ===
#if defined(__APPLE__)
template class RenderDeviceBase<MetalRenderDevice>;
#elif (OS_TYPE == OS_TYPE_WIN)
//template class RenderDeviceBase<Direct3DRenderDevice>;
#else
//template class RenderDeviceBase<VulkanRenderDevice>;
#endif
