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
    Created:  16/04/26  @  8:53 PM
    FileName: NativeDirectory.hpp @ RedSoUL Project
    History:
             - created by: 16/04/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// 目录控制类
///
class NativeDirectory
{
public:
    /// 获取Cache目录名
    static
    const char *
    cache_folder ();

    /// 获取Document目录名(用户的可读写目录)
    ///
    static
    const char *
    document_folder ();

    /// 获取临时目录名
    static
    const char *
    temporary_folder ();

    /// 获取当前工作目录名
    static
    const char *
    working_folder ();

    /// 检测指定的目录是否存在
    ///
    /// @param[in]  absolute_folder_name
    ///      目录的绝对路径
    /// @return
    ///      True    如果目录存在
    ///      False   如果目录不存在
    static
    bool
    does_folder_exist (
        const char * const absolute_folder_name);

    /// 设定当前工作目录
    ///
    /// @param[in]  absolute_folder_name
    ///      当前工作目录的绝对路径
    /// @return
    ///      True    如果目录设定成功
    ///      False   如果目录设定失败
    static
    bool
    set_working_folder (
        const char * const absolute_folder_name);

    /// 创建指定目录
    ///
    /// @param[in]  absolute_folder_name
    ///      目录的绝对路径
    /// @return
    ///      True    如果目录创建成功
    ///      False   如果目录创建失败
    /// NOTE: 此函数会创建所有路径参考的中间目录
    static
    bool
    create_folder (
        const char * const absolute_folder_name);

    /// 删除指定的目录
    /// NOTE: 此函数将删除所有中间目录
    ///
    /// @param[in]  absolute_folder_name
    ///      目录的绝对路径
    /// @return
    ///      True    如果目录删除成功
    ///      False   如果目录删除失败
    static
    bool
    delete_folder (
        const char * const absolute_folder_name);
};
