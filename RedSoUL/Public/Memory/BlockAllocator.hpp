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
    Created:  15/05/26  @  10:54 PM
    FileName: BlockAllocator.hpp @ RedSoUL Project
    History:
             - created by: 15/05/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint8_t, uint32_t,...
/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Common/CommonDefines.hpp" /// INLINE_FUNCTION, UNUSED_VARIABLE
#include "Math/MathUtilities.hpp"   /// round_up_count


class BlockAllocatorImpl;


// MARK: == BlockAllocatorBridge == //
/// BlockAllocator与BlockAllocator具体实现之间的桥梁
///
struct BlockAllocatorBridge
{
    /// 创建Allocator的具体实现
    static
    BlockAllocatorImpl *
    create (
        const uint16_t block_size);

    static
    void
    destroy (
        BlockAllocatorImpl * & allocator);

    static
    bool
    initialize (
        BlockAllocatorImpl * const allocator,
        const uint32_t             logic_page_count,
        const uint8_t              increment_rate);

    /// 申请一个Block
    static
    uint8_t *
    allocate (
        BlockAllocatorImpl * const allocator);

    /// 释放一个Block
    static
    bool
    deallocate (
        BlockAllocatorImpl * const allocator,
        void * const               alloc_addr);

    static
    void
    release (
        BlockAllocatorImpl * const allocator);

    static
    uint16_t
    block_size (
        const BlockAllocatorImpl * const allocator);

    static
    uint32_t
    block_count (
        const BlockAllocatorImpl * const allocator);

    static
    uint32_t
    free_count (
        const BlockAllocatorImpl * const allocator);

    static
    uint32_t
    used_count (
        const BlockAllocatorImpl * const allocator);

    static
    uint32_t
    allocated_bytes (
        const BlockAllocatorImpl * const allocator);
};



// MARK: == BlockAllocator == //
/// BlockAllocator中内存页的大小(4K｜4096字节)
static constexpr uint32_t BLOCK_ALLOCATOR_PAGE_SIZE = 4096u;


/// 内存Block分配器(以相同大小的Block的方式进行内存分配)
/// NOTE:
/// - Block大小范围为[16字节，256字节], 之间的任意8字节的倍数:
///   [16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128, ..., 256]
///
template <typename ClassType, uint32_t ObjectCount> /// <Block类型, 起始物体个数>
class BlockAllocator
{
public:
    static
    INLINE_FUNCTION
    BlockAllocator &
    ref ()
    {
        static BlockAllocator s_instance;
        return s_instance;
    }

    /// 初始化Allocator
    ///
    /// @param[in]  logic_page_count
    ///     初始的内存页数目(逻辑内存页的大小为4K: 4096字节)
    /// @param[in]  increment_rate
    ///     递增比率
    ///     例如: 设定为2，则表示:
    ///           如果当前没有空闲Block时, 需要申请新的内存，则申请两倍当前页数目的空间
    INLINE_FUNCTION
    bool
    initialize (
        const uint32_t logic_page_count = DEFAULT_LOGIC_PAGE_COUNT, /// 1
        const uint8_t  increment_rate   = DEFAULT_INCREMENT_RATE)   /// 2
    {
        return BlockAllocatorBridge::initialize(
            m_allocator, logic_page_count, increment_rate);
    }

    /// 申请一个Block
    INLINE_FUNCTION
    uint8_t *
    allocate ()
    {
        return BlockAllocatorBridge::allocate(m_allocator);
    }

    /// 释放一个Block
    INLINE_FUNCTION
    bool
    deallocate (
        void * const alloc_addr)
    {
        return BlockAllocatorBridge::deallocate(m_allocator, alloc_addr);
    }

    /// 释放当前Allocator占用的所有内存
    INLINE_FUNCTION
    void
    release ()
    {
        BlockAllocatorBridge::release(m_allocator);
    }

    /// 获取Block大小(字节数)
    INLINE_FUNCTION
    uint16_t
    block_size () const
    {
        return BlockAllocatorBridge::block_size(m_allocator);
    }

    /// 获取Block的总数
    INLINE_FUNCTION
    uint32_t
    block_count () const
    {
        return BlockAllocatorBridge::block_count(m_allocator);
    }

    /// 获取FreeBlock数目
    INLINE_FUNCTION
    uint32_t
    free_count () const
    {
        return BlockAllocatorBridge::free_count(m_allocator);
    }

    /// 获取UsedBlock数目
    INLINE_FUNCTION
    uint32_t
    used_count () const
    {
        return BlockAllocatorBridge::used_count(m_allocator);
    }

    /// 获取占用的内存大小(字节数)
    INLINE_FUNCTION
    uint32_t
    allocated_bytes () const
    {
        return BlockAllocatorBridge::allocated_bytes(m_allocator);
    }


private:
    INLINE_FUNCTION
    BlockAllocator ()
    :
        m_allocator(BlockAllocatorBridge::create(BLOCK_SIZE_USED))
    {
        static constexpr uint32_t OBJECT_BYTE_SIZE = (uint32_t)BLOCK_SIZE_USED * ObjectCount;
        static const     uint32_t LOGIC_PAGE_COUNT =
            MathUtility::round_up_count(OBJECT_BYTE_SIZE, BLOCK_ALLOCATOR_PAGE_SIZE);

        const bool is_initialized = initialize(LOGIC_PAGE_COUNT, DEFAULT_INCREMENT_RATE);
        UNUSED_VARIABLE(is_initialized);
        RUNTIME_ASSERT(is_initialized, "Can not initialize the allocator!!");
    }

    INLINE_FUNCTION
    ~BlockAllocator ()
    {
        BlockAllocatorBridge::destroy(m_allocator);
    }

    BlockAllocator (
        const BlockAllocator &) = delete;
    BlockAllocator & operator = (
        const BlockAllocator &) = delete;

private:
    static constexpr uint8_t DEFAULT_LOGIC_PAGE_COUNT = 1;
    static constexpr uint8_t DEFAULT_INCREMENT_RATE   = 2;
    static constexpr uint32_t BITS_MASK = ~((uint32_t)7);
    /// round up到的下一个8的倍数
    static constexpr uint32_t CLASS_SIZE_ROUNDED_UP = (uint32_t)sizeof(ClassType) + 7;
    /// 使用的Block大小(字节数)
    static constexpr uint16_t BLOCK_SIZE_USED =
        (CLASS_SIZE_ROUNDED_UP & BITS_MASK) < 16 ?
        16                                       :
        (uint16_t)(CLASS_SIZE_ROUNDED_UP & BITS_MASK);

    static_assert(sizeof(ClassType) <= 256,
                  "The given class is too large, "
                  "BlockAllocator can at most allocate 256 bytes!!");
    static_assert(BLOCK_SIZE_USED <= 256,
                  "The block size is too large, "
                  "BlockAllocator can at most allocate 256 bytes!!");
    static_assert(ObjectCount > 0,
                  "BlockAllocator must be used as BlockAllocator<T, ObjectCount>, "
                  "and ObjectCount must be greater than ZERO!!");

    /// 真实的Allocator实例
    BlockAllocatorImpl * m_allocator;
};
