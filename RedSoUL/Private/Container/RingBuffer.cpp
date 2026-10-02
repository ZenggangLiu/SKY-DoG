/// System headers
#include <algorithm> /// std::min
#include <cstring>   /// std::memcpy
/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Math/MathUtilities.hpp"
#include "Memory/MemoryUtilities.hpp"
/// Self header
#include "Container/RingBuffer.hpp"


/// 将指定Size，RoundUp到下一个2的N此幂
static
uint64_t
round_up_to_power_of_two (
    const uint64_t size)
{
    /// 2, 3,...
    if (size > 1)
    {
        uint64_t rounded = size - 1;
        rounded |= rounded >> 1;
        rounded |= rounded >> 2;
        rounded |= rounded >> 4;
        rounded |= rounded >> 8;
        rounded |= rounded >> 16;
        rounded |= rounded >> 32;
        rounded++;

        return rounded;
    }
    /// 0, 1
    else
    {
        return 1;
    }
}


/// 计算需要的内存页数目
static
uint32_t
vm_pages_needed (
    const uint64_t exp_size)
{
    const uint32_t needed_pages =
        (uint32_t)MathUtility::round_up_count(
            round_up_to_power_of_two(exp_size), (uint64_t)MemoryUtility::page_size());
    return needed_pages;
}


static
uint8_t *
allocate_data_buffer (
    const uint32_t page_count)
{
    /// 申请内存页
    void *   alloc_addr;
    uint32_t alloc_bytes;
    std::tie(alloc_addr, alloc_bytes) = MemoryUtility::allocate_vm_pages(page_count);
    return (uint8_t*)alloc_addr;
}



RingBuffer::RingBuffer (
    const uint64_t exp_size)
:
    m_vm_page_count(vm_pages_needed(exp_size)),
    m_buffer_size(m_vm_page_count * MemoryUtility::page_size()),
    m_modulo_mask(m_buffer_size - 1),
    m_data_buffer(allocate_data_buffer(m_vm_page_count)),
    m_data_read_pos(0),
    m_data_save_pos(0)
{
    RUNTIME_ASSERT(IS_POWER_OF_TWO(m_buffer_size),
                   "Buffer size must be of power of 2!!");

    /// 没有Memory Order要求
    m_saved_data_head.store(0, std::memory_order_relaxed);
    m_saved_data_tail.store(0, std::memory_order_relaxed);
}


RingBuffer::~RingBuffer()
{
    MemoryUtility::release_vm_pages(m_data_buffer, m_vm_page_count);
}



// MARK: == CONSUMER THREAD ACCESS ONLY ==
// +----------------------------------+ //
// |   CONSUMER THREAD ACCESS ONLY    | //
// +----------------------------------+ //
bool
RingBuffer::is_empty () const
{
    /// 读入辅助线程(Producer)最新设定的数据尾部值
    /// 并同步Producer线程(即: 同步所有mSaveDataTail之前的所有内存操作)
    const uint64_t saved_data_tail = m_saved_data_tail.load(std::memory_order_acquire);
    return (saved_data_tail - m_data_read_pos) == 0;
}


void
RingBuffer::finish_read ()
{
    /// 更新数据头
    m_saved_data_head.store(m_data_read_pos, std::memory_order_release);
}


void
RingBuffer::reset_read ()
{
    m_data_read_pos = m_saved_data_head.load(std::memory_order_relaxed);
}



// MARK: == PRODUCER THREAD ACCESS ONLY ==
// +----------------------------------+ //
// |   PRODUCER THREAD ACCESS ONLY    | //
// +----------------------------------+ //
bool
RingBuffer::is_full () const
{
    /// 读入辅助线程(Consumer)最新设定的数据头部值
    /// 并同步Consumer线程(即: 同步所有mSaveDataHead之前的所有内存操作)
    const uint64_t saved_data_head = m_saved_data_head.load(std::memory_order_acquire);
    const uint64_t saved_data_size = m_data_save_pos - saved_data_head;
    const uint64_t free_space_size = m_buffer_size   - saved_data_size;

    return free_space_size == 0;
}


