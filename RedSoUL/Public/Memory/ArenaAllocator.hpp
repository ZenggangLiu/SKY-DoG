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
    Created:  27/02/26  @  10:32 PM
    FileName: ArenaAllocator.hpp @ RedSoUL Project
    History:
             - created by: 27/02/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint32_t,...
#include <tuple>
#include <vector>


/// 内存Arena:
/// - 所有内存申请共享一个共同的内存区域(Memory Block)
/// - 释放Arena，则释放所有内存申请
///
class ArenaAllocator
{
public:
    /// 获得最大可申请空间长度(字节长度)
    static
    uint16_t
    max_alloc_size ();

    ArenaAllocator ();

    ~ArenaAllocator ();

    /// 初始化一个Arena
    ///
    /// @param[in]  page_count
    ///      申请的内存页数目
    /// @param[in]  increment_rate
    ///      递增比率
    ///      例如: 设定为2，则表示:
    ///           如果需要申请新的内存，则申请两倍当前页数目的空间
    bool
    initialize (
        const uint16_t page_count,
        const uint8_t  increment_rate);

    /// 获得内存统计
    ///
    /// @return
    ///      [ 总共申请的内存长度(KB长度)，空闲的内存长度(KB长度) ]
    std::tuple<float, float>
    memory_stats () const;

    /// 申请空间
    ///
    /// @param[in]  size
    ///      申请空间长度(字节长度)
    ///      NOTE: 目前最大申请空间为256字节
    /// @return
    ///      申请的空间，如果申请成功
    ///      nullptr,  如果申请失败
    uint8_t *
    allocate (
        const uint16_t size);

    /// 释放当前的Arena(即: 释放所有控制的内存)
    void
    release();


private:
    /// 添加一个新的Memory Block
    bool
    alloc_new_memory_block ();

    /// 在第一个空闲Memory Block中分配空间
    uint8_t *
    alloc_in_head_block (
        const uint16_t size);

    /// UNIT TEST逻辑
#if defined(CATCH_PLATFORM_MAC) || defined(CATCH_PLATFORM_WINDOWS) || defined(CATCH_PLATFORM_LINUX)
public:
#else
private:
#endif
    struct MemoryBlock
    {
        /// 起始地址
        uint8_t * start_addr;
        /// 下一个Memory Block的索引: 可能为INVALID_BLOCK_INDEX
        int32_t   next_block;
        /// 内存页数目
        uint32_t  page_count;
        /// Memory Block总共的字节大小
        uint32_t  block_bytes;
        /// 空闲字节数目
        uint32_t  free_bytes;
    };
    typedef std::vector<MemoryBlock> MemoryBlockListT;

    /// 所有Memory Block的列表
    MemoryBlockListT m_block_list;
    /// 第一个空闲Memory Block的索引: 可能为INVALID_BLOCK_INDEX
    int32_t          m_free_block_head;
    /// 如果要申请新的Memory Block使用的页数目
    uint16_t         m_page_count;
    /// 递增比率
    uint8_t          m_increment_rate;
};
