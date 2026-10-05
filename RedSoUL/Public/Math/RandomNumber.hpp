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
    Created:  31/03/26  @  10:20 PM
    FileName: RandomNumber.hpp @ RedSoUL Project
    History:
             - created by: 31/03/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <limits>   /// std::numeric_limits
#include <stdint.h> /// uint32_t


/// 伪随机数发生器(高速度版本: 相对于std::rand()快出六倍)
/// 数值重复周期: 2^256 -1
///
class RandomNumber
{
public:
    using result_type = uint64_t;

    /// 获得最小的随机数
    static
    constexpr result_type
    min ()
    {
        return 0;
    }

    /// 获得最大的随机数
    static
    constexpr result_type
    max ()
    {
        return std::numeric_limits<uint64_t>::max();
    }

    /// 使用当前时间作为Seed来构建一个随机数发生器
    RandomNumber ();

    /// 使用指定的Seed来构建一个随机数发生器
    RandomNumber (
        const uint64_t seed);

    /// 获得下一个64位随机数(随机数范围在[0, MaxUInt64]之间)
    result_type
    operator() ();

    /// 获得下一个64位随机数(随机数范围在[0, MaxUInt64]之间)
    result_type
    next_uint ();

    /// 获得下一个在指定范围内的64位随机数(随机数范围在[min, max]之间)
    result_type
    next_uint_range (
        const uint64_t min,
        const uint64_t max);

    /// 获得下一个32浮点数位随机数(随机数范围在[0, 1)之间)
    /// NOTE: 不包含数值1
    float
    next_float ();


private:
    /// 初始化这个伪随机数发生器
    void
    initialize (
        const uint64_t seed);

    /// 256位状态
    uint64_t state0;
    uint64_t state1;
    uint64_t state2;
    uint64_t state3;
};
