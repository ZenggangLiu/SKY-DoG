/// System headers
#include <cstdio>  /// std::printf
#include <cstring> /// std::memcpy, std::strlen
/// Library headers
#include "Assert/RuntimeAssert.hpp"   /// RUNTIME_ASSERT
#include "Math/MathUtilities.hpp"     /// round_up_count
#include "Memory/MemoryUtilities.hpp" /// page_size
/// Self header
#include "Text/StaticStringDepot.hpp"


StaticStringDepot &
StaticStringDepot::ref ()
{
    static StaticStringDepot s_instance;
    return s_instance;
}


void
StaticStringDepot::memory_usage () const
{
#if (PROFILING_MODE == 1)
    std::printf("<<static string depot>>: used %.2fMB memory\n",
                std::get<0>(m_allocator.memory_stats()) / 1024);
#endif
}


bool
StaticStringDepot::is_cached_string_id (
    const StaticStringIdT string_id) const
{
    return m_info_table.find(string_id) != m_info_table.end();
}


StaticStringIdT
StaticStringDepot::get_empty_string_id () const
{
    /// 空字符的Id
    static constexpr StaticStringIdT EMPTY_STRING_ID = StaticStringIdT::buildtime_id("");

    return EMPTY_STRING_ID;
}


uint16_t
StaticStringDepot::length (
    const StaticStringIdT string_id) const
{
    RUNTIME_ASSERT(string_id.is_valid(), "Invalid string Id!!");

    ConstInfoIteratorT string_info = m_info_table.find(string_id);
    RUNTIME_ASSERT(string_info != m_info_table.end(), "Can not find the string key!!");

    return string_info->second.string_length;
}


const uint8_t *
StaticStringDepot::data (
    const StaticStringIdT string_id) const
{
    RUNTIME_ASSERT(string_id.is_valid(), "Invalid string Id!!");

    ConstInfoIteratorT string_info = m_info_table.find(string_id);
    RUNTIME_ASSERT(string_info != m_info_table.end(), "Can not find the string key!!");

    return string_info->second.start_address;
}


StaticStringIdT
StaticStringDepot::cache_string (
    const char * const ascii_string)
{
    return cache_string((const uint8_t*)ascii_string);
}


StaticStringIdT
StaticStringDepot::cache_string (
    const uint8_t * const utf8_string)
{
    const StaticStringIdT string_id = StaticStringIdT::runtime_id(utf8_string);

    /// 如果Depot中没有数据
    if (m_info_table.find(string_id) == m_info_table.end())
    {
        /// NOTE: 对于UTF8编码的字符串std::strlen()返回它的总共字节长度(不考虑结尾处的\x0')
        const uint16_t  string_length  = (uint16_t)std::strlen((const char*)utf8_string);
        uint8_t * const string_address = m_allocator.allocate(string_length + 1);
        std::memcpy(string_address, utf8_string, string_length + 1);

        StringDataInfo & new_info = m_info_table[string_id];
        new_info.start_address = string_address;
        new_info.string_length = string_length;
    }

    return string_id;
}


StaticStringDepot::StaticStringDepot ()
{
    static constexpr uint32_t DEPOT_BYTE_SIZE = 4 * 1024;

    m_allocator.initialize(
        MathUtility::round_up_count(DEPOT_BYTE_SIZE, MemoryUtility::page_size()), 2);

    uint8_t * const empty_string_address = m_allocator.allocate(1);
    empty_string_address[0] = 0;

    StringDataInfo & new_info = m_info_table[get_empty_string_id()];
    new_info.start_address = empty_string_address;
    new_info.string_length = 0;
}


StaticStringDepot::~StaticStringDepot ()
{
    m_info_table.clear();
    m_allocator.release();
}
