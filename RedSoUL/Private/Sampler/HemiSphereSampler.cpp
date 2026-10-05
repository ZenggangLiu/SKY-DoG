/// Library headers
#include "DataType/Float2D.hpp"
#include "Math/MathUtilities.hpp"
/// Self header
#include "Sampler/HemiSphereSampler.hpp"


HemiSphereSampler::HemiSphereSampler ()
:
    SuperT(PointDistribution::SPHERICAL_DISTRIBUTION)
{
    
}


float_3
HemiSphereSampler::next_sample_point (
    const float_3 upward_axis,
    const float_3 forward_axis,
    const float_3 right_axis)
{
    /// 提取半球(Hemi-Sphere)上采样点(SAMPLE)的数据
    /// (φ, θ)
    const float_2 sample_point = m_sample_point_array[next_sample_index()];
    /// 计算Sine/Cosine
    float sin_phi, cos_phi;
    float sin_theta, cos_theta;
    MathUtility::fast_sincos(sample_point.x, sin_phi,   cos_phi);
    MathUtility::fast_sincos(sample_point.y, sin_theta, cos_theta);

    /// 合成采样点
    /// UPWARD|Y
    /// ^
    /// |    / FORWARD|Z
    /// |   /
    /// |  /
    /// | /
    /// |/
    /// o------------> RIGHT|X
    /// NOTE: φ表示Z轴向X轴方向的旋转
    /// 
    /// x := sin(θ)*sin(φ)*R
    /// y := cos(θ)*U
    /// z := sin(θ)*cos(φ)*F
    /// 详见Spherical_coordinate_to_cartesian_system.png文件
    const float_3 point_x = right_axis   * sin_theta * sin_phi;
    const float_3 point_y = upward_axis  * cos_theta;
    const float_3 point_z = forward_axis * sin_theta * cos_phi;
    const float_3 combined = point_x + point_y + point_z;
    return combined.unified_vec();
}
