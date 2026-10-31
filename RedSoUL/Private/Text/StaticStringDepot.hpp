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
    Created:  5/03/26  @  11:54 AM
    FileName: StaticStringDepot.hpp @ RedSoUL Project
    History:
             - created by: 5/03/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint32_t,...
#include <unordered_map>
/// Library headers
#include "Memory/ArenaAllocator.hpp"
#include "Text/StaticStringId.hpp"


class StaticStringDepot
{
public:
    /// 获得对此Depot的参考
    static
    StaticStringDepot &
    ref ();

    /// 输出内存使用统计
    void
    memory_usage () const;

    /// 判断指定字符串Id是否属于任何Cached的字符串
    bool
    is_cached_string_id (
        const StaticStringIdT string_id) const;

    /// 获得空字符串的Id
    StaticStringIdT
    get_empty_string_id () const;

    /// 获得指定Id对应的字符串长度
    /// NOTE: 对于UTF8编码的字符串，返回的长度为此字符串对应的字节长度(不包括结尾处的\x0')
    uint16_t
    length (
        const StaticStringIdT string_id) const;

    /// 获得指定Id对应的字符串内容
    const uint8_t *
    data (
        const StaticStringIdT string_id) const;

    /// 缓存指定的ASCII编码的字符串
    StaticStringIdT
    cache_string (
        const char * const ascii_string);

    /// 缓存指定的UTF8编码的字符串
    StaticStringIdT
    cache_string (
        const uint8_t * const utf8_string);


private:
     StaticStringDepot ();
    ~StaticStringDepot ();


private:
    /// String数据信息: NOTE: 每个String都以'\x0'结尾
    ///
    /// Layout如下:
    ///     | String 1     |  String 2    |
    /// +---+----------+--------------+---+-------+
    /// | 0 | ABCE ... | 0 | 12345... | 0 | ~ ~ ~ |
    /// +---+----------+--------------+---+-------+
    ///   ^
    ///   |
    ///   空字符串
    ///
    struct StringDataInfo
    {
        /// 字符串起始地址
        const uint8_t * start_address;
        /// 字符串长度
        /// NOTE: 对于UTF8编码的字符串，此处为字符串对应的字节长度(不包括结尾处的\x0')
        uint16_t        string_length;
    };

    typedef std::unordered_map < StaticStringIdT,
                                 StringDataInfo,
                                 StaticStringIdT::StlHasher,
                                 StaticStringIdT::StlComparer > StringInfoTableT;
    typedef StringInfoTableT::const_iterator                    ConstInfoIteratorT;

    ArenaAllocator   m_allocator;
    StringInfoTableT m_info_table;
};
