/// Self header
#import "Platform/GeiSTAppDelegate+iOS.hpp"


#if (OS_TYPE == OS_TYPE_IOS)
@interface GeiSTAppDelegate ()
{
    /// Private成员变量
}


/// Private属性

/// Private函数
@end



@implementation GeiSTAppDelegate (iOS)


/// 程序启动完成
/// 执行任务:
/// - RedSoUL初始化
/// - 加载Assets
/// X 创建RenderLoop, 放在ViewController::viewDidLoad中处理
- (BOOL) application: (UIApplication *)application didFinishLaunchingWithOptions: (NSDictionary *)launchOptions
{
    NSLog(@"-> App has Launched");
    NSLog(@"%@", self.window);
    return YES;
}


/// 进入前台
/// 执行任务: 与“Hide程序”的任务相反
/// - 加载游戏进度
/// - 启动/恢复Audio，Physics
/// - 启动/恢复GameLoop
/// - 恢复RenderLoop
- (void) applicationDidBecomeActive: (UIApplication *)application
{
    NSLog(@"-> App UNHIDDEN");
}


/// 进入后台
/// 执行任务:
/// - 暂停RenderLoop
/// - 暂停GameLoop
/// - 暂停Audio，Physics
/// - 保存游戏进度
- (void) applicationDidEnterBackground: (UIApplication *)application
{
    NSLog(@"-> App HIDDEN");
}


/// 程序关闭
/// CLEAN-UP
- (void) applicationWillTerminate: (UIApplication *)application
{
    NSLog(@"-> App terminating...");
}


@end
#endif /// (OS_TYPE == OS_TYPE_IOS)
