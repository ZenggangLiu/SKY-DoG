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
    Created:  21/03/26  @  10:17 PM
    FileName: Float3D.hpp @ RedSoUL Project
    History:
             - created by: 21/03/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint32_t
#include <type_traits>


/// 三维向量
///
/// NOTE:
/// float_3定义为POD/Aggregate类型: 16位字节对齐, 以支持SIMD运算
/// - NO constructor, NO copy constructor, NO operator=(),
///   NO protect/private data, NO base class, NO virtual functions
/// - 缺省的opeartor=()将使用std::memcpy()逐字节复制:
///   例如:
///   float_3 source;
///   float_3 copy = source; <-- 使用std::memcpy()进行复制, 内部使用SSE
///
struct alignas(16) float_3
{
    /// 纬度
    static constexpr uint8_t DIMENSION = 3;

    /// 全一向量
    static const float_3 ONE;       /// [  1,  1,  1 ]
    /// 全零向量
    static const float_3 ZERO;      /// [  0,  0,  0 ]

    /// 方向向量定义根据如下的左手系
    ///  ^ Y
    ///  |     Z
    ///  |    /
    ///  |   /
    ///  |  /
    ///  | /          X
    ///  +------------>
    /// 前向量
    static const float_3 FORWARD;   /// [  0,  0,  1 ]
    /// 后向量
    static const float_3 BACKWARD;  /// [  0,  0, -1 ]
    /// 左向量
    static const float_3 LEFT;      /// [ -1,  0,  0 ]
    /// 右向量
    static const float_3 RIGHT;     /// [  1,  0,  0 ]
    /// 上向量
    static const float_3 UP;        /// [  0,  1,  0 ]
    /// 下向量
    static const float_3 DOWN;      /// [  0, -1,  0 ]

    union
    {
        struct
        {
            float x;
            float y;
            float z;
            float __padding__;
        };
        float e[DIMENSION];
    };

    /// 使用指定的数值创建向量: [x, y, z]
    static
    float_3
    make (
        const float x,
        const float y,
        const float z);

    /// 创建一个随机单位向量
    static
    float_3
    make_random_unit_vec ();

    /// 检测当前向量是否与另一个向量vec相同(在容忍范围内是否相同)
    bool
    operator== (
        const float_3 vec) const;

    /// 检测当前向量是否与另一个向量vec不同(在容忍范围内是否不同)
    bool
    operator!= (
        const float_3 vec) const;

    /// 获得指定索引成员的数值
    ///
    /// 例如:
    /// const float_3 vec3d;
    /// const float fval = vec3d[1];
    float
    operator[] (
        const uint8_t idx) const;

    /// 设定指定索引成员
    ///
    /// 例如:
    /// float_3 vec3d;
    /// vec3d[1] = 3.14f;
    float &
    operator[] (
        const uint8_t idx);

    /// 计算当前向量的长度/Norm
    float
    length () const;

    /// 计算当前向量的长度的平方/Norm^2
    float
    length_squared () const;

    /// 判断当前向量是否与另一个向量vec同向
    bool
    same_direction_as (
        const float_3 vec) const;

    /// 判断当前向量是否与另一个向量vec垂直
    bool
    is_perpendicular_to (
        const float_3 vec) const;

    /// 判断当前向量是否与另一个向量vec平行
    bool
    is_parallel_to (
        const float_3 vec) const;

    /// 创建一个将当前向量沿着指定向量vec偏移后的向量
    /// c = THIS + vec
    float_3
    operator+ (
        const float_3 vec) const;

    /// 创建一个将当前向量沿着指定向量-vec偏移后的向量
    /// c = THIS - vec
    float_3
    operator- (
        const float_3 vec) const;

    /// 创建一个将当前向量均匀放大scale倍的向量
    /// c = THIS * scale
    float_3
    operator* (
        const float scale) const;

    /// 创建一个将当前向量均匀缩小scale倍的向量
    /// c = THIS * 1/scale
    float_3
    operator/ (
        const float scale) const;

    /// 计算当前向量与另一个向量vec的点积(Dot/Inner Product)
    /// - Dot(THIS, vec)
    float
    dot (
        const float_3 vec) const;

    /// 计算当前向量与另一个向量vec的叉积(Cross/Outer Product)
    /// - Cross(THIS, vec)
    /// THIS x vec
    ///    ^
    ///    |   / THIS
    ///    |  /
    ///    | /
    ///    |/
    ///    +---------> vec
    /// NOTE: 如果THIS与vec向量平行, 叉积为零向量[0, 0, 0]
    float_3
    cross (
        const float_3 vec) const;

    /// 计算将当前向量沿着Normal向量反射的向量
    ///
    ///   V   N   R
    ///    \  |  /
    ///     \ | /
    ///      \|/
    /// ------o------
    float_3
    reflected_vec (
        const float_3 normal) const;

    /// 计算将当前向量归一化后的向量
    float_3
    unified_vec () const;

    /// 计算与当前向量垂直的任意向量
    float_3
    perpendicular_vec ()const;

    /// 归一化当前向量
    void
    normalize ();
};
static_assert(std::is_trivial<float_3>::value &&
              std::is_standard_layout<float_3>::value,
              "float_3 is Not a POD type!!");
