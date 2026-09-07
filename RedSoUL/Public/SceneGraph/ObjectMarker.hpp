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
    Created:  16/05/26  @  6:50 PM
    FileName: ObjectMarker.hpp @ RedSoUL Project
    History:
             - created by: 16/05/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint32_t
/// Library headers
#include "Common/CommonDefines.hpp"   /// INLINE_FUNCTION
#include "Common/CompilerDefines.hpp" /// BUILD_MODE
#include "Text/StaticStringId.hpp"
#include "Text/StaticString.hpp"


class ObjectMarker;
class SceneObject;


/// 物体属性的创建函数
///
/// @param[in]  marker_owner
///     Marker的所有者
/// @return
///     新创建的ObjectMarker实例, 如果成功
///     nullptr, 如果失败
typedef ObjectMarker * (*CreateFuncPtr)(SceneObject & marker_owner);
/// 物体属性的销毁函数
///
/// @param[in]  marker_object
///     Marker的实例
typedef bool (*DestroyFuncPtr)(ObjectMarker * & marker_object);


/// 属性类型信息
class MarkerTypeInfo
{
public:
#if (BUILD_MODE == DEBUG_BUILD_MODE)
    MarkerTypeInfo (
        const char * const   marker_name,
        const CreateFuncPtr  create_func,
        const DestroyFuncPtr destroy_func,
        const bool           support_multi);
#else
    MarkerTypeInfo (
        const StaticStringIdT marker_name_id,
        const CreateFuncPtr   create_func,
        const DestroyFuncPtr  destroy_func,
        const bool            support_multi);
#endif

    /// 检查当前类型是否为指定类型或者其子类
    ///
    /// @param[in]  type_name_id
    ///     准备检查的类型名称Id
    bool
    isa (
        const StaticStringIdT type_name_id) const;

    bool
    is_abstract () const;

    /// 检查是否不支持创建多个实体
    bool
    not_support_multiple () const;

    /// 检查是否支持创建多个实体
    bool
    support_multiple () const;

    CreateFuncPtr
    create_function () const;

    DestroyFuncPtr
    destroy_function () const;

    StaticStringIdT
    marker_name_id () const;


private:
    /// Abstract类型: Create/Destroy Function为NULL
    const CreateFuncPtr   m_create_func;
    const DestroyFuncPtr  m_destroy_func;
#if (BUILD_MODE == DEBUG_BUILD_MODE)
    const StaticString    m_marker_name;
#else
    const StaticStringIdT m_marker_name_id;
#endif
    /// 标记是否支持创建多个实体
    const bool            m_support_multiple;
};


/// 物体属性基类
///
class ObjectMarker
{
protected:
    /// @param[in]  marker_name_id
    ///     Marker类型的名称。例如: TransformMarker
    ObjectMarker (
        SceneObject &         marker_owner,
        const StaticStringIdT marker_name_id);

    ~ObjectMarker ();

    ObjectMarker (
        const ObjectMarker &) = delete;
    ObjectMarker & operator = (
        const ObjectMarker &) = delete;

protected:
    friend class SceneObject;

    SceneObject &   m_marker_owner;
    StaticStringIdT m_marker_name_id;
};
