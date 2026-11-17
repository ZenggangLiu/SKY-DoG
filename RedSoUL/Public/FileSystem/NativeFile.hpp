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
    Created:  18/04/26  @  9:23 PM
    FileName: NativeFile.hpp @ RedSoUL Project
    History:
             - created by: 18/04/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h>  /// uint8_t, uint32_t,...


struct FileHandle;


/// 访问模式
enum class AccessMode : uint8_t
{
    /// 只读
    READ_ONLY_ACCESS_MODE,
    /// 只写
    WRITE_ONLY_ACCESS_MODE,

    /// 读写
    READ_WRITE_ACCESS_MODE,
};


/// SEEK模式
enum class SeekMode : uint8_t
{
    /// 相对于当前读写头位置
    CURRENT_CURSOR_SEEK_MODE,
    /// 相对于文件头位置
    FILE_BEGIN_SEEK_MODE,
    /// 相对于文件尾位置
    FILE_END_SEEK_MODE,
};


/// 文件控制类
///
/// 文件描述为:
///  文件头             最后字节   文件尾
/// +-----+-----+-----+-----+-----------+
/// | AAA | BBB | ... | EEE | EndOfFile |
/// +-----+-----+-----+-----+-----------+
///    0     1    ...   n-1
///          ^
///          |
///          +-- 读写头位置
///
class NativeFile
{
public:
    /// 获取指定文件的长度(字节数)
    static
    uint32_t
    file_length (
        const char * const absolute_file_name);

    /// 检测指定的文件是否存在
    ///
    /// @param[in]  absolute_file_name
    ///     绝对文件名称
    /// @return
    ///     True    如果文件存在
    ///     False   如果文件不存在
    static
    bool
    does_file_exist (
        const char * const absolute_file_name);

    /// 删除指定的文件
    ///
    /// @param[in]  absolute_file_name
    ///     绝对文件名称
    /// @return
    ///     True    如果文件删除成功
    ///     False   如果文件删除失败
    static
    bool
    delete_file (
        const char * const absolute_file_name);

    /// 构建一个空文件
    NativeFile ();

    ~NativeFile ();

    /// 打开指定文件
    ///
    /// @param[in]  absolute_file_name
    ///     绝对文件名称
    /// @param[in]  access_mode
    ///     打开方式
    /// @return
    ///     True    如果文件打开成功
    ///     False   如果文件打开失败
    bool
    open (
        const char * const absolute_file_name,
        const AccessMode   access_mode);

    /// 检查当前文件是否成功打开
    bool
    is_opened () const;

    /// 获得当前文件的读写头的位置: [0, 文件长度]
    uint32_t
    cursor_position () const;

    /// 获得当前文件的长度
    uint32_t
    file_length () const;

    /// 设定当前文件的读写头的位置
    ///
    /// @param[in]  cursor_pos
    ///     相对于SeekMode的读写头位置的偏移
    /// @param[in]  seek_mode
    ///     定义如何改变读写头的位置
    /// @return
    ///     True:  移动成功
    ///     False: 移动失败
    bool
    seek (
        const int32_t  cursor_pos,
        const SeekMode seek_mode);

    /// 读入指定字节数的数据
    ///
    /// @param[out] buffer_start
    ///     存入的缓存的起始地址
    /// @param[in]  buffer_size
    ///     缓存的大小
    /// @param[in]  buffer_offset
    ///     缓存中的起始位置
    /// @param[in]  read_count
    ///     希望读入的字节数
    /// @return
    ///     最终读入的字节数
    ///     >= 0: 读入成功
    ///     <  0: 读入失败
    int32_t
    read (
        uint8_t * const buffer_start,
        const uint32_t  buffer_size,
        const uint32_t  buffer_offset,
        const uint32_t  read_count);

    /// 写出指定字节数的数据
    ///
    /// @param[in]  buffer_start
    ///     数据缓存的起始地址
    /// @param[in]  buffer_size
    ///     缓存的大小
    /// @param[in]  buffer_offset
    ///     缓存中的起始位置
    /// @param[in]  write_count
    ///     希望写出的字节数
    /// @return
    ///     True:  写出成功
    ///     False: 写出失败
    bool
    write (
        const uint8_t * const buffer_start,
        const uint32_t        buffer_size,
        const uint32_t        buffer_offset,
        const uint32_t        write_count);

    /// 关闭当前文件
    void
    close ();


    /// UNIT TEST逻辑
#if defined(CATCH_PLATFORM_MAC) || defined(CATCH_PLATFORM_WINDOWS) || defined(CATCH_PLATFORM_LINUX)
public:
#else
private:
#endif

    /// 文件句柄
    FileHandle * m_file_handle;
    /// 文件长度
    uint32_t     m_file_length;
    /// 读写头位置
    uint32_t     m_cursor_pos;
    /// 文件是否成功打开
    bool         m_is_file_opened;
};
