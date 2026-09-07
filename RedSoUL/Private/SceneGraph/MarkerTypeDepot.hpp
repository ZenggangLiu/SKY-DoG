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
    Created:  22/05/26  @  11:36 PM
    FileName: MarkerTypeDepot.hpp @ RedSoUL Project
    History:
             - created by: 22/05/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <unordered_map>
/// Library headers
#include "Common/CommonDefines.hpp"   /// STRINGIFY
#include "Common/CompilerDefines.hpp" /// BUILD_MODE
#include "SceneGraph/ObjectMarker.hpp"
#include "Text/StaticStringId.hpp"


class MarkerTypeInfo;
class SceneObject;


#if (BUILD_MODE == DEBUG_BUILD_MODE)
    #define DEFINE_MARKER_TYPE_INFO( \
        marker_class, create_func, destroy_func, support_multi) \
    const MarkerTypeInfo marker_class::ms_type_info( \
        STRINGIFY(marker_class), create_func, destroy_func, support_multi);
#else
    #define DEFINE_MARKER_TYPE_INFO( \
        marker_class, create_func, destroy_func, support_multi) \
    const MarkerTypeInfo marker_class::ms_type_info( \
        STATIC_STRING_HASH( \
            STRINGIFY(marker_class)), create_func, destroy_func, support_multi);
#endif


/// 维护各种物体属性类型创建/销毁信息的仓库
///
class MarkerTypeDepot
{
public:
    /// 获得对此Depot的参考
    static
    MarkerTypeDepot &
    ref ();

    /// 获取指定名称的TypeInfo
    ///
    /// @return
    ///     nullptr, 如果无指定名称的Type
    const MarkerTypeInfo *
    type_info (
        const StaticStringIdT marker_name_id) const;

    /// 注册一个新的属性类型
    void
    register_type (
        const StaticStringIdT  marker_name_id,
        const MarkerTypeInfo & marker_type_info);

    /// 创建指定Id的属性
    ///
    /// @param[in]  marker_owner
    ///     Marker的所有者
    ObjectMarker *
    create_marker (
        SceneObject &         marker_owner,
        const StaticStringIdT marker_name_id);

    /// 销毁指定Id的属性实例
    bool
    destroy_marker (
        const StaticStringIdT marker_name_id,
        ObjectMarker * &      marker_object);


private:
    typedef std::unordered_map< StaticStringIdT,
                                const MarkerTypeInfo* > MarkerTypeTableT;

    /// NameId --> TypeInfo
    MarkerTypeTableT m_type_table;
};
