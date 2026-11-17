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
    Created:  7/11/26  @  12:43 PM
    FileName: TypedResourceId.hpp @ RedSoUL Project
    History:
             - created by: 7/11/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <cstddef>  /// std::size_t
#include <stdint.h> /// uint8_t, uint32_t,...
/// Library headers
#include "Common/CommonDefines.hpp" /// INLINE_FUNCTION
#include "Hashing/HashUtilities.hpp"


/// 内嵌Type类型的资源Id
///
template < uint8_t Type > /// 资源的Type
struct TypedResourceId
{
#if defined(USE_64BITS_ID) /// 使用64位Id类型
    using ValueTypeT = uint64_t;
    static constexpr ValueTypeT TYPE_MASK  = 0xFF00000000000000ULL;
    static constexpr uint8_t    TYPE_SHIFT = 56;
#else
    using ValueTypeT = uint32_t;
    static constexpr ValueTypeT TYPE_MASK  = 0xFF000000U;
    static constexpr uint8_t    TYPE_SHIFT = 24;
#endif

    /// STL要求的比较函数(Equality Predicate)
    struct StlComparer
    {
        INLINE_FUNCTION
        bool
        operator() (
            const TypedResourceId first,
            const TypedResourceId second) const
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
            const TypedResourceId id_to_hash) const
        {
#if defined(USE_64BITS_ID) /// 使用64位Id类型
            const uint64_t hashed_id = HashUtility::split_mix64(id_to_hash.value);
#else
            const uint32_t hashed_id = HashUtility::split_mix32(id_to_hash.value);
#endif
            return static_cast<std::size_t>(hashed_id);
        }
    };

    static constexpr uint8_t INVALID_TYPE = 0xFF;
    static_assert(Type != INVALID_TYPE, "Type can not be invalid(0xFF)!!");

    /// 内嵌Type类型的Id的数值
    ValueTypeT value;

    /// 创建内嵌Type类型的Id的数值
    ///
    /// @param[in]  type_value
    ///     Id类型
    /// @param[in]  id_value
    ///     Id的数值
    INLINE_FUNCTION
    static
    ValueTypeT
    make_typed_value (
        const uint8_t    type_value,
        const ValueTypeT id_value)
    {
        return ((ValueTypeT)type_value << TYPE_SHIFT) |
               (id_value & ~TYPE_MASK);
    }

    /// 创建非法Id
    INLINE_FUNCTION
    static
    TypedResourceId
    invalid_typed_id ()
    {
        return TypedResourceId{};
    }

    INLINE_FUNCTION
    TypedResourceId ()
    :
        value(make_typed_value(INVALID_TYPE, (ValueTypeT)(-1)))
    {

    }

    INLINE_FUNCTION
    explicit
    TypedResourceId (
        const ValueTypeT id_value)
    :
        value(make_typed_value(Type, id_value))
    {

    }

    INLINE_FUNCTION
    bool
    is_valid () const
    {
        return type() == Type;
    }

    INLINE_FUNCTION
    bool
    is_not_valid () const
    {
        return type() != Type;
    }

    INLINE_FUNCTION
    bool
    operator== (
        const TypedResourceId other) const
    {
        return value == other.value;
    }

    INLINE_FUNCTION
    bool
    operator!= (
        const TypedResourceId other) const
    {
        return value != other.value;
    }

    /// 获取内嵌的Type
    INLINE_FUNCTION
    uint8_t
    type () const
    {
        return (uint8_t)((value & TYPE_MASK) >> TYPE_SHIFT);
    }

    /// 获取无内嵌Type类型的Id的数值
    INLINE_FUNCTION
    ValueTypeT
    id () const
    {
        return value & ~TYPE_MASK;
    }

    /// 移至下一个Id; Id耗尽后回绕至0
    INLINE_FUNCTION
    void
    next_id ()
    {
        const ValueTypeT current_id = id();
        const bool is_maximal_id = current_id == ~TYPE_MASK;
        value = make_typed_value(Type, is_maximal_id ? 0 : current_id + 1);
    }

    /// 复位Id
    INLINE_FUNCTION
    void
    reset ()
    {
        value = make_typed_value(Type, 0);
    }
};
