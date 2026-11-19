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
    Created:  29/10/26  @  3:45 PM
    FileName: HashedStringId.hpp @ RedSoUL Project
    History:
             - created by: 29/10/26: Zenggang LIU
             - renamed by: 30/10/26: Zenggang LIU
               从HashedResourceId 更名为--> HashedStringId
               + 因为此类型只提供了对某字符串进行Hash的功能。Hash后的数值作为从字符串的Id使用。
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <cstddef>  /// std::size_t
#include <stdint.h> /// uint32_t,...
/// Library headers
#include "Common/CommonDefines.hpp" /// INLINE_FUNCTION
#include "Hashing/HashUtilities.hpp"
#include "Hashing/RuntimeHash.hpp"
#include "Hashing/StaticStringHash.hpp"


/// 使用Hash函数计算字符串对应的Id
///
template < uint32_t Seed > /// Hash函数使用的SEED
struct HashedStringId
{
    /// Hash数值的类型
#if defined(USE_64BITS_ID) /// 使用64位Id类型
    using HashedValueTypeT = uint64_t;
#else
    using HashedValueTypeT = uint32_t;
#endif

    /// STL要求的比较函数(Equality Predicate)
    struct StlComparer
    {
        INLINE_FUNCTION
        bool
        operator() (
            const HashedStringId first,
            const HashedStringId second) const
        {
            return first == second;
        }
    };

    /// STL要求的Hash函数
    struct StlHasher
    {
        INLINE_FUNCTION
        std::size_t
        operator() (
            const HashedStringId id_to_hash) const
        {
#if defined(USE_64BITS_ID) /// 使用64位Id类型
            const uint64_t hashed_id = HashUtility::split_mix64(id_to_hash.value);
#else
            const uint32_t hashed_id = HashUtility::split_mix32(id_to_hash.value);
#endif
            return static_cast<std::size_t>(hashed_id);
        }
    };

    /// 非法Hash数值
    static constexpr HashedValueTypeT INVALID_HASH_VALUE = (HashedValueTypeT)(-1);

    /// Hash后的数值
    HashedValueTypeT value;

    /// 创建非法Id
    INLINE_FUNCTION
    static
    constexpr
    HashedStringId
    invalid_hashed_id ()
    {
        return HashedStringId{};
    }

    /// 创建BuildTime/编译时的Id
    INLINE_FUNCTION
    static
    constexpr
    HashedStringId
    buildtime_id (
        const char * const string)
    {
#if defined(USE_64BITS_ID) /// 使用64位Id类型
        return HashedStringId{ STATIC_STRING_HASH_64(Seed, string) };
#else
        return HashedStringId{ STATIC_STRING_HASH_32(Seed, string) };
#endif
    }

    /// 创建RunTime/运行时的Id
    INLINE_FUNCTION
    static
    HashedStringId
    runtime_id (
        const uint8_t * const string)
    {
#if defined(USE_64BITS_ID) /// 使用64位Id类型
        return HashedStringId{ RUNTIME_STRING_HASH_64(Seed, string) };
#else
        return HashedStringId{ RUNTIME_STRING_HASH_32(Seed, string) };
#endif
    }

    INLINE_FUNCTION
    constexpr
    HashedStringId ()
    :
        value(INVALID_HASH_VALUE)
    {

    }

    INLINE_FUNCTION
    explicit
    constexpr
    HashedStringId (
        const HashedValueTypeT hashed_value)
    :
        value(hashed_value)
    {

    }

    INLINE_FUNCTION
    constexpr
    bool
    is_valid () const
    {
        return value != INVALID_HASH_VALUE;
    }

    INLINE_FUNCTION
    constexpr
    bool
    is_not_valid () const
    {
        return value == INVALID_HASH_VALUE;
    }

    INLINE_FUNCTION
    constexpr
    bool
    operator== (
        const HashedStringId other) const
    {
        return value == other.value;
    }

    INLINE_FUNCTION
    constexpr
    bool
    operator!= (
        const HashedStringId other) const
    {
        return value != other.value;
    }
};
