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
    Created:  24/02/26  @  10:28 PM
    FileName: WorkerThread.hpp @ RedSoUL Project
    History:
             - created by: 24/02/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


#include "Common/PlatformDefines.hpp" /// OS_TYPE
/// System headers
#if (OS_TYPE == OS_TYPE_WIN)
/// 禁止Windows.h包含winsock.h, 因为我们使用winsock2.h
#define _WINSOCKAPI_
#include <Windows.h>
#undef _WINSOCKAPI_
#else /// MacOS, iOS, Linux
#include <pthread.h>
#endif /// (OS_TYPE == OS_TYPE_WIN)
#include <stdint.h> /// uint32_t,...
/// Library headers
#include "Common/CompilerDefines.hpp" /// BUILD_MODE
#include "Math/MathDefines.hpp"       ///  MAXIMAL_UINT32
#include "Thread/ThreadDefines.hpp"


/// 辅助线程
///
class WorkerThread
{
    typedef const ThreadStackSize ConstThreadStackSizeT;
public:
    /// 错误返回值(线程错误退出时的数值)
    static constexpr uint32_t ERROR_EXIT_CODE = MAXIMAL_UINT32;

    /// 构造一个辅助线程
    explicit
    WorkerThread (
        const char * const   name        = "RedSoUL-Worker Thread",
        ConstThreadStackSizeT stack_size = ThreadStackSize::DEFAULT_THREAD_STACK_SIZE);

    /// 启动此线程: 让OS规划它的运行
    ///
    /// @return
    ///      True:  如果成功启动
    ///      False: 如果启动失败
    bool
    start ();

    /// 挂起此线程(暂停此线程的运行)
    void
    suspend ();

    /// 恢复此线程的运行
    void
    resume ();

    /// 无限期等待此线程退出
    ///
    /// @return
    ///     线程的返回值
    ///     - 0:               成功退出
    ///     - ERROR_EXIT_CODE: 错误退出
    uint32_t
    wait_for_exit () const;


protected:
    /// 此线程的真正任务所在
    /// NOTE: 子类必须提供具体的功能
    ///
    /// @return
    ///     线程的返回值
    ///     - 0:               成功退出
    ///     - ERROR_EXIT_CODE: 错误退出
    virtual
    uint32_t
    run_loop () = 0;

protected:
#if (OS_TYPE == OS_TYPE_WIN)
    typedef HANDLE ThreadHandleT;
    /// 线程句柄
    ThreadHandleT   m_handle;
    /// 线程Id
    WorkerThreadIdT m_thread_id;

#else
    /// 线程句柄
    typedef pthread_t ThreadHandleT;
    ThreadHandleT   m_handle;
#endif /// (OS_TYPE == OS_TYPE_WIN)

    /// 线程堆栈长度
    ConstThreadStackSizeT m_stack_size;

    // --- DEBUG模式下的辅助信息 --- //
#if (BUILD_MODE == DEBUG_BUILD_MODE)
    static constexpr uint16_t MAX_THREAD_NAME_LENGTH = 32;
    /// 线程的状态
    ThreadState m_state;
    /// 线程的名称
    char        m_name[MAX_THREAD_NAME_LENGTH + 1];
#endif


private:
    /// OS线程的入口
    ///
    /// @param[in] self_thread
    ///      当前WorkerThread的实例
    /// @return
    ///     线程的返回值
    ///     - 0:               成功退出
    ///     - ERROR_EXIT_CODE: 错误退出
    static
    uint32_t
    ThreadProc (
        void * const self_thread);

    /// 清理资源
    void
    cleanup (
        const uint32_t exit_code);

    // --- DEBUG模式下的辅助函数 --- //
    /// 获得线程名称
    const char *
    get_thread_name () const;

    /// 设置线程名称
    void
    set_thread_name (
        const char * const new_name);

    /// 设置线程状态
    void
    set_thread_state (
        const ThreadState new_state);
};
