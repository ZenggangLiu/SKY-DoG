/// System headers
#include <cstdio>   /// std::snprintf
#include <stdint.h> /// uint32_t
/// Library headers
#import  "Platform/GeiSTAppDelegate+Private.hpp"
#include "FileSystem/NativeDirectory.hpp"
#include "FileSystem/NativeFile.hpp"
#include "Render/SoULRender.hpp"
/// Self header
#import  "Platform/GeiSTAppDelegate.hpp"


static RenderMeshIdT    s_beveled_cube_mesh_id     = RenderMeshIdT();
static RenderTextureIdT s_checker_board_texture_id = RenderTextureIdT();
static bool             s_render_resources_loaded  = false;


static
RenderTextureIdT
create_checker_board ()
{
    static constexpr uint32_t TEXTURE_WIDTH  = 512;
    static constexpr uint32_t TEXTURE_HEIGHT = 521;
    static constexpr uint32_t CELL_DIMENSION = 256;
    static constexpr uint32_t BORDER_WIDTH   = 12;

    return
    SoULRender::create_checker_board(
            TEXTURE_WIDTH,
            TEXTURE_HEIGHT,
            CELL_DIMENSION,
            BORDER_WIDTH,
            LdrColor::make( 87, 103, 134, 255),
            LdrColor::make( 87, 103, 134, 255),
            LdrColor::make(255, 255, 255, 255));
}


/// Class Extension: 作为Private接口使用
@interface GeiSTAppDelegate ()
{
    // --- Private成员变量 --- //
}


// --- Private属性 --- //

// --- Private函数 --- //
- (uint32_t) resourcePathIntoBuffer:(char*)outputBuffer
                           withSize:(uint32_t)bufferSize;

- (NSString *) beveledCubeFileName;

- (NSString *) checkerBoardFileName;
@end



@implementation GeiSTAppDelegate


