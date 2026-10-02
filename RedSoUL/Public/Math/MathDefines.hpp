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
    Created:  31/03/26  @  10:17 PM
    FileName: MathDefines.hpp @ RedSoUL Project
    History:
             - created by: 31/03/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint32_t,...


/// 最大的32位整数
#if !defined(MAXIMAL_UINT32)
#define MAXIMAL_UINT32               ((uint32_t)-1)
#endif


/// 最大的64位整数
#if !defined(MAXIMAL_UINT64)
#define MAXIMAL_UINT64               ((uint64_t)-1)
#endif


/// 浮点数相等的Tolerance
/// 由于相邻的两个32位浮点数的最小数值差为: 0.00000006: 6e-8
/// 所以我们使用0.000001: 1e-6
#if !defined(EPSILON)
#define EPSILON                      (1.0e-6f)
#endif


/// PI
#if !defined(ONE_PI)
#define ONE_PI                       (3.14159265358979f)
#endif


#if !defined(TWO_PI)
#define TWO_PI                       (2.0f * ONE_PI)
#endif


#if !defined(HALF_PI)
#define HALF_PI                      (ONE_PI / 2.0f)
#endif


/// 将角度转换为弧度
#if !defined(DEGREE_TO_RADIAN)
#define DEGREE_TO_RADIAN(degrees)    ((float)degrees * (ONE_PI / 180.0f))
#endif


/// 将弧度转换为角度
#if !defined(RADIAN_TO_DEGREE)
#define RADIAN_TO_DEGREE(radians)    ((float)radians * (180.0f / ONE_PI))
#endif


//------------------------------------------------------------------------------
// 32 BITS FLOATING POINT NUMBER
//------------------------------------------------------------------------------
// MARK: == 32位浮点数 ==

/// 32位最小的Normalized的浮点数: +2^-126
/// NOTE: 不可使用此数来确定数值的间隙:
/// 由于数值的分布间隙有Exponent来确定。
/// 例如: 数值1附近的间隔为1.19209290e-07， 而0.1附近的间隔为7.45058060e-09
/// 数值1的间隙16倍大
#if !defined(MINIMUM_POSITIVE_FLOAT)
// 0x00800000
#define MINIMUM_POSITIVE_FLOAT       1.175494351e-38f
#endif


/// 32位浮点数1的Bit Pattern
#if !defined(FLOAT_ONE_PATTERN)
#define FLOAT_ONE_PATTERN            0x3F800000
#endif


/// 32位最大的的浮点数
#if !defined(MAXIMAL_FLOAT)
/// 0x7F7FFFFF
#define MAXIMAL_FLOAT                3.402823466e+38f
#endif
