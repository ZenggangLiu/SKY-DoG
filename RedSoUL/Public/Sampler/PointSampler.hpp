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
    Created:  28/09/26  @  11:03 PM
    FileName: PointSampler.hpp @ RedSoUL Project
    History:
             - created by: 28/09/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// System headers
#include <array>
#include <stdint.h> /// uint32_t
/// Library headers
#include "Math/RandomNumber.hpp"
#include "Sampler/HammersleySamplePointsDefines.hpp"


struct float_2;


/// 采样点分布
///
enum class PointDistribution
{
    /// 半球分布:
    ///
    ///         N/Y
    ///         ^
    ///         |    / B/Z
    ///       * +1* /
    ///    *    |  / *
    ///  *      | /    *
    ///  +------o------+-----> T/X
    /// -1             1
    SPHERICAL_DISTRIBUTION,

    /// 正方形分布:
    ///  ^ Y
    ///  |
    /// 1+-------------+ (1,1)
    ///  |             |
    ///  |             |
    ///  |             |
    ///  |             |
    /// 0o-------------+-----> X
    ///  0             1
    UNIT_SQUARE_DISTRIBUTION,
};


class PointSampler
{
protected:
    PointSampler (
        const PointDistribution dist);

    ~PointSampler () = default;

    /// 初始化采样点数据组
    const float_2 *
    initialize_sample_point_array (
        const PointDistribution dist);

    /// 初始化采样点索引数据组
    void
    initialize_index_data_array ();

    /// 返回下一个采样点(SAMPLE)的索引
    /// NOTE: 此函数将更新m_sample_set_index, m_index_array_index
    uint32_t
    next_sample_index ();

    /// (使用随机数发生器)更新采样组(SAMPLE SET)的索引
    void
    update_set_index ();

protected:
    using PointDataArrayT  = const float_2 * const;
    using PointIndexArrayT = std::array<uint16_t,
                                        HAMMERSLEY_TOTAL_SAMPLE_COUNT>;

    RandomNumber     m_random_number;
    /// SampleGen生成的采样点(SAMPLE)数据(所有采样点)
    PointDataArrayT  m_sample_point_array;
    /// Shuffle过的采样点索引(SAMPLE INDEX)数据(所有采样点索引)
    /// 索引组:
    /// SET0                  SET10
    /// +---+---+---+---+     +---+---+---+---+
    /// | 0 | 2 | 1 | 3 | ... | 3 | 2 | 0 | 1 |
    /// +---+---+---+---+     +---+---+---+---+
    PointIndexArrayT m_point_index_array;

    // --- WORKING DATA --- //
    /// 当前采样组(SAMPLE SET)的索引: [0, #SET-1]
    uint16_t         m_sample_set_index;
    /// 当前采样点索引(SAMPLE INDEX)数据的索引(对m_point_index_array的索引): [0, +INF]
    uint32_t         m_index_array_index;
};
