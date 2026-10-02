/// Self header
#import "Platform/GeiSTAppDelegate+macOS.hpp"


#if (OS_TYPE == OS_TYPE_MAC)
/// Class Extensioin: 作为Private接口使用
@interface GeiSTAppDelegate ()
{
    /// Private成员变量
}


/// Private属性

/// Private函数
@end



@implementation GeiSTAppDelegate (macOS)


/// 确定是否当最后一个Windows关闭时，此App退出
/// @return
///     YES: 表示退出
- (BOOL) applicationShouldTerminateAfterLastWindowClosed: (NSApplication *)sender
{
    return YES;
}


/// 程序启动完成
/// 执行任务:
/// - RedSoUL初始化
/// - 加载Assets
/// - 创建RenderLoop
- (void) applicationDidFinishLaunching: (NSNotification *)aNotification
{
    NSLog(@"-> App has Launched");
}


/// UnHide程序(如果程序处在Hidden状态)
/// 执行任务: 与“Hide程序”的任务相反
/// - 加载游戏进度
/// - 启动/恢复Audio，Physics
/// - 启动/恢复GameLoop
/// - 恢复RenderLoop
/// NOTE: 不使用:(void)applicationDidBecomeActive:(NSNotification *)notification
/// 因为它在每次程序获得焦点(当程序的窗口切换为前方窗口)时对会调用
- (void) applicationDidUnhide: (NSNotification *)notification
{
    NSLog(@"-> App UNHIDDEN");
}


/// Hide程序(使用Command + H)
/// 执行任务:
/// - 暂停RenderLoop
/// - 暂停GameLoop
/// - 暂停Audio，Physics
/// - 保存游戏进度
/// NOTE: 不要使用:(void)applicationWillResignActive:(NSNotification *)notification
/// 因为它在每次程序失去焦点(当程序的窗口切换为后方窗口)时都会调用
- (void) applicationDidHide: (NSNotification *)notification
{
    NSLog(@"-> App HIDDEN");
}


/// 程序关闭
/// CLEAN-UP
- (void) applicationWillTerminate: (NSNotification *)aNotification
{
    NSLog(@"-> App terminating...");
}


@end
#endif /// (OS_TYPE == OS_TYPE_MAC)
