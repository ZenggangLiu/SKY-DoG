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
    Created:  9/08/26  @  10:29 PM
    FileName: RenderTextureId.hpp @ RedSoUL Project
    History:
             - created by: 9/08/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint32_t,...
/// Library headers
#include "Common/CommonDefines.hpp" /// FOUR_CC
#include "Hashing/RuntimeHash.hpp"
#include "Hashing/StaticStringHash.hpp"


#if !defined(RENDER_TEXTURE_HASH)

    /// 计算Hash时使用的32位Seed
    #define RENDER_TEXTURE_HASH_SEED (FOUR_CC_32('R', 'N', 'D', 'T'))

    #if defined(USE_64BITS_ID) /// 使用64位Id类型
        /// 计算Hash值的函数
        #define RUNTIME_RENDER_TEXTURE_HASH(unique_name) \
            RUNTIME_STRING_HASH_64(RENDER_TEXTURE_HASH_SEED, unique_name)
        #define STATIC_RENDER_TEXTURE_HASH(unique_name)  \
            STATIC_STRING_HASH_64(RENDER_TEXTURE_HASH_SEED, unique_name)

        /// Texture Id类型
        typedef uint64_t RenderTextureIdT;

    #else
        /// 计算Hash值的函数
        #define RUNTIME_RENDER_TEXTURE_HASH(unique_name) \
            RUNTIME_STRING_HASH_32(RENDER_TEXTURE_HASH_SEED, unique_name)
        #define STATIC_RENDER_TEXTURE_HASH(unique_name)  \
            STATIC_STRING_HASH_32(RENDER_TEXTURE_HASH_SEED, unique_name)

        /// Texture Id类型
        typedef uint32_t RenderTextureIdT;
    #endif /// defined(USE_64BITS_ID)

        /// 非法Texture Id
    #define INVALID_RENDER_TEXTURE_ID ((RenderTextureIdT)(-1))

#endif /// !defined(RENDER_TEXTURE_HASH)
