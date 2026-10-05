/// Self header
#include "Sampler/UnitSquareSampler.hpp"


UnitSquareSampler::UnitSquareSampler ()
:
    SuperT(PointDistribution::UNIT_SQUARE_DISTRIBUTION)
{
    
}


float_2
UnitSquareSampler::next_sample_point ()
{
    const float_2 sample_point = m_sample_point_array[next_sample_index()];
    return sample_point;
}