- (uint32_t) resourcePathIntoBuffer:(char*)outputBuffer
                           withSize:(uint32_t)bufferSize
{
    if (outputBuffer && bufferSize)
    {
        outputBuffer[0] = '\0';

        /// "/var/mobile/Containers/Data/Application/56960AA6-XYZ/Library/Caches"
        const char * const cache_folder = NativeDirectory::cache_folder();
        if (cache_folder && *cache_folder)
        {
            /// "com.RedSoUL.SKYDog"
            NSString * const bundle_id = [[NSBundle mainBundle] bundleIdentifier];
            const char * bundle_id_string = bundle_id.UTF8String;
            if (bundle_id_string == nullptr || *bundle_id_string == '\0')
            {
                bundle_id_string = "MoonGeiST";
            }

            const int32_t path_byte_count = std::snprintf(
                outputBuffer, bufferSize,
                "%s/%s/Render", cache_folder, bundle_id_string);
            if (path_byte_count > 0 &&
                (uint32_t)path_byte_count < bufferSize)
            {
                if (NativeDirectory::does_folder_exist(outputBuffer) ||
                    NativeDirectory::create_folder(outputBuffer))
                {
                    return (uint32_t)path_byte_count;
                }
                else
                {
                    NSLog(@"Can not create Render resource directory: %s",
                          outputBuffer);
                    outputBuffer[0] = '\0';
                    return 0;
                }
            }
            else
            {
                outputBuffer[0] = '\0';
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}


- (NSString *) beveledCubeFileName
{
    char file_name[1024];
    const uint32_t resource_path_size =
        [self resourcePathIntoBuffer:file_name
                           withSize:(uint32_t)sizeof(file_name)];
    if (resource_path_size)
    {
        const uint32_t remaining_buffer_size =
            (uint32_t)sizeof(file_name) - resource_path_size;
        const int32_t appended_byte_count = std::snprintf(
            file_name + resource_path_size, remaining_buffer_size,
            "/UnitBeveledCube.rmesh");
        if (appended_byte_count > 0 &&
            (uint32_t)appended_byte_count < remaining_buffer_size)
        {
            return [NSString stringWithUTF8String:file_name];
        }
    }
    return nil;
}


- (NSString *) checkerBoardFileName
{
    char file_name[1024];
    const uint32_t resource_path_size =
        [self resourcePathIntoBuffer:file_name
                           withSize:(uint32_t)sizeof(file_name)];
    if (resource_path_size)
    {
        const uint32_t remaining_buffer_size =
            (uint32_t)sizeof(file_name) - resource_path_size;
        const int32_t appended_byte_count = std::snprintf(
            file_name + resource_path_size, remaining_buffer_size,
            "/CheckerBoard.rtexture");
        if (appended_byte_count > 0 &&
            (uint32_t)appended_byte_count < remaining_buffer_size)
        {
            return [NSString stringWithUTF8String:file_name];
        }
    }
    return nil;
}


- (void) loadRenderResources
{
    if (s_render_resources_loaded)
    {
        return;
    }

    NSString * const mesh_file_name    = [self beveledCubeFileName];
    NSString * const texture_file_name = [self checkerBoardFileName];

    const bool has_mesh_file = mesh_file_name &&
        NativeFile::does_file_exist(mesh_file_name.fileSystemRepresentation);
    const bool has_texture_file = texture_file_name &&
        NativeFile::does_file_exist(texture_file_name.fileSystemRepresentation);

    /// 第一次运行时先创建资源，以获得由Depot生成的稳定Id
    if (s_beveled_cube_mesh_id.is_not_valid())
    {
        s_beveled_cube_mesh_id = SoULRender::create_unit_beveled_cube();
    }
    if (s_checker_board_texture_id.is_not_valid())
    {
        s_checker_board_texture_id = create_checker_board();
    }

    /// 有缓存文件时，从序列化资源恢复Depot
    if (has_mesh_file || has_texture_file)
    {
        SoULRender::unload_resources();

        if (has_mesh_file)
        {
            const RenderMeshIdT loaded_mesh_id = SoULRender::load_mesh(
                mesh_file_name.UTF8String, s_beveled_cube_mesh_id);
            if (loaded_mesh_id.is_not_valid())
            {
                s_beveled_cube_mesh_id = SoULRender::create_unit_beveled_cube();
            }
        }
        else
        {
            s_beveled_cube_mesh_id = SoULRender::create_unit_beveled_cube();
        }

        if (has_texture_file)
        {
            const RenderTextureIdT loaded_texture_id = SoULRender::load_texture(
                texture_file_name.UTF8String, s_checker_board_texture_id);
            if (loaded_texture_id.is_not_valid())
            {
                s_checker_board_texture_id = create_checker_board();
            }
        }
        else
        {
            s_checker_board_texture_id = create_checker_board();
        }
    }
    else
    {
        s_beveled_cube_mesh_id     = SoULRender::create_unit_beveled_cube();
        s_checker_board_texture_id = create_checker_board();
    }

    s_render_resources_loaded =
        s_beveled_cube_mesh_id.is_valid() &&
        s_checker_board_texture_id.is_valid();
}


- (void) saveRenderResources
{
    if (s_render_resources_loaded)
    {
        NSString * const mesh_file_name    = [self beveledCubeFileName];
        NSString * const texture_file_name = [self checkerBoardFileName];
        if (mesh_file_name && texture_file_name)
        {
            if (SoULRender::save_mesh(
                    mesh_file_name.UTF8String, s_beveled_cube_mesh_id) == false)
            {
                NSLog(@"Can not save BeveledCube RenderMesh");
            }

            if (SoULRender::save_texture(
                    texture_file_name.UTF8String, s_checker_board_texture_id) == false)
            {
                NSLog(@"Can not save CheckerBoard RenderTexture");
            }
        }
    }
}


- (void) unloadRenderResources
{
    if (s_render_resources_loaded)
    {
        SoULRender::unload_resources();
        s_render_resources_loaded = false;
    }
}


@end
