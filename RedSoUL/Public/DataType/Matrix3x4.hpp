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
    Created:  31/03/26  @  10:14 PM
    FileName: Matrix3x4.hpp @ RedSoUL Project
    History:
             - created by: 31/03/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint32_t
/// Library headers
#include "DataType/Float3D.hpp"
#include "DataType/Float4D.hpp"
#include "DataType/Quaternion.hpp"


/// 三行四列矩阵(ROW Major)
///
/// NOTE:
/// matrix_3x4定义为POD/Aggregate类型: 16位字节对齐
/// - NO constructor, NO copy constructor, NO operator=(),
///   NO protect/private data, NO base class, NO virtual functions
/// - 缺省的opeartor=()将使用std::memcpy()逐字节复制:
///   例如:
///   matrix_3x4 source;
///   matrix_3x4 copy = source; <-- 使用std::memcpy()进行复制, 内部使用SSE
///
struct alignas(16) matrix_3x4
{
    static constexpr uint8_t ROW_COUNT    = 3;
    static constexpr uint8_t COLUMN_COUNT = 4;

    static const matrix_3x4 IDENTITY;

    union
    {
        struct
        {
            /// 第一行元素列表
            float e00, e01, e02, e03;
            /// 第二行元素列表
            float e10, e11, e12, e13;
            /// 第三行元素列表
            float e20, e21, e22, e23;
        };
        /// 所有元素列表
        float   e[ROW_COUNT * COLUMN_COUNT];
        /// 行向量列表
        float_4 rows[ROW_COUNT];
    };

    /// 创建移动矩阵
    ///
    /// @param[in]  translation
    ///     移动向量
    static
    matrix_3x4
    make_translation (
        const float_3 translation);

    /// 创建旋转矩阵
    ///
    /// @param[in]  rotation
    ///     旋转单位四元数
    static
    matrix_3x4
    make_rotation (
        const quaternion rotation);

    /// 创建Uniform放缩矩阵
    static
    matrix_3x4
    make_scaling (
        const float scaling);
    /// 创建放缩矩阵
    ///
    /// @param[in]  scaling
    ///     放缩向量
    static
    matrix_3x4
    make_scaling (
        const float_3 scaling);

    static
    matrix_3x4
    make_translation_rotation (
        const float_3    translation,
        const quaternion rotation);

    static
    matrix_3x4
    make_translation_scaling (
        const float_3 translation,
        const float   scaling);

    static
    matrix_3x4
    make_translation_scaling (
        const float_3 translation,
        const float_3 scaling);

    static
    matrix_3x4
    make_rotation_scaling (
        const quaternion rotation,
        const float      scaling);

    static
    matrix_3x4
    make_rotation_scaling (
        const quaternion rotation,
        const float_3    scaling);

    static
    matrix_3x4
    make_translation_rotation_scaling (
        const float_3    translation,
        const quaternion rotation,
        const float      scaling);

    static
    matrix_3x4
    make_translation_rotation_scaling (
        const float_3    translation,
        const quaternion rotation,
        const float_3    scaling);

    /// 检测当前矩阵是否与另一个矩阵matx相同
    bool
    operator== (
        const matrix_3x4 & matx) const;

    /// 检测当前矩阵是否与另一个矩阵matx不同
    bool
    operator!= (
        const matrix_3x4 & matx) const;

    /// 获得指定索引的行向量
    ///
    /// 例如:
    /// const matrix_3x4 matx;
    /// const float_4 row0_vec4d = matx[0];
    float_4
    operator[] (
        const uint8_t idx) const;

    /// 设定指定索引行向量
    ///
    /// 例如:
    /// matrix_3x4 matx;
    /// matx[0] = float_4::make(1, 2, 3, 4);
    float_4 &
    operator[] (
        const uint8_t idx);

    /// 计算当前矩阵与另一个矩阵M的乘机:
    /// THIS * M
    matrix_3x4
    operator* (
        const matrix_3x4 & matx) const;

    /// 计算当前旋转矩阵对应的四元数
    quaternion
    rotation () const;

    /// 使用当前矩阵对指定点P进行变换
    /// P' = M * P
    float_3
    transform_point (
        const float_3 pnt) const;
};
