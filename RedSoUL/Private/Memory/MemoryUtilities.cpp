#include "Common/PlatformDefines.hpp" /// OS_TYPE 
/// System headers
#if (OS_TYPE == OS_TYPE_WIN)
    #include <cstdlib>  /// _aligned_malloc, _aligned_free
    #include <Windows.h>
    #include <psapi.h>  /// PROCESS_MEMORY_COUNTERS, GetProcessMemoryInfo
#elif defined(__APPLE__)
    #include <mach/mach_init.h>
    #include <mach/task.h>
    #include <mach/task_info.h>
    #include <mach/vm_page_size.h>
    #if (OS_TYPE == OS_TYPE_MAC)
        #include <mach/mach_vm.h>
        #include <mach/vm_map.h>
    #else
        #include <mach/vm_map.h>
    #endif
    #include <stdlib.h>   /// posix_memalign
#elif (OS_TYPE == OS_TYPE_LINUX)
    #include <fstream>    /// ifstream
    #include <stdlib.h>   /// posix_memalign
    #include <string>
    #include <sys/mman.h> /// mmap, munmap
    #include <unistd.h>   /// sysconf
#endif
/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Common/CommonDefines.hpp" /// IS_POWER_OF_TWO
/// Self header
#include "Memory/MemoryUtilities.hpp"


#if defined(__APPLE__)
/// 申请指定大小的内存
///
/// @param[in]  alloc_size
///      想要申请的内存大小(字节)
///      NOTE: 此大小必须为Page size的整数倍
/// @param[in]  alloc_flag
///      申请时候使用的Flag
///      参考: VM allocation flags
///      - VM_FLAGS_FIXED: 尽可能在指定地址处申请内存空间
///      - VM_FLAGS_ANYWHERE: 在任意地方申请内存空间
///      - VM_FLAGS_PURGABLE: 此申请的内存空间在回收的时候不用Page out出去
///      - VM_FLAGS_OVERWRITE|VM_FLAGS_FIXED: 在指定地址申请内存空间，并且替换此处当前申请的空间
/// @param[in, out] alloc_addr
///      如果alloc_flag为VM_FLAGS_ANYWHERE: 用来保存申请的地址
///      如果alloc_flag为VM_FLAGS_FIXED:    目标地址
///
static
bool
allocate_vm (
    const uint32_t alloc_size,
    const uint32_t alloc_flag,
    void ** const  alloc_addr)
{
    RUNTIME_ASSERT((alloc_size & (MemoryUtility::page_size() - 1)) == 0,
                    "The allocated memory size must be a multiple of page size");

#if (OS_TYPE == OS_TYPE_MAC)
    // VM_FLAGS_ANYWHERE: 在VM的空间的任意位置创建一个大小合适的区域
    const  kern_return_t op_result =
        mach_vm_allocate(
            (vm_map_t)mach_task_self(),
            (mach_vm_address_t*)alloc_addr,
            (mach_vm_size_t)alloc_size,
            alloc_flag);
#else
    const kern_return_t op_result =
        vm_allocate(
            (vm_map_t)mach_task_self(),
            (vm_address_t*)alloc_addr,
            (vm_size_t)alloc_size,
            alloc_flag);
#endif /// (OS_TYPE == OS_TYPE_MAC)
    return op_result == KERN_SUCCESS;
}


/// 释放指定大小的内存
static
bool
release_vm (
    void * const   alloc_addr,
    const uint32_t alloc_size)
{
    RUNTIME_ASSERT((alloc_size & (MemoryUtility::page_size() - 1)) == 0,
                    "The allocated memory size must be a multiple of page size");

#if (OS_TYPE == OS_TYPE_MAC)
    const kern_return_t op_result =
        mach_vm_deallocate(
            (vm_map_t)mach_task_self(),
            (mach_vm_address_t)alloc_addr,
            (mach_vm_size_t)alloc_size);
#else
    const kern_return_t op_result =
        vm_deallocate(
            (vm_map_t)mach_task_self(),
            (vm_address_t)alloc_addr,
            (vm_size_t)alloc_size);
#endif /// (OS_TYPE == OS_TYPE_MAC)
    return op_result == KERN_SUCCESS;
}
#endif /// defined(__APPLE__)



std::tuple<bool, uint64_t, uint64_t>
MemoryUtility::memory_usage ()
{
#if (OS_TYPE == OS_TYPE_WIN)
    PROCESS_MEMORY_COUNTERS mem_info;
    /// WorkingSetSize: Working set/Resident set: the amount of physical memory mapped to this process
    /// PagefileUsage : VM size: the amount of virtual memory committed to this process
    /// GetProcessMemoryInfo(): 返回非零，如果操作成功
    if (GetProcessMemoryInfo(GetCurrentProcess(), &mem_info, sizeof(mem_info)))
    {
        return std::make_tuple(
            true, (uint64_t)mem_info.WorkingSetSize, (uint64_t)mem_info.PagefileUsage);
    }
#elif defined(__APPLE__)
    mach_task_basic_info mach_task_info;
    mach_msg_type_number_t info_size = MACH_TASK_BASIC_INFO_COUNT;
    const kern_return_t op_result = task_info(
        mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&mach_task_info, &info_size);
    if (op_result == KERN_SUCCESS)
    {
        return std::make_tuple(
            true, (uint64_t)mach_task_info.resident_size, (uint64_t)mach_task_info.virtual_size);
    }
#elif (OS_TYPE == OS_TYPE_LINUX)
    struct ProcessMemoryInfo
    {
        long physical_mem = 0;
        long virtual_mem  = 0;
    } mem_info;

    std::ifstream proc_status_file("/proc/self/status");
    std::string   status_line;

    if (proc_status_file.is_open())
    {
        while(std::getline(proc_status_file, status_line))
        {
            if (status_line.find("VmSize:") == 0)
            {
                mem_info.virtual_mem = std::stol(status_line.substr(7));
            }
            else if (status_line.find("VmRSS:") == 0)
            {
                mem_info.physical_mem = std::stol(status_line.substr(6));
            }
        }
        proc_status_file.close();
    }

    if (mem_info.physical_mem && mem_info.virtual_mem)
    {
        return std::make_tuple(
            true, (uint64_t)mem_info.physical_mem * 1024, (uint64_t)mem_info.virtual_mem * 1024);
    }
#else
    #error TODO: No implementation
#endif /// (OS_TYPE == OS_TYPE_WIN)
    else
    {
        return std::make_tuple(false, 0, 0);
    }
}