void
RingBuffer::finish_save ()
{
    /// 更新数据尾
    m_saved_data_tail.store(m_data_save_pos, std::memory_order_release);
}


void
RingBuffer::reset_save ()
{
    m_data_save_pos = m_saved_data_tail.load(std::memory_order_relaxed);
}


bool
RingBuffer::read_data_in_bytes (
    void * const   output_buffer_ptr,
    const uint64_t buffer_size_in_bytes,
    const uint64_t exp_data_size_in_bytes)
{
    /// Buffer足够长
    if (buffer_size_in_bytes >= exp_data_size_in_bytes)
    {
        /// 读入辅助线程(Producer)最新设定的数据尾部值
        /// 并同步Producer线程(即: 同步所有mSaveDataTail之前的所有内存操作)
        const uint64_t saved_data_tail = m_saved_data_tail.load(std::memory_order_acquire);
        const uint64_t saved_data_size = saved_data_tail - m_data_read_pos;
        /// 有足够的数据可读
        if (saved_data_size >= exp_data_size_in_bytes)
        {
            /// 计算Mod后, 数据读入位置索引
            const uint64_t data_read_index = m_data_read_pos & m_modulo_mask;

            /// 计算最多可以往后读入多少字节: 从当前ReadIndex开始一直到Buffer末尾
            const uint64_t read_bytes_to_buffer_end =
                (uint64_t)std::min(exp_data_size_in_bytes, m_buffer_size - data_read_index);
            /// 复制随后的数据
            uint8_t * const data_outout_buffer = (uint8_t*)output_buffer_ptr;
            std::memcpy(
                data_outout_buffer,
                &m_data_buffer[data_read_index],
                read_bytes_to_buffer_end);

            /// 计算需要从Buffer头部读入多少字节，如果有Wrap
            const uint64_t read_bytes_from_buffer_start =
                exp_data_size_in_bytes - read_bytes_to_buffer_end;
            /// 复制Buffer开头处的数据
            if (read_bytes_from_buffer_start)
            {
                std::memcpy(
                    data_outout_buffer + read_bytes_to_buffer_end,
                    &m_data_buffer[0],
                    read_bytes_from_buffer_start);
            }

            /// 更新数据读入位置
            m_data_read_pos += exp_data_size_in_bytes;
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}


bool
RingBuffer::save_data_in_bytes (
    const void * const input_buffer_ptr,
    const uint64_t     exp_data_size_in_bytes)
{
    /// 读入辅助线程(Consumer)最新设定的数据头部值
    /// 并同步Consumer线程(即: 同步所有mSaveDataHead之前的所有内存操作)
    const uint64_t saved_data_head = m_saved_data_head.load(std::memory_order_acquire);
    const uint64_t saved_data_size = m_data_save_pos - saved_data_head;
    const uint64_t free_space_size = m_buffer_size   - saved_data_size;
    /// 有足够空间可写出
    if (free_space_size >= exp_data_size_in_bytes)
    {
        /// 计算Mod后, 数据写出位置索引
        const uint64_t data_save_index = m_data_save_pos & m_modulo_mask;

        /// 计算最多可以往后写出多少字节: 从当前SaveIndex开始一直到Buffer末尾
        const uint64_t save_bytes_to_buffer_end =
            (uint64_t)std::min(exp_data_size_in_bytes, m_buffer_size - data_save_index);
        /// 依次写出数据
        const uint8_t * const data_input_buffer = (const uint8_t*)input_buffer_ptr;
        std::memcpy(
            &m_data_buffer[data_save_index],
            data_input_buffer,
            save_bytes_to_buffer_end);

        /// 计算需要从Buffer头部写出多少字节，如果有Wrap
        const uint64_t save_bytes_from_buffer_start =
            exp_data_size_in_bytes - save_bytes_to_buffer_end;
        if (save_bytes_from_buffer_start)
        {
            std::memcpy(
                &m_data_buffer[0],
                data_input_buffer + save_bytes_to_buffer_end,
                save_bytes_from_buffer_start);
        }

        /// 更新数据写出位置
        m_data_save_pos += exp_data_size_in_bytes;
        return true;
    }
    else
    {
        return false;
    }
}
