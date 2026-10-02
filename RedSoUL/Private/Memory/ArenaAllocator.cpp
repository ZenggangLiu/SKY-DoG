/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Memory/MemoryUtilities.hpp"
/// Self header
#include "Memory/ArenaAllocator.hpp"


/// 判断Memory Block索引是否有效
#define IS_VALID_BLOCK_IDX(idx) (idx >= 0)
/// 非法Memory Block索引
#define INVALID_BLOCK_INDEX     (-1)


uint16_t
ArenaAllocator::max_alloc_size ()
{
    return (uint16_t)MemoryUtility::page_size();
}


ArenaAllocator::ArenaAllocator ()
:
    m_free_block_head(INVALID_BLOCK_INDEX),
    m_page_count(0),
    m_increment_rate(0)
{}


ArenaAllocator::~ArenaAllocator ()
{
    release();
}


bool
ArenaAllocator::initialize (
    const uint16_t page_count,
    const uint8_t  increment_rate)
{
    /// 释放现有内存
    release();

    /// 保存数据
    m_page_count     = page_count;
    m_increment_rate = increment_rate;

    /// 申请空间
    return alloc_new_memory_block();
}


std::tuple<float, float>
ArenaAllocator::memory_stats () const
{
    /// Byte -> KiloByte变换的放缩系数
    static constexpr float BYTES_2_KILO_BYTES_SCALE = 1.0f / 1024.0f;

    float total_used_memory = 0;
    float total_free_memory = 0;

    for (const auto & block : m_block_list)
    {
        total_used_memory += block.block_bytes;
        total_free_memory += block.free_bytes;
    }
    total_used_memory *= BYTES_2_KILO_BYTES_SCALE;
    total_free_memory *= BYTES_2_KILO_BYTES_SCALE;

    return std::make_tuple(total_used_memory, total_free_memory);
}


uint8_t *
ArenaAllocator::allocate (
    const uint16_t size)
{
    /// 1. 使用第一个Block(Free Memory Block Head)中分配空间
    /// 2. 如果空间申请完毕后，当前Block已无空闲，将其后续Block设置为Head，此Block将再无参考。
    ///    如果其没有后续Block，则创建一个新的Block(将其为Head)
    ///    逻辑在alloc_in_head_block()中。
    /// 3. 如果无法进行内存分配，逐个查找其后续Block，判断是否某个可以分配空间
    /// 4. 如果找到可以满足申请的Block C: 假设Block C前的一个Block为B
    ///                   FOUND
    ///                   |
    ///                   v
    ///    [P] <- [O] <- [C] <- [B] <- [A] <- HEAD
    ///    [P] <- [O] <- [B] <- [A] <- [C] <- HEAD
    ///    a) [B].next_block = [C].next_block
    ///    b) [C].next_block = HEAD
    ///    c) HEAD = [C]
    ///    d) 进入Step 1
    /// 5. 如果所有后续Block都无法申请空间。创建一个新的Block(将其为Head)，进入Step 1
    ///
    RUNTIME_ASSERT(size < max_alloc_size(),
                   "Too large allocation size(maximal %u bytes)!!", max_alloc_size());
    RUNTIME_ASSERT(IS_VALID_BLOCK_IDX(m_free_block_head),
                   "Invalid free block head. Please call initialize() at first!!");

    if (IS_VALID_BLOCK_IDX(m_free_block_head))
    {
        /// 使用第一个Block(Free Memory Block Head)中分配空间
        uint8_t * alloc_addr = alloc_in_head_block(size);

        /// 如果无法进行内存分配，逐个查找其所有后续Block，判断是否某个可以分配空间
        if (alloc_addr == nullptr)
        {
            /// 用来进行内存分配的Memory Block的索引: 对于上例中的Block C
            int32_t used_block_idx = m_free_block_head;
            /// 刚刚忽略的Block: 对于上例中的Block B
            int32_t skip_block_idx = used_block_idx;
            /// 如果当前Block的空闲空间太小，查找其后续的Block
            while (IS_VALID_BLOCK_IDX(used_block_idx) &&
                   m_block_list[used_block_idx].free_bytes < size)
            {
                skip_block_idx = used_block_idx;
                used_block_idx = m_block_list[used_block_idx].next_block;
            }

            /// 找到可以满足申请的Block，将其设置为Head
            if (IS_VALID_BLOCK_IDX(used_block_idx))
            {
                /// [P] <- [O] <- [C] <- [B] <- [A] <- HEAD
                /// [P] <- [O] <- [B] <- [A] <- [C] <- HEAD
                /// a) [B].next_block = [C].next_block
                /// b) [C].next_block = HEAD
                /// c) HEAD = [C]
                m_block_list[skip_block_idx].next_block =
                    m_block_list[used_block_idx].next_block;
                m_block_list[used_block_idx].next_block = m_free_block_head;
                m_free_block_head = used_block_idx;
            }
            /// 如果所有后续Block都无法申请空间。创建一个新的Block(将其为Head)
            else
            {
                if (!alloc_new_memory_block())
                {
                    return nullptr;
                }
            }

            alloc_addr = alloc_in_head_block(size);
        }

        return alloc_addr;
    }
    else
    {
        return nullptr;
    }
}