uint32_t
MemoryUtility::page_size ()
{
#if (OS_TYPE == OS_TYPE_WIN)
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    return (uint32_t)sys_info.dwPageSize;
#elif defined(__APPLE__)
    return (uint32_t)vm_page_size;
#elif (OS_TYPE == OS_TYPE_LINUX)
    const long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1)
    {
        return 0;
    }
    else
    {
        return (uint32_t)page_size;
    }
#else
    #error TODO: No implementation
#endif /// (OS_TYPE == OS_TYPE_WIN)
}


void *
MemoryUtility::aligned_alloc (
    const uint32_t alloc_size,
    const uint32_t alignment)
{
    /// posix_memalign()有如下要求
    RUNTIME_ASSERT(alignment >= sizeof(void*) && IS_POWER_OF_TWO(alignment),
                   "Wrong alignment(at least %u, and is power of 2!!", sizeof(void*));

#if (OS_TYPE == OS_TYPE_WIN)
    return _aligned_malloc(alloc_size, alignment);
#else /// macOS, iOS, Linux
    void * alloc_addr;
    const int opcode = posix_memalign(&alloc_addr, alignment, alloc_size);
    if (opcode == 0)
    {
        return alloc_addr;
    }
    else
    {
        return nullptr;
    }
#endif
}


void
MemoryUtility::aligned_free (
    const void * const alloc_addr)
{
#if (OS_TYPE == OS_TYPE_WIN)
    _aligned_free((void*)alloc_addr);
#else /// macOS, iOS, Linux
    free((void*)alloc_addr);
#endif
}


std::tuple<void*, uint32_t>
MemoryUtility::allocate_vm_pages (
    const uint32_t page_count)
{
    RUNTIME_ASSERT(page_count * page_size() <= 0xFFFFFFFF,
                   "We can JUST allocate 4G at most");

    const uint32_t alloc_size = page_count * page_size();
#if (OS_TYPE == OS_TYPE_WIN)
    /// NOTE:
    /// - MEM_COMMIT: 分配虚拟空间
    /// === 起始时，内存统计(页字节数: 4096) ===
    /// [虚拟内存]: 0.43Mb, [物理内存]: 2.58Mb
    /// === 使用了ALLOC函数 ===
    /// [虚拟内存]: 4.45Mb, [物理内存]: 2.62Mb <-- 虚拟内存增加，物理内存不变
    /// === 写入(0xFF) ===
    /// [虚拟内存]: 4.45Mb, [物理内存]: 2.62Mb
    /// === RELEASE ===
    /// [虚拟内存]: 4.45Mb, [物理内存]: 2.62Mb
    ///
    /// - MEM_RESERVE: 不分配虚拟空间，但是无法访问此内存空间
    /// === 起始时，内存统计(页字节数: 4096) ===
    /// [虚拟内存]: 0.43Mb, [物理内存]: 2.58Mb
    /// === ALLOC ===
    /// [虚拟内存]: 0.44Mb, [物理内存]: 2.62Mb
    /// === RELEASE ===
    /// [虚拟内存]: 0.44Mb, [物理内存]: 2.62Mb
    ///
    void * const memory_addr =
        VirtualAlloc(NULL, alloc_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (memory_addr != nullptr)
#elif defined(__APPLE__)
    void * memory_addr = nullptr;
    allocate_vm(alloc_size, VM_FLAGS_ANYWHERE, &memory_addr);
    if (memory_addr != nullptr)
#elif (OS_TYPE == OS_TYPE_LINUX)
    void * const memory_addr = mmap(nullptr,   // system selects the location
                                    alloc_size,
                                    PROT_READ | PROT_WRITE,      // read & write right
                                    MAP_PRIVATE | MAP_ANONYMOUS, // mapping
                                    -1,                           // file descriptor
                                    0);
    if (memory_addr != MAP_FAILED)
#else
    #error TODO: No implementation
#endif /// (OS_TYPE == OS_TYPE_WIN)
    {
        return std::make_tuple(memory_addr, alloc_size);
    }
    else
    {
        return std::make_tuple(nullptr, 0);
    }
}


bool
MemoryUtility::release_vm_pages (
    void * const   alloc_addr,
    const uint32_t page_count)
{
    RUNTIME_ASSERT(page_count * page_size() <= 0xFFFFFFFF,
                   "We can JUST allocate 4G at most");

#if (OS_TYPE == OS_TYPE_WIN)
    /// 调用错误放回: 0
    /// NOTE: If dwFreeType is MEM_RELEASE, dwSize must be 0 (zero).
    return VirtualFree(alloc_addr, 0, MEM_RELEASE) != 0;
#elif defined(__APPLE__)
    return release_vm(alloc_addr, page_count * page_size());
#elif (OS_TYPE == OS_TYPE_LINUX)
    return munmap(alloc_addr, page_count * page_size()) != -1;
#else
    #error TODO: No implementation
#endif
}
