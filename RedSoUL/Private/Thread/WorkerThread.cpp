/// System headers
#include <cstring>           /// std::memcpy
#if defined(__APPLE__)
#include <mach/thread_act.h> /// thread_suspend, thread_resume
#endif
/// Library headers
#include "Assert/RuntimeAssert.hpp"
/// Self header
#include "Thread/WorkerThread.hpp"


/// PThread的回调函数类型
/// NOTE: 内部的回调返回值类型为uint32_t。
///       PThread将uint32_t整数作为void*看待: (void*)(uint64_t)1234。此处exit_code为1234
///       所以在wait_for_exit()中必须将其CAST回Int: (uint32_t)(uint64_t)exit_code
typedef void* (*pthread_start_routine) (void*);


WorkerThread::WorkerThread (
    const char * const    name       /* = "RedSoUL-Worker Thread" */,
    ConstThreadStackSizeT stack_size /* = ThreadStackSize::DEFAULT_THREAD_STACK_SIZE */)
:
    m_stack_size(stack_size)
{
    set_thread_name(name);
    set_thread_state(ThreadState::CREATED_THREAD_STATE);
}


bool
WorkerThread::start ()
{
    RUNTIME_ASSERT(m_state == ThreadState::CREATED_THREAD_STATE,
                   "The thread is NOT in the 'CREATED' state!!");

    /// 设置堆栈长度: 堆栈的长度(字节长度): 0表示使用缺省长度
    const size_t stack_size = (m_stack_size == ThreadStackSize::DEFAULT_THREAD_STACK_SIZE)
                            ? 0 : (size_t)m_stack_size;

#if (OS_TYPE == OS_TYPE_WIN)
    /// 创建一个挂起的OS线程
    m_handle = CreateThread(nullptr,    /* 此句柄不可继承 */
                            stack_size, /* 保留的堆栈长度: 0为缺省长度 */
                            (LPTHREAD_START_ROUTINE)&WorkerThread::ThreadProc,
                            this, /* 传入ThreadProc()的参数 */
                            /* 挂起此线程，并且设定保留的堆栈长度 */
                            CREATE_SUSPENDED|STACK_SIZE_PARAM_IS_A_RESERVATION,
                            (LPDWORD)&m_thread_id /* 保留ThreadId */);
    if (m_handle)
    {
        /// 启动线程: ResumeThread()返回之前挂起的计数。-1表示调用失败
        if (ResumeThread(m_handle) == (DWORD)-1)
        {
            /// 启动失败
            RUNTIME_ASSERT(false, "Can not start Windows thread!!");
            set_thread_state(ThreadState::INVALID_THREAD_STATE);
            return false;
        }
        else
        {
            /// 设置状态
            set_thread_state(ThreadState::STARTED_THREAD_STATE);
            return true;
        }
    }
    else
    {
        RUNTIME_ASSERT(false, "Can not create Windows thread!!");
        set_thread_state(ThreadState::INVALID_THREAD_STATE);
        return false;
    }

#else /// macOS, iOS, Linux
    /// 创建线程缺省的attribute
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);

    /// 添加joinable属性，因为我们想调用join()来等待此线程的结束
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);

    /// 设置堆栈大小
    pthread_attr_setstacksize(&thread_attr, stack_size);

    /// pthread_create(): 0表示成功，非0为错误码
    const bool is_failed =
        pthread_create(&m_handle,
                       &thread_attr,
                       (pthread_start_routine)&WorkerThread::ThreadProc,
                       this /* 传入ThreadProc()的参数 */);
    if (is_failed)
    {
        RUNTIME_ASSERT(false, "Can not create pthread!!");
        set_thread_state(ThreadState::INVALID_THREAD_STATE);
    }
    else
    {
        set_thread_state(ThreadState::STARTED_THREAD_STATE);
    }

    /// 释放attribute set
    pthread_attr_destroy(&thread_attr);

    return !is_failed;
#endif /// (OS_TYPE == OS_TYPE_WIN)
}


void
WorkerThread::suspend ()
{
    RUNTIME_ASSERT(m_state == ThreadState::RUNNING_THREAD_STATE,
                   "The thread is NOT in the 'RUNNING' state!!");

#if (OS_TYPE == OS_TYPE_WIN)
    /// 挂起线程: SuspendThread()返回之前挂起的计数，-1表示调用失败
    /// NOTE: 在返回前，将暂停线程的运行
    if (SuspendThread(m_handle) != (DWORD)(-1))
    {
        set_thread_state(ThreadState::SUSPENDED_THREAD_STATE);
    }

#elif defined(__APPLE__) /// macOS, iOS
    /// 由于PThread没有提供Suspend函数，所以这里只能使用Mach API
    const mach_port_t apple_thread = pthread_mach_thread_np(m_handle);
    if (thread_suspend(apple_thread) == KERN_SUCCESS)
    {
        set_thread_state(ThreadState::SUSPENDED_THREAD_STATE);
    }

#else /// Linux
    #error No Implementation!!
#endif /// (OS_TYPE == OS_TYPE_WIN)
}


void
WorkerThread::resume ()
{
    RUNTIME_ASSERT(m_state == ThreadState::SUSPENDED_THREAD_STATE,
                   "The thread is NOT in the 'SUSPENDED' state!!");

#if (OS_TYPE == OS_TYPE_WIN)
    /// 启动线程: ResumeThread()返回之前挂起的计数，-1表示调用失败
    /// NOTE: ResumeThread()在返回之前，将运行挂起的线程
    if (ResumeThread(m_handle) != (DWORD)(-1))
    {
        set_thread_state(ThreadState::RUNNING_THREAD_STATE);
    }

#elif defined(__APPLE__) /// macOS, iOS
    /// 由于PThread没有提供Suspend函数，所以这里只能使用Mach API
    const mach_port_t apple_thread = pthread_mach_thread_np(m_handle);
    if (thread_resume(apple_thread) == KERN_SUCCESS)
    {
        set_thread_state(ThreadState::RUNNING_THREAD_STATE);
    }

#else /// Linux
    #error No Implementation!!
#endif /// (OS_TYPE == OS_TYPE_WIN)
}


