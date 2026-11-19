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
    Created:  11/10/26  @  5:15 PM
    FileName: GeiSTAppDelegate+Private.hpp @ MoonGeiST Project
    History:
             - created by: 11/10/26: Zenggang LIU
                                                                                        
***************************************************************************************/



#pragma once


#import "Platform/GeiSTAppDelegate.hpp"


/// GeiSTAppDelegate其它Category共享的Private函数定义
/// 
@interface GeiSTAppDelegate () /// Class Extension: 作为Private接口使用
// --- Private函数 --- //

/// 加载Render资源。无缓存文件时创建内置资源
- (void) loadRenderResources;

/// 保存当前Render资源
- (void) saveRenderResources;

/// 释放当前Render资源
- (void) unloadRenderResources;
@end
