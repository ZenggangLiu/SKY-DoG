/// System headers
#include <algorithm> /// std::shuffle
#include <random>    /// std::mt19937
/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Common/CommonDefines.hpp" /// FOUR_CC_32
#include "Sampler/HammersleySamplePointsData.hpp"
#include "Sampler/HammersleySamplePointsDefines.hpp"
#include "Time/ClockTime.hpp"
/// Self header
#include "Sampler/PointSampler.hpp"


#define FIXED_RANDOM_SEED FOUR_CC_32('H', 'M', 'L', 'Y')


PointSampler::PointSampler (
    const PointDistribution dist)
:
#if defined(ENABLE_RENDERING_DEBUG) /// 如果 === 进行渲染调试 ===, 使用固定SEED
    m_random_number(FIXED_RANDOM_SEED),
#else
    m_random_number(),
#endif
    m_sample_point_array(initialize_sample_point_array(dist)),
    m_sample_set_index(0), /// 第一回调用next_sample_index()时, 会随机更新m_sample_set_index
    m_index_array_index(0)
{
    initialize_index_data_array();
}


const float_2 *
PointSampler::initialize_sample_point_array (
    const PointDistribution dist)
{
    switch (dist)
    {
        case PointDistribution::SPHERICAL_DISTRIBUTION:
        {
            return HAMMERSLEY_SPHERICAL_SAMPLE_ARRAY;
            break;
        }

        case PointDistribution::UNIT_SQUARE_DISTRIBUTION:
        {
            return HAMMERSLEY_UNIT_SQUARE_SAMPLE_ARRAY;
            break;
        }

        default:
        {
            RUNTIME_ASSERT(false, "Unknown PointDistribution type!!");
            return nullptr;
            break;
        }
    }
}


void
PointSampler::initialize_index_data_array ()
{
    /// 创建初始索引数据: [ 0, 1, 2, ... , #SamplePerSet-1 ]
    std::array<uint16_t, HAMMERSLEY_SET_SAMPLE_COUNT> index_data_set;
    for (uint16_t i = 0; i < HAMMERSLEY_SET_SAMPLE_COUNT; ++i)
    {
        index_data_set[i] = i;
    }

    /// 访问m_point_index_array数组的当前索引
    uint32_t array_index = 0;

    /// 遍历所有SET
    for (uint16_t i = 0; i < HAMMERSLEY_SAMPLE_SET_COUNT; ++i)
    {
        /// 对当前SET进行shuffle
        std::shuffle(index_data_set.begin(), index_data_set.end(), m_random_number);

        /// 保存shuffle过的SET数据
        for (const auto index_data : index_data_set)
        {
            m_point_index_array[array_index] = index_data;
            ++array_index;
        }
    }
}


uint32_t
PointSampler::next_sample_index ()
{
    /// 先更新采样组索引
    update_set_index();

    /// 由于每个采样组数据都有一个索引组数据:
    /// 采样组:
    /// SET0                      SET10
    /// +----+----+----+----+     +----+----+----+----+
    /// | S0 | S1 | S2 | S3 | ... | P0 | P1 | P2 | P3 |
    /// +----+----+----+----+     +----+----+----+----+
    /// 索引组:
    /// SET0                  SET10
    /// +---+---+---+---+     +---+---+---+---+
    /// | 0 | 2 | 1 | 3 | ... | 3 | 2 | 0 | 1 |
    /// +---+---+---+---+     +---+---+---+---+
    ///
    /// 起始索引(绝对): 当前采样点索引(SAMPLE INDEX)组的起始索引
    const uint32_t start_index = m_sample_set_index * HAMMERSLEY_SET_SAMPLE_COUNT;
    /// 组内索引(相对): 当前采样点索引(SAMPLE INDEX)组内的索引
    const uint16_t index_in_set = m_index_array_index % HAMMERSLEY_SET_SAMPLE_COUNT;
    /// 采样点索引数据的绝对索引
    const uint32_t index_abs = start_index + index_in_set;

    /// 采样点(SAMPLE)的相对索引数据
    const uint16_t rel_sample_index = m_point_index_array[index_abs];
    const uint32_t abs_sample_index = start_index + rel_sample_index;

    /// Advance组内索引(相对): 当前采样点索引(SAMPLE INDEX)组内的索引
    ++m_index_array_index;

    return abs_sample_index;
}


void
PointSampler::update_set_index ()
{
    /// 如果没有耗尽一个采样组
    if (m_index_array_index % HAMMERSLEY_SET_SAMPLE_COUNT)
    {
        return;
    }
    else
    {
        /// 使用随机数发生器，更新当前采样组索引
        m_sample_set_index =
            static_cast<uint16_t>(
                m_random_number.next_uint_range(0, HAMMERSLEY_SAMPLE_SET_COUNT - 1));
        RUNTIME_ASSERT(m_sample_set_index < HAMMERSLEY_SAMPLE_SET_COUNT,
                       "Sample set index is out of range!!");
    }
}
