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
    Created:  20/08/26  @  10:43 PM
    FileName: RenderTextureDepot.hpp @ RedSoUL Project
    History:
             - created by: 20/08/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint8_t, uint32_t,...
#include <tuple>
#include <unordered_map>
/// Library headers
#include "DataType/LdrColor.hpp"
#include "Render/RenderTexture.hpp"
#include "Render/RenderTextureId.hpp"


/// 渲染贴图仓库
///
class RenderTextureDepot
{
public:
    /// 获取对此Depot的参考
    static
    RenderTextureDepot &
    ref ();

    /// 获取指定尺寸的贴图中每个Slice中Texel的总数
    uint32_t
    slice_texel_count (
        const uint32_t texture_width,
        const uint32_t texture_height,
        const uint8_t  mipmap_count) const;

    /// 获取贴图中的Slice总数
    uint8_t
    total_slice_count (
        const bool is_cube_map) const;

    /// 获取指定尺寸的贴图中Texel的总数
    uint32_t
    total_texel_count (
        const uint32_t texture_width,
        const uint32_t texture_height,
        const uint8_t  mipmap_count,
        const bool     is_cube_map) const;

    /// 获取指定Id对应的RenderTexture数据
    ///
    /// @return
    ///     合法RenderTexture指针, 如果指定Id对应Cached的数据
    ///     nullptr, 如果指定Id非法, 或者指定Id不在Cache中
    const RenderTexture *
    texture_data (
        const RenderTextureIdT texture_id) const;

    /// 获取指定Id对应的RenderTexture中指定Slice, Mipmap的Texel数据列表地址
    ///
    /// @return
    ///     <List的地址, List的大小(字节数)>, 如果成功
    ///     <nullptr, 0>, 如果失败
    std::tuple<const uint8_t *, uint32_t>
    texel_list (
        const RenderTextureIdT texture_id,
        const uint8_t          slice_idx,
        const uint8_t          mipmap_idx) const;

    /// 创建一个Ldr Checker board贴图
    RenderTextureIdT
    create_checker_board (
        const uint32_t texture_width,
        const uint32_t texture_height,
        const uint32_t cell_dimension,
        const uint32_t cell_border_width,
        const LdrColor color_cell_0,
        const LdrColor color_cell_1,
        const LdrColor color_cell_border);

    /// 创建一个256x256大小的Ldr Mipmap调试贴图
    /// Mipmap色彩分布
    /// Mip0 --> Mip6: 赤橙黄绿青蓝紫
    RenderTextureIdT
    create_debug_mipmap ();

    /// 创建一个Hdr Cubemap调试贴图
    ///
    /// NOTE: Face的保存顺序为: +X, -X, +Y, -Y, +Z, -Z
    ///
    ///             ^ Y
    ///          +Z |     / Z                  +--------+
    ///       +-----|-------+                  |        |
    ///      /|     .   /  /|                  |   +Y   |
    ///     / |        .  / |                  |        |
    ///    +-------------+  |         +--------+--------+--------+--------+
    ///    |  |     |/   |  |         |        |        |        |        |
    /// -X |  |     o----|.----> X    |   -X   |   -Z   |   +X   |   +Z   |
    ///    |  |          |  |         |        |        |        |        |
    ///    |  +----------|--+         +--------+--------+--------+--------+
    ///    | /           | /                   |        |
    ///    +-------------+                     |   -Y   |
    ///          -Y                            |        |
    ///                                        +--------+
    ///
    RenderTextureIdT
    create_debug_cubemap (
        const uint32_t texture_size);

    /// 创建指定RenderTexture文件中的贴图
    ///
    /// @param[in]  abs_file_name
    ///     RenderTexture文件的绝对路径
    /// @param[in]  exp_texture_id
    ///     希望使用的Texture Id
    /// @return
    ///     合法Id, 如果加载成功
    ///     非法Id, 如果加载失败
    RenderTextureIdT
    create_from_texture_file (
        const char * const     abs_file_name,
        const RenderTextureIdT exp_texture_id);

    /// 将指定Id的RenderTexture的数据保存在指定的Image文件中
    ///
    /// @param[in]  abs_file_name
    ///     文件绝对路径
    ///     NOTE: 指定文件扩展名将忽略(将根据纹理数据类型生成Ppm或者Exr文件)
    void
    write_to_image_file (
        const char * const     abs_file_name,
        const RenderTextureIdT texture_id) const;

    /// 将指定Id的RenderTexture的数据保存在指定的RenderTexture文件中
    void
    write_to_texture_file (
        const char * const     abs_file_name,
        const RenderTextureIdT texture_id) const;

    /// 清空所有已缓存贴图
    void
    clear ();

private:
     RenderTextureDepot ();
    ~RenderTextureDepot ();

    /// 获取每个Texel的字节大小
    uint8_t
    texel_size_in_bytes (
        const TextureDataType data_type) const;

    /// 获取指定Mipmap的Texel列表数据的相对字节偏移(相对于Slice的起始位置)
    uint32_t
    mipmap_texel_list_offset_in_bytes (
        const RenderTexture * const texture,
        const uint8_t               mipmap_idx) const;

    /// 获取指定Mipmap中Texel数据列表的字节大小
    ///
    uint32_t
    mipmap_texel_list_size_in_bytes (
        const RenderTexture * const texture,
        const uint8_t               mipmap_idx) const;

    /// Cache新的渲染贴图
    ///
    /// @param[in]  mipmap_count
    ///     Mipmap总数。如果没有Mipmap将其设置为1
    /// @param[in]  is_cube_map
    ///     标记纹理是否为CubeMap
    /// @param[in]  is_linear_rgb
    ///     标记纹理数据是否使用Linear RGB空间
    /// @param[in]  is_dyn_allocated
    ///     标记纹理数据列表是否为动态分配(True)
    void
    cache_texture (
        const RenderTextureIdT texture_id,
        const uint32_t         texture_width,
        const uint32_t         texture_height,
        const uint8_t          mipmap_count,
        const bool             is_cube_map,
        const bool             is_linear_rgb,
        const TextureDataType  texture_data_type,
        const uint32_t         slice_data_size,
        const bool             is_dyn_allocated,
        const uint8_t * const  texel_list,
        const uint32_t         texel_list_size);

private:
    typedef std::unordered_map < RenderTextureIdT,
                                 RenderTexture,
                                 RenderTextureIdT::StlHasher,
                                 RenderTextureIdT::StlComparer > RenderTextureTableT;
    typedef RenderTextureTableT::const_iterator ConstRenderTextureIteratorT;
    typedef RenderTextureTableT::iterator       RenderTextureIteratorT;

    RenderTextureTableT m_texture_table;
};
