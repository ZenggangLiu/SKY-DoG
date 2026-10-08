/// System headers
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
/// Library headers
#import "RenderDevice/GeiSTRenderLoop.hpp"
/// Self header
#import "Platform/GeiSTViewController.hpp"


@interface GeiSTViewController ()
{
    /// Private成员变量
    MTKView *_view;

    Renderer *_renderer;
}


/// Private属性

/// Private函数

@end



@implementation GeiSTViewController


- (void) viewDidLoad
{
    [super viewDidLoad];

    _view = (MTKView *)self.view;

    /// 禁止Multiple Touch(即: 只回报第一个Touch Event)
    ///_view.multipleTouchEnabled = NO;

    _view.device = MTLCreateSystemDefaultDevice();

    _view.clearColor = MTLClearColorMake(0.16, 0.165, 0.184, 1);

    if(_view.device)
    {
        _renderer = [[Renderer alloc] initWithMetalKitView:_view];
        [_renderer mtkView:_view drawableSizeWillChange:_view.bounds.size];
        _view.delegate = _renderer;
    }
    else
    {
        NSLog(@"Metal is not supported on this device");
#if (OS_TYPE == OS_TYPE_IOS) /// iOS
        self.view = [[UIView alloc] initWithFrame:self.view.frame];
#elif (OS_TYPE == OS_TYPE_MAC) /// macOS
        self.view = [[NSView alloc] initWithFrame:self.view.frame];
#endif
        return;
    }
}


@end
