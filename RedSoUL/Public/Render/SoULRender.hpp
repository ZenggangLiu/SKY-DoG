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
    Created:  2/11/26  @  3:52 PM
    FileName: SoULRender.hpp @ RedSoUL Project
    History:
             - created by: 2/11/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint32_t
/// Library headers
#include "DataType/LdrColor.hpp"
#include "Render/RenderMeshId.hpp"
#include "Render/RenderTextureId.hpp"


/// Render FrontEnd
///
class SoULRender
{
public:
    /// 创建Beveled Cube(带Position, Normal, UV数据)
    static
    RenderMeshIdT
    create_unit_beveled_cube ();

    /// 创建Ldr Checker board贴图
    static
    RenderTextureIdT
    create_checker_board (
        const uint32_t texture_width,
        const uint32_t texture_height,
        const uint32_t cell_dimension,
        const uint32_t cell_border_width,
        const LdrColor color_cell_0,
        const LdrColor color_cell_1,
        const LdrColor color_cell_border);

    /// 从文件加载RenderMesh
    ///
    /// @param[in]  abs_file_name
    ///     RenderMesh文件的绝对路径
    /// @param[in]  exp_mesh_id
    ///     希望使用的Mesh Id
    /// @return
    ///     合法Id, 如果加载成功
    ///     非法Id, 如果加载失败
    static
    RenderMeshIdT
    load_mesh (
        const char * const  abs_file_name,
        const RenderMeshIdT exp_mesh_id);

    /// 从文件加载RenderTexture
    ///
    /// @param[in]  abs_file_name
    ///     RenderTexture文件的绝对路径
    /// @param[in]  exp_texture_id
    ///     希望使用的Texture Id
    /// @return
    ///     合法Id, 如果加载成功
    ///     非法Id, 如果加载失败
    static
    RenderTextureIdT
    load_texture (
        const char * const     abs_file_name,
        const RenderTextureIdT exp_texture_id);

    /// 将RenderMesh保存到文件
    static
    bool
    save_mesh (
        const char * const  abs_file_name,
        const RenderMeshIdT mesh_id);

    /// 将RenderTexture保存到文件
    static
    bool
    save_texture (
        const char * const     abs_file_name,
        const RenderTextureIdT texture_id);

    /// 卸载所有Cached资源(目前只有RenderMesh/RenderTexture)
    static
    void
    unload_resources ();


private:
    SoULRender () = delete;
};
