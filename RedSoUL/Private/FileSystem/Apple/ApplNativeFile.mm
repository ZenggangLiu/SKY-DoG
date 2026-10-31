#if defined(__APPLE__)

/// System headers
#import  <Foundation/NSData.h>
#import  <Foundation/NSFileManager.h>
#import  <Foundation/NSString.h>
/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "FileSystem/Apple/ApplFileHandle.hpp"
#include "FileSystem/NativeDirectory.hpp"
/// Self header
#include "FileSystem/NativeFile.hpp"


/// 获取真正的文件句柄
#define REAL_FILE_HANDLE (((ApplFileHandle*)m_file_handle)->real_handle)


uint32_t
NativeFile::file_length (
    const char * const absolute_file_name)
{
    RUNTIME_ASSERT( absolute_file_name, "File name can not be NULL!!");
    RUNTIME_ASSERT(*absolute_file_name, "File name can not be empty!!");

    NativeFile file_io;
    file_io.open(absolute_file_name, AccessMode::READ_ONLY_ACCESS_MODE);
    if (file_io.is_opened())
    {
        const uint32_t io_length = file_io.file_length();
        file_io.close();

        return io_length;
    }
    else
    {
        return 0;
    }
}


bool
NativeFile::does_file_exist (
    const char * const absolute_file_name)
{
    RUNTIME_ASSERT( absolute_file_name, "File name can not be NULL!!");
    RUNTIME_ASSERT(*absolute_file_name, "File name can not be empty!!");

    @autoreleasepool
    {
        NSString * const file_name =
            [NSString stringWithUTF8String:absolute_file_name];
        if (file_name)
        {
            NSFileManager * const file_manager = [NSFileManager defaultManager];
            BOOL is_folder = NO;
            const BOOL does_exist =
                [file_manager fileExistsAtPath:file_name isDirectory:&is_folder];
            return (does_exist == YES) && (is_folder == NO);
        }
        else
        {
            return false;
        }
    }
}


bool
NativeFile::delete_file (
    const char * const absolute_file_name)
{
    RUNTIME_ASSERT( absolute_file_name, "File name can not be NULL!!");
    RUNTIME_ASSERT(*absolute_file_name, "File name can not be empty!!");

    /// 检查文件是否不存在
    if (does_file_exist(absolute_file_name) == false)
    {
        return true;
    }

    @autoreleasepool
    {
        NSString * const file_name =
            [NSString stringWithUTF8String:absolute_file_name];
        if (file_name)
        {
            NSFileManager * const file_manager = [NSFileManager defaultManager];
            const BOOL opcode = [file_manager removeItemAtPath:file_name error:nil];
            return opcode == YES;
        }
        else
        {
            return false;
        }
    }
}


NativeFile::NativeFile ()
:
    m_file_handle(nullptr),
    m_file_length(0),
    m_cursor_pos(0),
    m_is_file_opened(false)
{

}


NativeFile::~NativeFile ()
{
    close();
}


bool
NativeFile::open (
    const char * const absolute_file_name,
    const AccessMode   access_mode)
{
    RUNTIME_ASSERT( absolute_file_name, "File name can not be NULL!!");
    RUNTIME_ASSERT(*absolute_file_name, "File name can not be empty!!");

    @autoreleasepool
    {
        NSString * const file_name =
            [NSString stringWithUTF8String:absolute_file_name];
        if (file_name)
        {
            NSFileHandle * exp_file_handle = nil;
            switch (access_mode)
            {
                case AccessMode::READ_ONLY_ACCESS_MODE:
                {
                    /// 尝试打开指定文件
                    /// NOTE: 如果文件不存在返回nil
                    exp_file_handle =
                        [NSFileHandle fileHandleForReadingAtPath:file_name];
                    break;
                }

                case AccessMode::WRITE_ONLY_ACCESS_MODE:
                {
                    /// 确保其父目录存在
                    NSString * const folder_name =
                        [file_name stringByDeletingLastPathComponent];
                    /// 尝试创建父目录
                    const bool opcode =
                        NativeDirectory::create_folder(
                            [folder_name cStringUsingEncoding:NSUTF8StringEncoding]);
                    if (opcode == false)
                    {
                        break;
                    }

                    /// 如果文件不存在, 创建它
                    if (does_file_exist(absolute_file_name) == false)
                    {
                        NSFileManager * const file_mgr = [NSFileManager defaultManager];
                        const BOOL opcode = [file_mgr createFileAtPath:file_name
                                                              contents:nil
                                                            attributes:nil];
                        if (opcode == NO)
                        {
                            break;
                        }
                    }

                    /// 尝试打开指定文件
                    exp_file_handle =
                        [NSFileHandle fileHandleForWritingAtPath:file_name];
                    break;
                }

                case AccessMode::READ_WRITE_ACCESS_MODE:
                {
                    /// 尝试打开指定文件
                    /// NOTE: 如果文件不存在返回nil
                    exp_file_handle =
                        [NSFileHandle fileHandleForUpdatingAtPath:file_name];
                    break;
                }

                default:
                {
                    RUNTIME_ASSERT(false, "Unknown access mode!!");
                    return false;
                }
            }

            /// 成功打开指定文件
            if (exp_file_handle != nil)
            {
                /// 关闭当前打开的文件
                close();

                m_file_handle    = new ApplFileHandle;
                REAL_FILE_HANDLE = exp_file_handle;
                /// 获取文件长度: seekToEndOfFile()返回文件的长度
                m_file_length    = (uint32_t)[REAL_FILE_HANDLE seekToEndOfFile];
                /// 将读写头重新设置回文件头
                [REAL_FILE_HANDLE seekToFileOffset:0];
                m_cursor_pos     = 0;
                m_is_file_opened = true;
                return true;
            }
            /// 无法打开指定文件
            else
            {
                return false;
            }
        }
        /// 无法创建文件名
        else
        {
            return false;
        }
    }
 }


