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
    Created:  23/03/26  @  11:21 PM
    FileName: Quaternion.hpp @ RedSoUL Project
    History:
             - created by: 23/03/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <type_traits>
#include <stdint.h> /// uint32_t
/// Library headers
#include "DataType/AxisType.hpp"
#include "DataType/Float3D.hpp"
#include "DataType/Float4D.hpp"


/// 单位四元数: 描述任意围绕轴U的三维旋转
///
/// 成员: { Sin(ϕ/2)*U, Cos(ϕ/2) }, U为旋转轴，ϕ为旋转角度
/// 特性:
/// - Q与-Q描述相同旋转: { -Sin(ϕ/2)*U, -Cos(ϕ/2) } == { Sin(ϕ/2)*U, Cos(ϕ/2) }
///   因为:
///   { -Sin(ϕ/2)*U, -Cos(ϕ/2) } == { Sin(π + ϕ/2)*U, Cos(π + ϕ/2) }
///   即:
///           ϕ             ϕ               2π + ϕ           2π + ϕ
///   { -Sin(───)*U , -Cos(───) } == { Sin(────────)*U, Cos(────────) }
///           2             2                 2                 2
///   因此: -Q与Q具有相同旋转轴U, 但-Q的旋转角度ϕ':= 2π + ϕ
/// - Q与Q*: { <x, y, z>, w }与{ <-x, -y, -z>, w }表示相同旋转轴U, 但旋转相反
///   即: { -Sin(ϕ/2)*U, Cos(ϕ/2) } == { Sin(-ϕ/2)*U, Cos(-ϕ/2) }
///
/// NOTE:
/// quaternion定义为POD/Aggregate类型: 16位字节对齐
/// - NO constructor, NO copy constructor, NO operator=(),
///   NO protect/private data, NO base class, NO virtual functions
/// - 缺省的opeartor=()将使用std::memcpy()逐字节复制:
///   例如:
///   quaternion source;
///   quaternion copy = source; <-- 使用std::memcpy()进行复制, 内部使用SSE
///
struct alignas(16) quaternion
{
    /// 纬度
    static constexpr uint8_t DIMENSION = 4;

    /// 无旋转四元数
    static const quaternion IDENTITY; /// { Sin(0)*U, Cos(0) }

    union
    {
        struct
        {
            float x;
            float y;
            float z;
            float w;
        };
        float_4 xyzw;
        float   e[DIMENSION];
    };

    /// 创建一个绕指定轴旋转的四元数
    ///
    /// @param[in]  rot_axis
    ///      旋转轴
    /// @param[in]  rot_rads
    ///      旋转角度(以弧度衡量)
    static
    quaternion
    make (
        const AxisType rot_axis,
        const float    rot_rads);

    /// 创建一个绕指定轴旋转的四元数
    ///
    /// @param[in]   rot_axis
    ///      旋转轴
    /// @param[in]   rot_rads
    ///      旋转角度(以弧度衡量)
    static
    quaternion
    make (
        const float_3 rot_axis,
        const float   rot_rads);

    /// 创建一个描述从向量v1到向量v2的旋转的四元数
    ///
    /// @param[in] v1
    ///      起始方向
    /// @param[in] v2
    ///      终止方向
    static
    quaternion
    make(
        const float_3 v1,
        const float_3 v2);

    /// 使用归一化线性插值两个四元数Q0, Q1(Normalized Linear Interpolation)
    /// 插值系数T:
    /// 0            1
    /// Q0 --------> Q1
    ///
    /// NOTE:
    /// Nlerp的特性:
    /// - 沿着最短路径插值
    /// - 合成顺序无关(Commutative): 多个Pose混合, 它的结果不依赖与他们混合的顺序
    /// - 非均匀角速度(Non-Constant Angular Velocity)
    /// - 最小转矩(Minimal Torque)
    /// NOTE:
    /// - 当两个四元数夹角ω小于30度时, 不使用Slerp(球面插值):
    /// - 在通常的Animation系统中, 连续的需要插值的四元数间的夹角ω不大
    static
    quaternion
    nlerp (
        const quaternion quat0,
        const quaternion quat1,
        const float      time);

    /// 使用球面插值两个四元数Q0, Q1(Spherical Linear Interpolation)
    /// 插值系数T:
    /// 0            1
    /// Q0 --------> Q1
    ///
    /// NOTE:
    /// Slerp的特性:
    /// - 沿着最短路径插值
    /// - 依赖合成顺序
    /// - 均匀角速度(Constant Angular Velocity)
    /// - 最小转矩(Minimal Torque)
    static
    quaternion
    slerp (
        const quaternion quat0,
        const quaternion quat1,
        const float      time);

    /// 检测当前四元数是否与另一个四元数Q相同(即: 表示相同的旋转)
    bool
    operator== (
        const quaternion quat) const;

    /// 检测当前四元数是否与另一个四元数Q不同
    bool
    operator!= (
        const quaternion quat) const;

    /// 获取指定索引成员的数值
    ///
    /// 例如:
    /// const quaternion quat;
    /// const float fval = quat[1];
    float
    operator[] (
        const uint8_t idx) const;

    /// 设定指定索引成员
    ///
    /// 例如:
    /// quaternion quat;
    /// quat[1] = 3.14f;
    float &
    operator[] (
        const uint8_t idx);

    /// 计算当前四元数与另一个四元数R的乘积(复合旋转)
    /// C := THIS * R
    ///
    /// NOTE:
    /// 复合旋转为: 先使用R进行旋转, 再使用THIS旋转
    /// 因为: 对于合成旋转C := THIS * R, 如果作用在点P上:
    /// P' := (THIS * R) * P * (THIS * R)^-1
    ///    := THIS *  R * P * R^-1  * THIS^-1
    ///    := THIS * (R * P * R^-1) * THIS^-1
    quaternion
    operator* (
        const quaternion rot) const;

    /// 旋转指定向量/点
    float_3
    operator* (
        const float_3 vec) const;

    /// 获取旋转轴向量
    float_3
    rot_axis () const;

    /// 获取旋转角(以弧度衡量)
    float
    rot_rads () const;

    /// 计算Euler角(以弧度衡量)
    /// NOTE: 角度的排列顺序:
    /// - x轴: 俯仰/Pitch角
    /// - y轴: 偏航/Yaw  角
    /// - z轴: 翻滚/Roll 角
    float_3
    euler_angles ()const;

    /// 创建当前四元数的反四元数(即: 反旋转)
    quaternion
    inverse () const;
};
static_assert(std::is_trivial<quaternion>::value &&
              std::is_standard_layout<quaternion>::value,
              "quaternion is Not a POD type!!");
static_assert(sizeof(quaternion) == sizeof(float_4),
              "quaternion is in wrong size!!");
