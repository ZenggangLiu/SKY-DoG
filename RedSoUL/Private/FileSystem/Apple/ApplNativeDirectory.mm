#if defined(__APPLE__)

/// System headers
#import  <Foundation/NSBundle.h>
#import  <Foundation/NSFileManager.h>
#import  <Foundation/NSPathUtilities.h>
#import  <Foundation/NSString.h>
#include <string>
/// Library headers
#include "Assert/RuntimeAssert.hpp"
/// Self header
#include "FileSystem/NativeDirectory.hpp"


// MARK: == Helpers ==
static
std::string
fetch_cache_folder_name ()
{
    @autoreleasepool
    {
        /// 获得Cache目录列表
        /// NSSearchPathForDirectoriesInDomains创建目录查找路径列表
        NSArray * const cache_folder_list =
            NSSearchPathForDirectoriesInDomains(
                NSCachesDirectory, NSUserDomainMask, YES);

        /// 获取第一个Cache目录
        NSString * const cache_folder = [cache_folder_list objectAtIndex:0];
        return [cache_folder cStringUsingEncoding:NSUTF8StringEncoding];
    }
}


static
std::string
fetch_document_folder_name ()
{
    @autoreleasepool
    {
        /// 获得Document目录列表
        /// NSSearchPathForDirectoriesInDomains创建目录查找路径列表
        NSArray * const doc_folder_list =
            NSSearchPathForDirectoriesInDomains(
                NSDocumentDirectory, NSUserDomainMask, YES);

        /// 获得第一个Docuemnt目录
        NSString * const doc_folder = [doc_folder_list objectAtIndex:0];
        return [doc_folder cStringUsingEncoding:NSUTF8StringEncoding];
    }
}


static
std::string
fetch_temporary_folder_name ()
{
    @autoreleasepool
    {
        NSString * const temp_folder = NSTemporaryDirectory();
        return [temp_folder cStringUsingEncoding:NSUTF8StringEncoding];
    }
}


static
std::string
fetch_working_folder_name ()
{
    @autoreleasepool
    {
        NSFileManager * const file_manager = [NSFileManager defaultManager];
        /// 获得当前工作目录
        NSString * working_folder = [file_manager currentDirectoryPath];
        /// 检查当前工作目录是否为Root: "/"
        /// 如果是Root，我们将其指向Bundle目录
        if ([working_folder isEqualToString: @"/"] == YES)
        {
            working_folder = [[NSBundle mainBundle] bundlePath];
        }
        return [working_folder cStringUsingEncoding:NSUTF8StringEncoding];
    }
}



// MARK: == NativeDirectory ==
const char *
NativeDirectory::cache_folder ()
{
    static std::string CACHE_FOLDER_NAME = fetch_cache_folder_name();
    return CACHE_FOLDER_NAME.c_str();
}


const char *
NativeDirectory::document_folder ()
{
    static std::string DOCUMENT_FOLDER_NAME = fetch_document_folder_name();
    return DOCUMENT_FOLDER_NAME.c_str();
}


const char *
NativeDirectory::temporary_folder ()
{
    static std::string TEMPORARY_FOLDER_NAME = fetch_temporary_folder_name();
    return TEMPORARY_FOLDER_NAME.c_str();
}


const char *
NativeDirectory::working_folder ()
{
    static std::string WORKING_FOLDER_NAME = fetch_working_folder_name();
    return WORKING_FOLDER_NAME.c_str();
}


bool
NativeDirectory::does_folder_exist (
    const char * const absolute_folder_name)
{
    RUNTIME_ASSERT( absolute_folder_name, "Directory name can not be NULL!!");
    RUNTIME_ASSERT(*absolute_folder_name, "Directory name can not be empty!!");

    @autoreleasepool
    {
        NSString * const folder_name =
            [NSString stringWithUTF8String:absolute_folder_name];
        if (folder_name)
        {
            NSFileManager * const file_manager = [NSFileManager defaultManager];

            BOOL is_folder = NO;
            const BOOL does_exist =
                [file_manager fileExistsAtPath:folder_name isDirectory:&is_folder];
            return (does_exist == YES) && (is_folder == YES);
        }
        else
        {
            return false;
        }
    }
}


bool
NativeDirectory::set_working_folder (
    const char * const absolute_folder_name)
{
    RUNTIME_ASSERT( absolute_folder_name, "Path name can not be NULL!!");
    RUNTIME_ASSERT(*absolute_folder_name, "Path name can not be empty!!");

    @autoreleasepool
    {
        NSString * const folder_name =
            [NSString stringWithUTF8String:absolute_folder_name];
        if (folder_name)
        {
            NSFileManager * const file_manager = [NSFileManager defaultManager];
            const BOOL opcode =
                [file_manager changeCurrentDirectoryPath:folder_name];
            return opcode == YES;
        }
        else
        {
            return false;
        }
    }
}


bool
NativeDirectory::create_folder (
    const char * const absolute_folder_name)
{
    RUNTIME_ASSERT( absolute_folder_name, "Path name can not be NULL!!");
    RUNTIME_ASSERT(*absolute_folder_name, "Path name can not be empty!!");

    /// 检查指定的目录是否存在
    if (does_folder_exist(absolute_folder_name))
    {
        return true;
    }

    @autoreleasepool
    {
        NSString * const folder_name =
            [NSString stringWithUTF8String:absolute_folder_name];
        if (folder_name)
        {
            /// 递归创建目录
            NSFileManager * const file_manager = [NSFileManager defaultManager];
            const BOOL opcode =
                [file_manager createDirectoryAtPath:folder_name
                        withIntermediateDirectories:YES
                                         attributes:nil
                                              error:nil];
            return opcode == YES;
        }
        else
        {
            return false;
        }
    }
}


bool
NativeDirectory::delete_folder (
    const char * const absolute_folder_name)
{
    RUNTIME_ASSERT( absolute_folder_name, "Path name can not be NULL!!");
    RUNTIME_ASSERT(*absolute_folder_name, "Path name can not be empty!!");

    /// 检查指定的目录是否不存在
    if (does_folder_exist(absolute_folder_name) == false)
    {
        return true;
    }

    @autoreleasepool
    {
        NSString * const folder_name =
            [NSString stringWithUTF8String:absolute_folder_name];
        if (folder_name)
        {
            /// 递归删除这个目录
            NSFileManager * const file_manager = [NSFileManager defaultManager];
            const BOOL opcode =
                [file_manager removeItemAtPath:folder_name error:nil];
            return opcode == YES;
        }
        else
        {
            return false;
        }
    }
}

#endif /// defined(__APPLE__)
