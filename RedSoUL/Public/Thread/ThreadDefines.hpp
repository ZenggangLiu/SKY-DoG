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
    Created:  25/02/26  @  9:40 PM
    FileName: ThreadDefines.hpp @ RedSoUL Project
    History:
             - created by: 25/02/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint32_t,...


/// 线程堆栈的长度(字节长度)
///
enum class ThreadStackSize
{
    /// NOTE:
    /// - iOS: 堆栈的长度将rounded up到页的边界(16k页)
    /// - Mac: 堆栈的长度将rounded up到页的边界(AppleSilicon:16k页, IntelSilicon:4k页)
    /// - Win: 堆栈的长度将rounded up到64k(allocation granularity)
    /// - 最小为16k，必须为内存页的倍数。所有我们这里只定义16k的倍数的堆栈长度
    THREAD_STACK_SIZE_16K  =   16 * 1024,
    THREAD_STACK_SIZE_32K  =   32 * 1024,
    THREAD_STACK_SIZE_64K  =   64 * 1024,
    THREAD_STACK_SIZE_128K =  128 * 1024,
    THREAD_STACK_SIZE_256K =  256 * 1024,
    THREAD_STACK_SIZE_512K =  512 * 1024,
    THREAD_STACK_SIZE_1M   = 1024 * 1024,
    THREAD_STACK_SIZE_2M   = 2048 * 1024,
    THREAD_STACK_SIZE_4M   = 4096 * 1024,
    THREAD_STACK_SIZE_8M   = 8192 * 1024,

    /// 缺省的堆栈长度
    /// - iOS:  512k
    /// - Mac:    8M: 可以通过调用pthread_get_stacksize_np(pthread_self())来获得
    /// - Win:    1M: 可以通过调用GetCurrentThreadStackLimits(Low,HighLim)来获得
    DEFAULT_THREAD_STACK_SIZE = -1
};


/// 线程的状态
///
enum class ThreadState
{
    /// 无效状态
    INVALID_THREAD_STATE,

    /// 已创建但未运转
    CREATED_THREAD_STATE,
    /// 已启动但未运行
    STARTED_THREAD_STATE,
    /// 运转中...
    RUNNING_THREAD_STATE,
    /// 等待Event中...
    WAITING_EVENT_THREAD_STATE,
    /// 已暂停
    SUSPENDED_THREAD_STATE,
    /// 终结中...
    TERMINATING_THREAD_STATE,
    /// 已销毁(DEAD)
    DEAD_THREAD_STATE
};


/// 线程ID
typedef uint64_t WorkerThreadIdT;
