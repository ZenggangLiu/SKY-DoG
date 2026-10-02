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
    Created:  31/03/26  @  10:15 PM
    FileName: Matrix4x4.hpp @ RedSoUL Project
    History:
             - created by: 31/03/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <stdint.h> /// uint32_t
/// Library headers
#include "DataType/Float4D.hpp"


struct matrix_3x4;


/// 四行四列矩阵(ROW Major)
///
/// NOTE:
/// matrix_4x4定义为POD/Aggregate类型: 16位字节对齐
/// - NO constructor, NO copy constructor, NO operator=(),
///   NO protect/private data, NO base class, NO virtual functions
/// - 缺省的opeartor=()将使用std::memcpy()逐字节复制:
///   例如:
///   matrix_4x4 source;
///   matrix_4x4 copy = source; <-- 使用std::memcpy()进行复制, 内部使用SSE
///
struct alignas(16) matrix_4x4
{
    static constexpr uint8_t ROW_COUNT    = 4;
    static constexpr uint8_t COLUMN_COUNT = 4;

    static const matrix_4x4 IDENTITY;

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
            /// 第四行元素列表
            float e30, e31, e32, e33;
        };
        /// 所有元素列表
        float   e[ROW_COUNT * COLUMN_COUNT];
        /// 行向量列表
        float_4 rows[ROW_COUNT];
    };

    /// 创建一个由指定RTS矩阵构成的4x4矩阵
    static
    matrix_4x4
    make (
        const matrix_3x4 & rts_matx);

    /// 获得指定索引的行向量
    ///
    /// 例如:
    /// const matrix_4x4 matx;
    /// const float_4 row0_vec4d = matx[0];
    float_4
    operator[] (
        const uint8_t idx) const;

    /// 设定指定索引行向量
    ///
    /// 例如:
    /// matrix_4x4 matx;
    /// matx[0] = float_4::make(1, 2, 3, 4);
    float_4 &
    operator[] (
        const uint8_t idx);
};
