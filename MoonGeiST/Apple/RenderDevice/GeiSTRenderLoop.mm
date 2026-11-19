/// Library headers
#include "DataType/Float3D.hpp"
#include "DataType/Matrix3x4.hpp"
#include "DataType/Matrix4x4.hpp"
#include "DataType/Quaternion.hpp"
#include "Math/MathDefines.hpp"   /// DEGREE_TO_RADIAN
#include "Render/Backend/RenderDevice.hpp"
#include "Render/Backend/RenderDeviceDesc.hpp"
#include "Render/Backend/RenderDeviceFactory.hpp"
#include "Render/SoULRender.hpp"
/// Self header
#import "RenderDevice/GeiSTRenderLoop.hpp"


@interface GeiSTRenderLoop ()
{
    RenderDevice *   m_render_device;
    RenderMeshIdT    m_host_mesh_id;
    RenderTextureIdT m_host_texture_id;
    matrix_4x4       m_projection_transform;
    float            m_rotation;
}


- (BOOL) load_game_assets;

@end


@implementation GeiSTRenderLoop


- (nullable instancetype) initWithMTKView: (nonnull MTKView *)view
{
    self = [super init];
    if (self)
    {
        const RenderDeviceDesc render_device_desc
        {
            RenderDeviceType::METAL_RENDER_DEVICE,
            (__bridge void *)view,
        };
        m_render_device = RenderDeviceFactory::create(render_device_desc);
        m_host_mesh_id = RenderMeshIdT();
        m_host_texture_id = RenderTextureIdT();
        m_projection_transform = matrix_4x4::IDENTITY;
        m_rotation = 0;

        if (m_render_device == nullptr || [self load_game_assets] == NO)
        {
            RenderDeviceFactory::destroy(m_render_device);
            m_render_device = nullptr;
            return nil;
        }
    }
    return self;
}


- (void) dealloc
{
    RenderDeviceFactory::destroy(m_render_device);
    m_render_device = nullptr;
}


- (BOOL) load_game_assets
{
    m_host_mesh_id = SoULRender::create_unit_beveled_cube();
    if (m_host_mesh_id.is_not_valid())
    {
        NSLog(@"Can not create BeveledCube Device Mesh");
        return NO;
    }

    m_host_texture_id = SoULRender::create_checker_board(
        512, 521, 256, 12,
        LdrColor::make( 87, 103, 134, 255),
        LdrColor::make( 87, 103, 134, 255),
        LdrColor::make(255, 255, 255, 255));
    if (m_host_texture_id.is_not_valid())
    {
        NSLog(@"Can not create CheckerBoard Device Texture");
        return NO;
    }
    return YES;
}


- (void) drawInMTKView: (nonnull MTKView *)view
{
    const float_3    rot_axis = float_3::make(1.0f, 1.0f, 0.0f);
    const quaternion rot_quat = quaternion::make(rot_axis, m_rotation);
    const matrix_3x4 model_transform = matrix_3x4::make_rotation(rot_quat);

    const float_3 trans_vec = float_3::make(0.0f, 0.0f, 2.5f);
    const matrix_3x4 view_transform = matrix_3x4::make_translation(trans_vec);

    const matrix_3x4 model_view_transform = view_transform * model_transform;

    m_render_device->prepare_draw(
        m_host_mesh_id,
        m_host_texture_id,
        matrix_4x4::make(model_view_transform),
        m_projection_transform);
    m_render_device->draw((__bridge void *)view);

    /// 逆时针旋转: 从左上到右下旋转
    m_rotation -= 0.01f;
}


- (void) mtkView: (nonnull MTKView *)view
        drawableSizeWillChange: (CGSize)rt_pixel_size
{
    /// MacOS:
    /// - view.window.backingScaleFactor: 2
    /// - bounds: < 800, 600 >, drawableSize: < 1600, 1200 >
    /// iOS(iPhone 7 Plus):
    /// - view.contentScaleFactor: 3
    /// - bounds: < 414, 736 >, drawableSize: < 1242, 2208 >
    ///
    /// const float aspect    = size.width / (float)size.height;
    /// const float fovY_rads = DEGREE_TO_RADIAN(65.0f);
    /// const float fovX_rads = 2.0f * std::atan(std::tan(fovY_rads * 0.5f) * aspect);
    /// const float fovx_degs = RADIAN_TO_DEGREE(fovX_rads)
    static constexpr float FOVX_DEGREES = 80.6909332f;
    static constexpr float FOVX_RADIANS = DEGREE_TO_RADIAN(FOVX_DEGREES);

    if (rt_pixel_size.width > 0 && rt_pixel_size.height > 0)
    {
        const float aspect = rt_pixel_size.width / (float)rt_pixel_size.height;
        m_projection_transform =
            matrix_4x4::make_perspective_projection(FOVX_RADIANS, aspect, 0.1f);
    }
}


@end
