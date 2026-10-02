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
    Created:  10/02/26  @  10:33 PM
    FileName: CommonDefines.hpp @ RedSoUL Project
    History:
             - created by: 10/02/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint32_t,...
/// Library headers
#include "Common/CompilerDefines.hpp" /// BUILD_MODE
#include "Common/PlatformDefines.hpp" /// OS_TYPE


/// 判断指定size是否为2的N次幂
#if !defined(IS_POWER_OF_TWO)
#define IS_POWER_OF_TWO(size) \
    ((uint64_t)size && ((uint64_t)size & ((uint64_t)size - 1)) == 0)
#endif /// !defined(IS_POWER_OF_TWO)


/// 获得类型 T 中指定成员变量的64位偏移
/// 使用方式:
/// class T
/// {
/// public:
///     T() : iVal(0xC00DDEC0) {}
///     int32_t  iVal; /// 4字节
///     uint8_t  cVal; /// 4字节: 1字节 + 3字节Padding
///     float    fVal; /// 4字节
///     uint8_t  var;  /// 4字节: 1字节 + 3字节Padding
/// };
///
/// MEMBER_OFFSET(T, var) 返回 12(即: var的偏移)
#if !defined(MEMBER_OFFSET)
    /// NOTE: 删除使用__builtin_offsetof(T, member), 由于此时T无法使用继承
    #define MEMBER_OFFSET(T, member) ((uint64_t)&(((T*)0)->member))
#endif /// !defined(MEMBER_OFFSET)


/// 获取静态数组的长度(元素个数)
#if !defined(ARRAY_LENGTH)
/// 我们定义N个函数参考的数组
/// 使用如下方法定义一个对char[N]参考的函数: a function referring to a char[N]
/// 使用如下方法定义一个对char[N]参考:
/// - char(&reference) [N]: reference is a reference to char[N]
/// NOTE:
/// - if the array has overloaded operator[], it will change the expected behavior
    template < typename T, uint32_t N > char(&__GetArrayLength__(const T(&)[N]))[N];
/// 我们使用 sizeof 操作符来获得静态数组的成员个数
/// NOTE:
/// sizeof 不会真正计算（evaluate）指定的表达式: 此处不会真正调用 __GetArrayLength__ 函数
    #define ARRAY_LENGTH(T) (sizeof(__GetArrayLength__(T)))
    /// #define ARRAY_LENGTH(array) (sizeof(array) / sizeof(array[0]))
#endif  /// !defined(ARRAY_LENGTH)


/// 链接Token: COMBINE(t1, t2) --> t1t2
#if !defined(COMBINE)
/// NOTE: 由于 ## 将抑制对于宏的替代:
/// 如果我们直接定义 #define COMBINE(x, y) x ## y
/// x 与 y 将不会进行宏扩展
    #define  COMBINE(token1, token2) _TOKEN_COMBINE(token1, token2)
    #define _COMBINE(token1, token2)  token1 ## token2
#endif /// !defined(COMBINE)


/// 将SOMETHING字符串化: STRINGIGY(SOMETHING) --> "SOMETHING"
#if !defined(STRINGIFY)
    #define  STRINGIFY(SOMETHING) _STRINGIFY(SOMETHING)
    #define _STRINGIFY(SOMETHING)  #SOMETHING
#endif /// !defined(STRINGIFY)


#if !defined(FOUR_CC)
/// 产生一个32位FourCC数值:
/// 描述C0, C1, C2, C3一个32位整数(整数描述顺序C3,C2,C1,C0)
/// NOTE:
/// - 产生的数值为: 0xC3_C2_C1_C0
/// - 存储的字符序列: C0 C1 C2 C3: 如果使用Littel Endianess存入文件
    #define FOUR_CC_32(c0, c1, c2, c3) \
        (((uint32_t)(c3) << 24) | ((uint32_t)(c2) << 16) | \
         ((uint32_t)(c1) << 8 ) | ((uint32_t)(c0) << 0 ))

