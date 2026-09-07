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
    FileName: TransformMarker.hpp @ RedSoUL Project
    History:
             - created by: 16/05/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <vector>
/// Library headers
#include "DataType/Float3D.hpp"
#include "DataType/Quaternion.hpp"
#include "DataType/Matrix3x4.hpp"
#include "Message/MessageDefines.hpp"
#include "SceneGraph/ObjectMarker.hpp"
#include "SceneGraph/TransformType.hpp"


class SceneObject;
class TransformData;
class TransformMarker;


/// 空间变换属性(维护SceneObject层级关系的属性)
class TransformMarker : public ObjectMarker
{
public:
    /// 获取Transform属性的消息Id
    static
    MessageId
    message_id ();

    // --- HIERARCHY --- //
    /// 获取父节点变换属性
    /// NOTE: COULD BE NULL!!
    const TransformMarker *
    father () const;

    TransformMarker *
    father ();

    /// 设置父节点
    ///
    /// @param[in]  father
    ///      父节点的空间变换(如果没有设置为nullptr)
    void
    set_father (
        TransformMarker * const father = nullptr);

    /// 获取子节点的个数
    uint32_t
    kinder_count () const;

    /// 获取指定索引的子节点的变换属性
    ///
    /// @param[in]  index
    ///      零开始的子节点索引
    ///      NOTE: COULD BE NULL!!
    const TransformMarker *
    kinder_at (
        const uint32_t index) const;

    TransformMarker *
    kinder_at (
        const uint32_t index);


    // --- TRANSFORM --- //
    /// 将本地空间的点变换到世界空间
    float_3
    local_point_to_world_space (
        const float_3 local_point) const;

    /// 将世界空间的点变换到本地空间
    float_3
    world_point_to_local_space (
        const float_3 world_point) const;

    /// 获取当前的世界变换矩阵
    const matrix_3x4 &
    local_to_world_transform () const;

    /// 获取当前世界的位移
    float_3
    world_position () const;

    /// 获取当前世界右方向量(单位向量|Rightward/X轴)
    float_3
    world_rightward_vector () const;

    /// 获取当前世界上方向量(单位向量|Upward/Y轴)
    float_3
    world_upward_vector () const;

    /// 获取当前世界朝向(单位向量|Forward/Z轴)
    float_3
    world_forward_vector () const;

    /// 获取当前本地位移
    float_3
    local_position () const;

    /// 获取当前本地Pitch(X轴)/俯仰角度(以度来衡量)
    float
    local_pitch () const;

    /// 获取当前本地Yaw(Y轴)/偏航角度(以度来衡量)
    float
    local_yaw () const;

    /// 获取当前本地Roll(Z轴)/翻滚角度(以度来衡量)
    float
    local_roll () const;

    /// 获取当前本地旋转
    /// NOTE:
    /// - 旋转顺序: Roll(Z轴), Pitch(X轴), Yaw(Y轴)
    quaternion
    local_rotation () const;

    float_3
    local_scaling () const;

    /// 设置世界朝向
    ///
    /// @param[in]  world_point
    ///     世界空间中的观察点
    /// NOTE: 将绕着当前的UP轴旋转
    void
    set_world_forward_vector (
        const float_3 world_point);

    /// 设置本地位移
    void
    set_local_position (
        const float_3 position);

    /// 设置本地俯仰/Pitch(X轴)角度(以度来衡量)
    void
    set_local_pitch (
        const float angle_degs);

    /// 设置本地偏航/Yaw(Y轴)角度(以度来衡量)
    void
    set_local_yaw (
        const float angle_degs);

    /// 设置本地翻滚/Roll(Z轴)角度(以度来衡量)
    void
    set_local_roll (
        const float angle_degs);

    void
    set_local_rotation (
        const quaternion rotation);

    void
    set_local_scaling (
        const float_3 scaling);


private:
    friend class SceneObject;

    typedef ObjectMarker SuperT;

    /// 创建函数
    ///
    /// @param[in]  marker_owner
    ///     Marker的所有者
    static
    ObjectMarker *
    create (
        SceneObject & marker_owner);

    /// 销毁函数
    ///
    /// @param[in,out] marker_object
    ///     Marker实例。设置为nullptr, 如果销毁成功
    /// @return
    ///     True,  如果销毁成功
    ///     False, 如果销毁失败
    static
    bool
    destroy (
        ObjectMarker * & marker_object);

    TransformMarker (
        SceneObject &           marker_owner,
        TransformMarker * const father_transform);

    ~TransformMarker ();

    TransformMarker (
        const TransformMarker &) = delete;
    TransformMarker & operator = (
        const TransformMarker &) = delete;

    /// 添加/Attach指定的子节点
    ///
    /// @param[in]  new_kinder
    ///     子节点的参考
    void
    attach (
        TransformMarker & new_kinder);

    /// 移除/Detach指定的子节点
    ///
    /// @param[in]  old_kinder
    ///     子节点的参考
    void
    detach (
        TransformMarker & old_kinder);

    /// 设置Cached世界转换矩阵Dirty
    void
    set_cache_dirty ();

    /// 设置旋转类型(根据当前Euler角的数值)
    void
    set_rotation_type ();

private:
    typedef std::vector<TransformMarker*> KinderListT;

    /// 起始的Marker个数
    static constexpr uint16_t INIT_TRANSFORM_MARKER_COUNT = 100;

    /// Marker类型信息
    static const MarkerTypeInfo ms_type_info;
    /// 消息Id
    static const MessageId      ms_message_id;

    // --- CACHED WORLD TRANSFORM --- //
    /// 世界转换矩阵: 本地 --> 世界
    mutable matrix_3x4    m_world_transform;

    // --- HIERARCHY --- //
    TransformMarker *     m_father_transform;
    KinderListT           m_kinder_list;

    // --- LOCAL TRANSFORM --- //
    TransformData * const m_transform_data;
    TransformType         m_transform_type;

    // --- DIRTY FLAG --- //
    /// 标记是否世界变换矩阵失效(Local --> World)
    mutable bool          m_is_world_transform_dirty;
};
