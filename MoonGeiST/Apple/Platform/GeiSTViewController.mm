/// System headers
#import  <MetalKit/MetalKit.h>
/// Library headers
#include "Common/PlatformDefines.hpp" /// OS_TYPE
#import  "RenderDevice/GeiSTRenderLoop.hpp"
/// Self header
#import  "Platform/GeiSTViewController.hpp"


@interface GeiSTViewController ()
{
    // --- Private成员变量 --- //
    MTKView *         m_mtl_view;
    GeiSTRenderLoop * m_render_loop;
}


// --- Private属性 --- //

// --- Private函数 --- //

@end



@implementation GeiSTViewController


- (void) viewDidLoad
{
    [super viewDidLoad];

    m_mtl_view = (MTKView *)self.view;

#if (OS_TYPE == OS_TYPE_IOS)
    /// 禁止Multiple Touch(即: 只回报第一个Touch Event)
    m_mtl_view.multipleTouchEnabled = NO;
#endif

    m_mtl_view.clearColor = MTLClearColorMake(0.16, 0.165, 0.184, 1);

    m_render_loop = [[GeiSTRenderLoop alloc] initWithMTKView:m_mtl_view];
    if (m_render_loop)
    {
        const CGSize drawable_size = m_mtl_view.drawableSize;
        [m_render_loop mtkView:m_mtl_view drawableSizeWillChange:drawable_size];
        m_mtl_view.delegate = m_render_loop;
    }
    else
    {
        NSLog(@"Can not create SoULRender");
#if (OS_TYPE == OS_TYPE_IOS) /// iOS
        self.view = [[UIView alloc] initWithFrame:self.view.frame];
#elif (OS_TYPE == OS_TYPE_MAC) /// macOS
        self.view = [[NSView alloc] initWithFrame:self.view.frame];
#endif
        return;
    }
}


@end
