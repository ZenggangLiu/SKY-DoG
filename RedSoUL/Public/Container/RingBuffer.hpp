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
    Created:  21/02/26  @  3:47 PM
    FileName: RingBuffer.hpp @ RedSoUL Project
    History:
             - created by: 21/02/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <atomic>
#include <stdint.h> /// uint32_t,...
#include <string>
/// Libray headers
#include "Common/CommonDefines.hpp" /// INLINE_FUNCTION


/// Single Producer - Single Consumer Ring Buffer
///
/// 调用流程:
/// - Consumer: read_data(), read_data(), ..., read_data(), finish_read()
/// - Producer: save_data(), save_data(), ..., save_data(), finish_save()
///
class RingBuffer
{
public:
    /// 构建一个Ring Buffer
    ///
    /// @param[in]  exp_size
    ///     期望的Ring Buffer的长度(字节长度)
    ///     NOTE: 此长度将RoundUp到下一个内存页的倍数
    RingBuffer (
        const uint64_t exp_size);

    ~RingBuffer();


    // MARK: == CONSUMER THREAD ACCESS ONLY ==
    // +----------------------------------+ //
    // |   CONSUMER THREAD ACCESS ONLY    | //
    // +----------------------------------+ //

    /// 检查是否此Ring Buffer为空
    bool
    is_empty () const;

    /// Read数据
    ///
    /// @param[out] output
    ///     数据存入的变量
    template <typename T>
    INLINE_FUNCTION
    bool
    read_data (
        T & output);

    /// 标记Read操作完成
    void
    finish_read ();

    /// 复位内部Read状态
    void
    reset_read ();


    // MARK: == PRODUCER THREAD ACCESS ONLY ==
    // +----------------------------------+ //
    // |   PRODUCER THREAD ACCESS ONLY    | //
    // +----------------------------------+ //

    /// 检查是否此Ring Buffer满
    bool
    is_full () const;

    /// Save数据
    ///
    /// @param[in]  data
    ///     期待写出的数据
    template<typename T>
    INLINE_FUNCTION
    bool
    save_data (
        const T & data);

    /// 标记Save操作完成
    void
    finish_save ();

    /// 复位内部Save状态
    void
    reset_save ();


private:
    /// Ring Buffer读/写头的类型(NOTE: 按照Cache Line边界对齐(64 Bytes对齐)
#define AccessHeadT    alignas(64) std::atomic<uint64_t>
#define ConstUInt32T   const uint32_t
#define ConstUInt64T   const uint64_t
#define MutableUInt64T uint64_t

    /// 读入指定长度的数据
    ///
    /// @param[out] output_buffer_ptr
    ///     用来保存读入数据的输出Buffer的指针
    /// @param[in]  buffer_size_in_bytes
    ///     输出Buffer的长度(字节长度)
    /// @param[in]  exp_data_size_in_bytes
    ///     期待读入的数据长度(字节长度)
    /// @return
    ///     TRUE, 读入成功
    bool
    read_data_in_bytes (
        void * const   output_buffer_ptr,
        const uint64_t buffer_size_in_bytes,
        const uint64_t exp_data_size_in_bytes);

    /// 写出指定长度的数据
    ///
    /// @param[in]  input_buffer_ptr
    ///     写出的数据的输入Buffer的指针
    /// @param[in]  exp_data_size_in_bytes
    ///     期待写出的数据长度(字节长度)
    /// @return
    ///     TRUE, 写出成功
    bool
    save_data_in_bytes (
        const void * const input_buffer_ptr,
        const uint64_t     exp_data_size_in_bytes);

    /// UNIT TEST逻辑
#if defined(CATCH_PLATFORM_MAC) || defined(CATCH_PLATFORM_WINDOWS) || defined(CATCH_PLATFORM_LINUX)
public:
#else
private:
#endif
    // --- cached values --- //
    ConstUInt32T    m_vm_page_count;
    ConstUInt64T    m_buffer_size;
    ConstUInt64T    m_modulo_mask; /// 计算Modulo(%)使用的Mask
    // --- 数据Buffer --- //
    uint8_t * const m_data_buffer;
    // --- local values --- //
    /// 在完成整个读入操作前，最后的数据读入位置
    MutableUInt64T  m_data_read_pos; /// Consumer local
    /// 在完成整个存入操作前，最后的数据写入位置
    MutableUInt64T  m_data_save_pos; /// Producer local
    // --- saved data's head/tail --- //
    AccessHeadT     m_saved_data_head; /// Consumer updates
    AccessHeadT     m_saved_data_tail; /// Producer updates
};



// MARK: == TEMPLATED FUNCTION IMPLEMENTATION ==
//------------------------------------------------------------------------------- //
// TEMPLATED FUNCTION IMPLEMENTATION                                              //
//------------------------------------------------------------------------------- //

/// 通用readData函数: uint64_t, const char*, CustomType
template <typename T>
INLINE_FUNCTION
bool
RingBuffer::read_data (
    T & output_buffer_ref)
{
    return read_data_in_bytes(&output_buffer_ref, sizeof(T), sizeof(T));
}


/// c type string(const char*)专用readData函数
///
/// @param[in]  output_string_ref
///     将读入的字符串保存此std::string中
template<>
INLINE_FUNCTION
bool
RingBuffer::read_data (
    std::string & output_string_ref)
{
    /// 目前字符串的保存格式:
    /// +----------------------+-----------------+
    /// | String Length(Bytes) | String contents |
    /// +----------------------+-----------------+
    ///  16 Bits                std:strlen Bytes
    ///
    uint16_t string_lenght;
    bool is_success = read_data(string_lenght);
    if (is_success)
    {
        output_string_ref.resize(string_lenght);
        is_success &= read_data_in_bytes(const_cast<char*>(output_string_ref.data()), string_lenght, string_lenght);
    }
    return is_success;
}


/// 禁止将const char*作为输出目标。请使用std::string
template<>
INLINE_FUNCTION
bool
RingBuffer::read_data (const char *&);


/// 通用saveData函数: uint64_t, const char*, CustomType
template<typename T>
INLINE_FUNCTION
bool
RingBuffer::save_data (
    const T & data)
{
    return save_data_in_bytes(&data, sizeof(T));
}


/// 禁止直接使用rvalue: 例如函数名称
/// NOTE: 如果开启，saveData(const T&) 必须改为 --> saveData(T&)
/// template<typename T>
/// bool saveData(T&&) = delete;


/// c type string(const char*)专用saveData函数
template<>
INLINE_FUNCTION
bool
RingBuffer::save_data (
    const char * const & c_string_ref)
{
    /// 目前字符串的保存格式:
    /// +----------------------+-----------------+
    /// | String Length(Bytes) | String contents |
    /// +----------------------+-----------------+
    ///  16 Bits                std:strlen Bytes
    ///
    const uint16_t string_length = (uint16_t)std::strlen(c_string_ref);
    return save_data(string_length) && save_data_in_bytes(c_string_ref, string_length);
}


/// std::string专用saveData函数
template<>
INLINE_FUNCTION
bool
RingBuffer::save_data (
    const std::string & std_string_ref)
{
    return save_data(std_string_ref.c_str());
}