/// 产生一个64位FourCC数值:
/// 描述C0, C1, C2, C3, C4, C5, C6, C7一个64位整数(整数描述顺序C7,C6,C5,C4,C3,C2,C1,C0)
/// NOTE:
/// - 产生的数值为: 0xC7_C6_C5_C4_C3_C2_C1_C0
/// - 存储的字符序列: C0 C1 C2 C3 C4 C5 C6 C7 : 如果使用Littel Endianess存入文件
    #define FOUR_CC_64(c0, c1, c2, c3, c4, c5, c6, c7) \
        (((uint64_t)(c7) << 56) | ((uint64_t)(c6) << 48) | \
         ((uint64_t)(c5) << 40) | ((uint64_t)(c4) << 32) | \
         ((uint64_t)(c3) << 24) | ((uint64_t)(c2) << 16) | \
         ((uint64_t)(c1) << 8 ) | ((uint64_t)(c0) << 0 ))
#endif /// !defined(FOUR_CC)


/// 将指定的数值调整到指定的Alignment的倍数
///
/// 例如:
/// ALIGN_UP(1, 4)          --> 4
/// ALIGN_UP(4, 4)          --> 4
/// ALIGN_UP(5, 4)          --> 8
/// ALIGN_UP(0xFFFF0001, 4) --> 0xFFFF0004
/// ALIGN_UP(0xFFFF0002, 4) --> 0xFFFF0004
/// ALIGN_UP(0xFFFF0003, 4) --> 0xFFFF0004
/// ALIGN_UP(0xFFFF0004, 4) --> 0xFFFF0004
/// ALIGN_UP(0xFFFF0005, 4) --> 0xFFFF0008
#if !defined(ALIGN_UP)
#define ALIGN_UP(value, alignment) (((value) + ((alignment) - 1)) & (~((alignment) - 1)))
#endif /// !defined(ALIGN_UP)


/// Inlined function
#if !defined(INLINE_FUNCTION)
    /// debug build mode
    #if (BUILD_MODE == DEBUG_BUILD_MODE)
        #define INLINE_FUNCTION inline
    /// release build mode
    #else
        #if (OS_TYPE == OS_TYPE_WIN && defined(_MSC_VER))
        /// Windows using MSVC
            #define INLINE_FUNCTION __forceinline
        #else
        /// others
            #define INLINE_FUNCTION inline __attribute__((always_inline))
        #endif /// (OS_TYPE == OS_TYPE_WIN && defined(_MSC_VER))
    #endif /// (BUILD_MODE == DEBUG_BUILD_MODE)
#endif /// !defined(INLINE_FUNCTION)


/// Unused variable
#if !defined(UNUSED_VARIABLE)
    #define UNUSED_VARIABLE(v) ((void)v)
#endif /// !define(UNUSED_VARIABLE)


/// Packed struct
#if !defined(PACKED_STRUCT)
    #if (OS_TYPE == OS_TYPE_WIN && defined(_MSC_VER))
        #define PACKED_STRUCT(name, ...) \
            __pragma(pack(push, 1)) struct name { __VA_ARGS__ }; __pragma(pack(pop))
    #else
        #define PACKED_STRUCT(name, ...) \
            struct __attribute__((packed)) name { __VA_ARGS__ };
    #endif /// (OS_TYPE == OS_TYPE_WIN && defined(_MSC_VER))
#endif /// !defined(PACKED_STRUCT)


/// 定义16位RedSoUL文件版本号
#if !defined(MAKE_FILE_VERSION)
#define MAKE_FILE_VERSION(major_version, minior_version) \
    (((uint16_t)major_version << 8) | (uint16_t)minior_version)
#endif /// !defined(MAKE_FILE_VERSION)


/// 创建一个Built-In使用的特殊名称
#if !defined(GENERATE_BUILTIN_NAME)
#define GENERATE_BUILTIN_NAME(unique_name) "__/::@" unique_name "@::\\__"
#endif /// !defined(GENERATE_BUILTIN_NAME)
