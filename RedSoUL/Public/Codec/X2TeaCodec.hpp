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
    Created:  11/02/26  @  2:59 PM
    FileName: X2TeaCodec.hpp @ RedSoUL Project
    History:
             - created by: 11/02/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint32_t,...
/// Library headers
#include "Common/CommonDefines.hpp" /// ARRAY_LENGTH


struct XXTEA
{
    /// 编码指定数据中的内容
    ///
    /// @param[in] key
    ///      编码时使用的Key
    /// @param[in] key_length
    ///      Key的长度(字节数)
    ///      NOTE: 最长的Key为16字节(128位)
    /// @param[in] data_ptr
    ///      原始数据的指针
    /// @param[in] data_size
    ///      原始数据的长度(以uint32_t来衡量)
    ///      NOTE: 最短的数据长度为2
    ///
    static
    void
    encode (
        const uint8_t * const key,
        const uint32_t        key_length,
        uint32_t * const      data_ptr,
        const uint32_t        data_size);

    /// 解码指定数据中的内容
    ///
    /// @param[in] key
    ///      解码时使用的Key
    /// @param[in] key_length
    ///      Key的长度(字节数)
    ///      NOTE: 最长的Key为16字节(128位)
    /// @param[in] data_ptr
    ///      编码数据的指针
    /// @param[in] data_size
    ///      编码数据的长度(以uint32_t来衡量)
    ///      NOTE: 最短的数据长度为2
    ///
    static
    void
    decode (
        const uint8_t * const key,
        const uint32_t        key_length,
        uint32_t * const      data_ptr,
        const uint32_t        data_size);
};


#define XXTEA_ENCODE(key, key_length, data_ptr) \
    XXTEA::encode(key, key_length, data_ptr, ARRAY_LENGTH(data_ptr));
#define XXTEA_DECODE(key, key_length, data_ptr) \
    XXTEA::decode(key, key_length, data_ptr, ARRAY_LENGTH(data_ptr));
