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
    Created:  5/03/26  @  9:20 PM
    FileName: RuntimeHash.hpp @ RedSoUL Project
    History:
             - created by: 5/03/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <cstring>  /// std::strlen
#include <stdint.h> /// uint32_t,...


struct RuntimeHash
{
    /// 为指定长度的数据计算一个32位Hash
    static
    uint32_t
    hash32 (
        const uint32_t        seed,
        const uint8_t * const data,
        const uint64_t        length);

    /// 为指定长度的数据计算一个64位Hash
    static
    uint64_t
    hash64 (
        const uint32_t        seed,
        const uint8_t * const data,
        const uint64_t        length);
};


#if !defined(RUNTIME_HASH)
    #define RUNTIME_HASH_32(seed, data, length)  \
        RuntimeHash::hash32(seed, (const uint8_t*)data, (uint64_t)length)
    #define RUNTIME_HASH_64(seed, data, length)  \
        RuntimeHash::hash64(seed, (const uint8_t*)data, (uint64_t)length)
    #define RUNTIME_STRING_HASH_32(seed, string) \
        RUNTIME_HASH_32(seed, string, std::strlen((const char*)string))
    #define RUNTIME_STRING_HASH_64(seed, string) \
        RUNTIME_HASH_64(seed, string, std::strlen((const char*)string))
#endif /// !defined(RUNTIME_HASH)