uint32_t
WorkerThread::wait_for_exit () const
{
#if (OS_TYPE == OS_TYPE_WIN)
    DWORD exit_code;
    if ((WaitForSingleObject(m_handle, INFINITE) == WAIT_OBJECT_0) &&
        GetExitCodeThread(m_handle, &exit_code)) /// True: GetExitCodeThread() succeeded
    {
        return (uint32_t)exit_code;
    }
    return ERROR_EXIT_CODE;

#else /// macOS, iOS, Linux
    void * exit_code; /// 保存强行CAST为void*的整数
    pthread_join(m_handle, &exit_code);
    return (uint32_t)(uint64_t)exit_code; /// 再强行CAST回整数
#endif /// (OS_TYPE == OS_TYPE_WIN)
}


uint32_t
WorkerThread::ThreadProc (
    void * const self_thread)
{
    /// 不同的线程入口:
    /// PThread: void* ThreadProc(void* )
    /// Windows: DWORD ThreadProc(LPVOID)
    ///
    WorkerThread * const SELF = (WorkerThread*)self_thread;
    RUNTIME_ASSERT(SELF->m_state == ThreadState::STARTED_THREAD_STATE,
                   "The thread is NOT in the 'STARTED' state!!");

#if (BUILD_MODE == DEBUG_BUILD_MODE)
    /// 设置线程名称
    /// NOTE: macOS, iOS, Windows只能设置当前运行的线程的名字，所以将名称设置放在ThreadProc中
    ///
#if (OS_TYPE == OS_TYPE_WIN)
#pragma pack(push,8)
        typedef struct tagTHREADNAME_INFO
        {
            DWORD   dwType;     /// Must be 0x1000.
            LPCSTR  szName;     /// Pointer to name (in user addr space).
            DWORD   dwThreadID; /// Thread ID: -1表示当前线程
            DWORD   dwFlags;    /// Reserved for future use, must be zero.
        } THREADNAME_INFO;
#pragma pack(pop)

        THREADNAME_INFO thread_name_info;
        thread_name_info.dwType = 0x1000;
        thread_name_info.szName = (const LPCSTR)SELF->get_thread_name();
        thread_name_info.dwThreadID = -1; /// -1表示当前线程
        thread_name_info.dwFlags    = 0;
        __try
        {
            RaiseException(0x406D1388,
                           0,
                           sizeof(thread_name_info) / sizeof(ULONG_PTR),
                           (ULONG_PTR*)&thread_name_info);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {}

#else /// macOS, iOS, Linux
    pthread_setname_np(SELF->get_thread_name());
#endif /// (OS_TYPE == OS_TYPE_WIN)
#endif /// (BUILD_MODE == DEBUG_BUILD_MODE)

    SELF->set_thread_state(ThreadState::RUNNING_THREAD_STATE);

    /// 调用RunLoop
    const uint32_t exit_code = SELF->run_loop();

    /// RunLoop调用结束，清理资源
    SELF->set_thread_state(ThreadState::TERMINATING_THREAD_STATE);

    SELF->cleanup(exit_code);

    return exit_code;
}


void
WorkerThread::cleanup (
    const uint32_t exit_code)
{
    RUNTIME_ASSERT(m_state == ThreadState::TERMINATING_THREAD_STATE,
                   "The thread is NOT in the 'TERMINATING' state!!");

#if (OS_TYPE == OS_TYPE_WIN)
    /// 关闭句柄
    CloseHandle(m_handle);
    /// 清除缓存的句柄
    m_handle = (HANDLE)-1;
    /// 标记状态
    set_thread_state(ThreadState::DEAD_THREAD_STATE);
    /// 清理线程的资源(如果此线程已经退出)
    /// NOTE: 如果线程正在忙碌，此函数将Blocking
    ExitThread(exit_code);

#else /// macOS, iOS, Linux
    /// 清除缓存的句柄
    m_handle = (pthread_t)-1;
    /// 标记状态
    set_thread_state(ThreadState::DEAD_THREAD_STATE);
    /// 清理线程的资源(如果此线程已经退出)
    /// NOTE: 如果线程正在忙碌，此函数将Blocking
    pthread_exit((void*)(uint64_t)exit_code); /// 强行将uint32_t整数作为void*指针返回
#endif /// (OS_TYPE == OS_TYPE_WIN)
}


const char *
WorkerThread::get_thread_name () const
{
#if (BUILD_MODE == DEBUG_BUILD_MODE)
    return m_name;
#else
    return "";
#endif
}


void
WorkerThread::set_thread_name (
    const char * const new_name)
{
#if (BUILD_MODE == DEBUG_BUILD_MODE)
    RUNTIME_ASSERT(new_name, "Thread name can not be NULL pointer!!");
    RUNTIME_ASSERT(new_name && std::strlen(new_name) <= MAX_THREAD_NAME_LENGTH,
                   "Thread name is too long(maximal %u characters)!!", MAX_THREAD_NAME_LENGTH);
    std::memcpy(m_name, new_name, std::strlen(new_name) + 1);
#endif
}


void
WorkerThread::set_thread_state (
    const ThreadState new_state)
{
#if (BUILD_MODE == DEBUG_BUILD_MODE)
    m_state = new_state;
#endif
}