bool
NativeFile::is_opened () const
{
    return m_is_file_opened;
}


uint32_t
NativeFile::cursor_position () const
{
    return m_cursor_pos;
}


uint32_t
NativeFile::file_length () const
{
    return m_file_length;
}


bool
NativeFile::seek (
    const int32_t  cursor_pos,
    const SeekMode seek_mode)
{
    RUNTIME_ASSERT(m_is_file_opened, "The file is not opened!!");

    if (m_is_file_opened)
    {
        /// 用来保存Cursor相对于文件头的位置
        uint32_t absolute_cursor_pos;

        switch (seek_mode)
        {
            /// 相对于文件头部
            case SeekMode::FILE_BEGIN_SEEK_MODE:
            {
                /// 无法将读写头设置到文件头之前
                if (cursor_pos < 0)
                {
                    return false;
                }
                else
                {
                    absolute_cursor_pos = (uint32_t)cursor_pos;
                    break;
                }
            }

            /// 相对于当前位置
            case SeekMode::CURRENT_CURSOR_SEEK_MODE:
            {
                absolute_cursor_pos = (uint32_t)((int32_t)m_cursor_pos + cursor_pos);
                break;
            }

            /// 相对于文件尾
            case SeekMode::FILE_END_SEEK_MODE:
            {
                /// 无法将读写头设置到文件尾之后
                if (cursor_pos > 0)
                {
                    return false;
                }
                else
                {
                    /// 相对于文件尾计算期待位置
                    absolute_cursor_pos = (uint32_t)((int32_t)m_file_length + cursor_pos);
                    break;
                }
            }

            default:
            {
                RUNTIME_ASSERT(false, "Uknown seek mode!!");
                return false;
            }
        }
        RUNTIME_ASSERT(absolute_cursor_pos <= m_file_length,
                       "Cursor position is out of range!!");

        /// 移动读写头
        [REAL_FILE_HANDLE seekToFileOffset:absolute_cursor_pos];
        return true;
    }
    else
    {
        return false;
    }
}


int32_t
NativeFile::read (
    uint8_t * const buffer_start,
    const uint32_t  buffer_size,
    const uint32_t  buffer_offset,
    const uint32_t  read_count)
{
    RUNTIME_ASSERT(buffer_start, "Output buffer can not be NULL!!");
    RUNTIME_ASSERT(buffer_offset + read_count <= buffer_size,
                   "No place to store all read bytes!!");
    RUNTIME_ASSERT(m_is_file_opened, "The file is not opened!!");

    /// 可以保存足够的数据, 并且文件够大
    if (buffer_start      && (buffer_offset + read_count <= buffer_size) &&
        m_is_file_opened  && (m_cursor_pos  + read_count <= m_file_length))
    {
        /// 同步读人数据
        @autoreleasepool
        {
            NSData * const input_data_array =
                [REAL_FILE_HANDLE readDataOfLength:read_count];

            /// 检测是否失败
            if (input_data_array == nil)
            {
                return -1;
            }
            else
            {
                /// 复制读入的数据
                [input_data_array getBytes:buffer_start + buffer_offset
                                    length:read_count];

                /// 更新读写头位置
                m_cursor_pos += read_count;
                return (int32_t)input_data_array.length;
            }
        }
    }
    else
    {
        return -1;
    }
}


bool
NativeFile::write (
    const uint8_t * const buffer_start,
    const uint32_t        buffer_size,
    const uint32_t        buffer_offset,
    const uint32_t        write_count)
{
    RUNTIME_ASSERT(buffer_start, "Input buffer can not be NULL!!");
    RUNTIME_ASSERT(buffer_offset + write_count <= buffer_size,
                   "No enough data in the input buffer!!");
    RUNTIME_ASSERT(m_is_file_opened, "The file is not opened!!");

    /// 可以写出足够的数据
    if (buffer_start && (buffer_offset + write_count <= buffer_size) && m_is_file_opened)
    {
        @autoreleasepool
        {
            /// 创建输出数组
            NSData * const out_data_array =
                [NSData dataWithBytes:buffer_start + buffer_offset
                               length:write_count];

            /// 检测是否创建失败
            if (out_data_array == nil)
            {
                return false;
            }

            /// 同步写出数据
            [REAL_FILE_HANDLE writeData:out_data_array];

            /// 更新读写头位置
            m_cursor_pos  += write_count;
            /// 更新文件长度
            m_file_length += write_count;
            return true;
        }
    }
    else
    {
        return false;
    }
}


void
NativeFile::close ()
{
    /// 检查是否当前文件仍然打开
    if (m_is_file_opened)
    {
        /// 关闭文件并清除Handle
        [REAL_FILE_HANDLE closeFile];
        REAL_FILE_HANDLE = nil;
        delete m_file_handle;

        m_file_handle    = nullptr;
        m_file_length    = 0;
        m_cursor_pos     = 0;
        m_is_file_opened = false;
    }
}

#endif /// defined(__APPLE__)