void
ArenaAllocator::release ()
{
    /// 释放所有Memory Block
    if (!m_block_list.empty())
    {
        for (const auto & block : m_block_list)
        {
            MemoryUtility::release_vm_pages(block.start_addr, block.page_count);
        }
        m_block_list.clear();
    }

    /// 复位所有成员变量
    m_free_block_head = INVALID_BLOCK_INDEX;
    m_page_count      = 0;
    m_increment_rate  = 0;
}


bool
ArenaAllocator::alloc_new_memory_block ()
{
    // --- 创建一个新的Block，并且将其为设置为空闲Block的Head --- //

    /// 申请内存页
    void *   alloc_addr;
    uint32_t alloc_bytes;
    std::tie(alloc_addr, alloc_bytes) = MemoryUtility::allocate_vm_pages(m_page_count);
    if (alloc_addr && alloc_bytes)
    {
        /// 获得Memory Block的索引
        int32_t new_block_idx = (int32_t)m_block_list.size();

        /// 创建Memory Block
        MemoryBlock new_block;
        new_block.start_addr  = (uint8_t*)alloc_addr;
        new_block.next_block  = m_free_block_head;
        new_block.page_count  = m_page_count;
        new_block.block_bytes = alloc_bytes;
        new_block.free_bytes  = alloc_bytes;
        m_block_list.push_back(new_block);

        /// 将此Memory Block挂到头部
        m_free_block_head = new_block_idx;

        /// 递增下次申请时的内存页数目
        m_page_count *= m_increment_rate;

        return true;
    }
    else
    {
        return false;
    }
}


uint8_t *
ArenaAllocator::alloc_in_head_block (
    const uint16_t size)
{
    uint8_t * alloc_addr  = nullptr;
    MemoryBlock & block = m_block_list[m_free_block_head];

    /// 判断是否可以申请空间
    if (block.free_bytes >= size)
    {
        alloc_addr = block.start_addr + block.block_bytes - block.free_bytes;
        block.free_bytes -= size;
    }

    /// 如果Head Block已无空闲空间
    /// - 将其后续Block设置为Head
    /// - 如果没有后续Block，则申请新的Block，并将新的Block设置为Head
    if (block.free_bytes == 0)
    {
        // 将当前Block从Free Block列表中移除
        m_free_block_head = block.next_block;
        block.next_block  = INVALID_BLOCK_INDEX;

        if (!IS_VALID_BLOCK_IDX(m_free_block_head))
        {
            alloc_new_memory_block();
        }
    }

    /// NOTE: 如果此Block无法满足内存申请，将放回缺省的数值: nullptr
    return alloc_addr;
}
