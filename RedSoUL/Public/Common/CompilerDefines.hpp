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
    Created:  11/02/26  @  3:51 PM
    FileName: CompilerDefines.hpp @ RedSoUL Project
    History:
             - created by: 11/02/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// C++11 性能支持检测
#if !defined(CXX_STANDARD_VER)

    /// Microsoft
    /// - VS2015 defines __cplusplus as 199711L,
    ///   which has support on c++14 in addition to c++11
    #if defined(_MSC_VER) && _MSC_VER >= 1900
        /// 所以，从VS2015起，我们认为 C++11 完全支持: MSVC++ 14.0
        ///     _MSC_VER == 1900 (Visual Studio 2015)
        ///we consider starting from vs 2015 a full support on c++ 11
        #define CXX_STANDARD_VER 2011
        #define CXX_STANDARD_VER_TEXT "C++ 2011"

    #else
        /// other compilers
        #if defined(__cplusplus)
            /// C++ 98标准
            #if __cplusplus == 199711L
                #define CXX_STANDARD_VER 1998
                #define CXX_STANDARD_VER_TEXT "C++ 1998"
            /// C++ 2011标准
            #elif __cplusplus == 201103L
                #define CXX_STANDARD_VER 2011
                #define CXX_STANDARD_VER_TEXT "C++ 2011"
            /// C++ 2014标准
            #elif __cplusplus == 201402L
                #define CXX_STANDARD_VER 2014
                #define CXX_STANDARD_VER_TEXT "C++ 2014"
            /// C++ 2017标准
            #elif __cplusplus == 201703L
                #define CXX_STANDARD_VER 2017
                #define CXX_STANDARD_VER_TEXT "C++ 2017"
            /// C++ 2020标准
            #elif __cplusplus == 201707L
                #define CXX_STANDARD_VER 2020
                #define CXX_STANDARD_VER_TEXT "C++ 2020"
            #else
                #error Unknown C++ standard
            #endif /// __cplusplus == 199711L

        #else
           /// 如果此文件使用Objective-C进行编译我们将使用C++1998标准
            #define CXX_STANDARD_VER 1998
            #define CXX_STANDARD_VER_TEXT "C++ 1998"
        #endif /// defined(__cplusplus)

    #endif /// defined(_MSC_VER) && _MSC_VER >= 1900

#endif /// !defined(CXX_STANDARD_VER)


/// Exception 使用情况检测
#if !defined(USE_EXCEPTION)

    #if defined(_MSC_VER)
        /// Microsoft
        #if defined(_CPPUNWIND) && (_CPPUNWIND == 1)
            #define USE_EXCEPTION 1
        #else
            #define USE_EXCEPTION 0
        #endif
    #else
        /// Apple OR Linux
        #if defined(__cpp_exceptions)
            #define USE_EXCEPTION 1
        #else
            #define USE_EXCEPTION 0
        #endif /// defined(__cpp_exceptions)
    #endif /// defined(_MSC_VER)

#endif /// !defined(USE_EXCEPTION)


/// RTTI 使用情况监测
#if !defined(USE_RTTI)

    #if defined(_MSC_VER)
        /// Microsoft
        #if defined(_CPPRTTI) && (_CPPRTTI == 1)
            #define USE_RTTI 1
        #else
            #define USE_RTTI 0
        #endif
    #else
        /// Apple OR Linux
        #if defined(__cpp_rtti)
            #define USE_RTTI 1
        #else
            #define USE_RTTI 0
        #endif /// defined(__cpp_rtti)
    #endif /// defined(_MSC_VER)

#endif /// !defined(USE_RTTI)


/// Build mode 检测
#if !defined(BUILD_MODE)
    #define DEBUG_BUILD_MODE    1
    #define RELEASE_BUILD_MODE  0

    /// Debug mode
    #if !defined(NDEBUG) && (defined(DEBUG) || defined(_DEBUG))
        #define BUILD_MODE DEBUG_BUILD_MODE
        #define BUILD_MODE_TEXT "Debug build"

    /// Release mode
    #else
        #define BUILD_MODE RELEASE_BUILD_MODE
        #define BUILD_MODE_TEXT "Release build"
    #endif /// !defined(NDEBUG) && (defined(DEBUG) || defined(_DEBUG))
#endif /// !defined(BUILD_MODE)


// --- 编译器配置检测 --- //
/// C++11 support
#if CXX_STANDARD_VER < 2011
    #error We need C+11 compiler
#endif /// CXX_STANDARD_VER < 2011


/// 是否使用异常处理
#if (USE_EXCEPTION == 0)
    #if defined(__APPLE__)
        #error Please set "Enabled C++ Exceptions" to "Yes", because STL needs the exception
    #else
        #error Please enable exceptions, because STL needs the exception
    #endif /// defined(__APPLE__)
#endif /// (USE_EXCEPTION == 0)


/// 是否使用 RTTI
#if (USE_RTTI == 1)
    #if defined(__APPLE__)
        #error Please set "Enabled C++ Runtime Types" to "No"
    #else
        #error Please disable RTTI
    #endif /// defined(__APPLE__)
#endif /// #if (USE_RTTI == 1)
