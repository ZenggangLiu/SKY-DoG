/// Library headers
#include "Render/RenderMeshDepot.hpp"
#include "Render/RenderTextureDepot.hpp"
/// Self header
#include "Render/SoULRender.hpp"


RenderMeshIdT
SoULRender::create_unit_beveled_cube ()
{
    return RenderMeshDepot::ref().create_unit_beveled_cube();
}


RenderTextureIdT
SoULRender::create_checker_board (
    const uint32_t texture_width,
    const uint32_t texture_height,
    const uint32_t cell_dimension,
    const uint32_t cell_border_width,
    const LdrColor color_cell_0,
    const LdrColor color_cell_1,
    const LdrColor color_cell_border)
{
    return RenderTextureDepot::ref().create_checker_board(
        texture_width, texture_height,
        cell_dimension, cell_border_width,
        color_cell_0, color_cell_1, color_cell_border);
}


RenderMeshIdT
SoULRender::load_mesh (
    const char * const  abs_file_name,
    const RenderMeshIdT exp_mesh_id)
{
    if (abs_file_name && *abs_file_name && exp_mesh_id.is_valid())
    {
        RenderMeshDepot & mesh_depot = RenderMeshDepot::ref();
        return mesh_depot.create_from_mesh_file(abs_file_name, exp_mesh_id);
    }
    else
    {
        return RenderMeshIdT::invalid_hashed_id();
    }
}


RenderTextureIdT
SoULRender::load_texture (
    const char * const     abs_file_name,
    const RenderTextureIdT exp_texture_id)
{
    if (abs_file_name && *abs_file_name && exp_texture_id.is_valid())
    {
        RenderTextureDepot & texture_depot = RenderTextureDepot::ref();
        return texture_depot.create_from_texture_file(abs_file_name, exp_texture_id);
    }
    else
    {
        return RenderTextureIdT::invalid_hashed_id();
    }
}


bool
SoULRender::save_mesh (
    const char * const  abs_file_name,
    const RenderMeshIdT mesh_id)
{
    if (abs_file_name && *abs_file_name && mesh_id.is_valid())
    {
        RenderMeshDepot & mesh_depot = RenderMeshDepot::ref();
        return mesh_depot.write_to_mesh_file(abs_file_name, mesh_id);
    }
    else
    {
        return false;
    }
}


bool
SoULRender::save_texture (
    const char * const     abs_file_name,
    const RenderTextureIdT texture_id)
{
    if (abs_file_name && *abs_file_name && texture_id.is_valid())
    {
        RenderTextureDepot & texture_depot = RenderTextureDepot::ref();
        return texture_depot.write_to_texture_file(abs_file_name, texture_id);
    }
    else
    {
        return false;
    }
}


void
SoULRender::unload_resources ()
{
    RenderMeshDepot::ref().clear();
    RenderTextureDepot::ref().clear();
}
