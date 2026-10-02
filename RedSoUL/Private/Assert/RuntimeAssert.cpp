#include "Common/PlatformDefines.hpp" /// OS_TYPE
/// System headers
#if (OS_TYPE == OS_TYPE_WIN)
#define _WINSOCKAPI_
#include <Windows.h>
#undef _WINSOCKAPI_
#include <string>
#include <tchar.h>
#elif (OS_TYPE == OS_TYPE_LINUX)
#include <cassert>
#endif
#include <cstdio>   /// std::printf, std::snprintf, std::vsnprintf
#include <cstring>  /// std::strlen
#include <stdarg.h> /// va_list
/// Self header
#include "Assert/RuntimeAssert.hpp"


#if (BUILD_MODE == DEBUG_BUILD_MODE)
void
RuntimeAssert (
   const bool         condition,
   const char * const file_name,
   const uint32_t     line_number,
   const char * const format,
   ...)
{
    if (condition)
    {
        return;
    }
    else
    {
        static constexpr uint16_t MAX_FORMAT_LENGTH = 1024*2;

        va_list arg_list;
        va_start(arg_list, format);

        char buffer[MAX_FORMAT_LENGTH];
        std::snprintf(buffer, MAX_FORMAT_LENGTH,
                      "--- [ASSERT FAILED] ---\n"
                      "Message: %s\n"
                      "Location: %s: %u\n"
                      "-----------------------\n",
                      format, file_name, line_number);

        const uint16_t msgIdx = (uint16_t)(std::strlen(buffer) + 1 /* '0' */);
        std::vsnprintf(buffer + msgIdx, sizeof(buffer) - msgIdx, buffer, arg_list);

        va_end(arg_list);

        /// 输出信息
#if (OS_TYPE == OS_TYPE_WIN)
    #if defined(UNICODE)
        /// 转换: ANSI --> WCHAR
        const int32_t wchar_length =
            (int32_t)MultiByteToWideChar(CP_ACP, 0, buffer + msgIdx, -1, nullptr, 0);
        const std::wstring utf16_string(wchar_length, L'\0');

        MultiByteToWideChar(
            CP_ACP, 0, buffer + msgIdx, -1,
            const_cast<wchar_t*>(utf16_string.c_str()), wchar_length);
        OutputDebugString(utf16_string.data());
    #else
        OutputDebugString(buffer + msgIdx);
    #endif /// defined(UNICODE)
#endif /// (OS_TYPE == OS_TYPE_WIN)
        std::printf("%s", buffer + msgIdx);

        /// Debuger Trap
#if (OS_TYPE == OS_TYPE_WIN)
        DebugBreak();
#elif defined(__APPLE__)
        /// __builtin_trap():
        /// - debug mode:
        ///   debugger will stop at the line immediately before the __builtin_trap call
        /// - normal mode: the program will crash and generate a crash report.
        __builtin_trap();
#else
        assert(false);
#endif /// (OS_TYPE == OS_TYPE_WIN)
    }
}
#endif /// (BUILD_MODE == DEBUG_BUILD_MODE)
