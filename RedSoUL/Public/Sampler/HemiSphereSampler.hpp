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
    Created:  28/09/26  @  11:05 PM
    FileName: HemiSphereSampler.hpp @ RedSoUL Project
    History:
             - created by: 28/09/26: Zenggang LIU
                                                                                        
***************************************************************************************/


#pragma once


/// Library headers
/// Library headers
#include "DataType/Float3D.hpp"
#include "Sampler/PointSampler.hpp"


/// 半球采样器
///
///         UPWARD|Y
///         ^
///         |
///         |   / FORWARD|Z
///       * + */
///    *    | /  *
///  *      |/     *
///  +------o------+-----> RIGHT|X
/// -1             1
///
class HemiSphereSampler : public PointSampler
{
public:
    HemiSphereSampler ();

    /// 返回单位半球(Unit Hemi-Sphere)上的下一个采样点
    ///
    /// UPWARD|Y
    /// ^
    /// |    / FORWARD|Z
    /// |   /
    /// |  /
    /// | /
    /// |/
    /// o------------> RIGHT|X
    /// NOTE: 输入的Axis必须归一化
    /// 返回的点已归一化
    float_3
    next_sample_point (
        const float_3 upward_axis,
        const float_3 forward_axis,
        const float_3 right_axis);

private:
    using SuperT = PointSampler;
};
