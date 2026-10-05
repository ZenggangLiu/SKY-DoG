/// System headers
#include <limits> /// std::numeric_limit<>::max
/// Library headers
#include "Assert/RuntimeAssert.hpp"
#include "Time/ClockTime.hpp"
/// Self header
#include "Math/RandomNumber.hpp"


#define ROTL(x, k) (((x) << (k)) | ((x) >> (64 - (k))))


/// 计算基于当前时间的SEED
static
uint64_t
calc_time_based_seed ()
{
    static uint64_t RANDOM_NUMBER_GENERATOR_INDEX = 0;

    /// 创建基于时间的Seed
    const uint64_t seed_value =
        (uint64_t)ClockTime::mono_time_ms() + RANDOM_NUMBER_GENERATOR_INDEX;
    ++RANDOM_NUMBER_GENERATOR_INDEX;

    /// 使用Seed内存的位置来得到一个随机的数值
    const uint64_t noise_value = (uint64_t)&seed_value;
    return seed_value ^ noise_value;
}


/// 混合指定数值
///
/// @return
///     混合结果
static
uint64_t
splitmix64 (
    const uint64_t num)
{
    uint64_t z = num;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}



RandomNumber::RandomNumber ()
{
    initialize(calc_time_based_seed());
}


RandomNumber::RandomNumber (
    const uint64_t seed)
{
    initialize(seed);
}


RandomNumber::result_type
RandomNumber::operator() ()
{
    return next_uint();
}


uint64_t
RandomNumber::next_uint ()
{
    /// 原始代码请参考: https://prng.di.unimi.it/xoshiro256plusplus.c
    /// xoshiro256++
    const uint64_t result = ROTL(state0 + state3, 23) + state0;

    const uint64_t t = state1 << 17;

    state2 ^= state0;
    state3 ^= state1;
    state1 ^= state2;
    state0 ^= state3;

    state2 ^= t;
    state3 = ROTL(state3, 45);

    return result;
}


uint64_t
RandomNumber::next_uint_range (
    const uint64_t min,
    const uint64_t max)
{
    static constexpr uint64_t MAX_UINT64 = std::numeric_limits<uint64_t>::max();

    RUNTIME_ASSERT(max > min, "Wrong number range!!");

    /// 使用如下方式来防止对于某些数值产生的Bias:
    /// - 将整个数据区域划分为N个指定个数的群: 指定各数Range := Max - Min + 1 个数值
    /// - 这样整个数值空间[0, MaxUInt64] := N * Range + DiscardRange
    ///   其中DiscardRange为无法归组的数值各数
    /// - 抛弃所有落入DiscardRange中的随机数
    /// 例如: 总共有16个数, 假设min, max为[2, 4], 指定各数Range为3
    /// 那么: 将16个数可以分为五组: 16 := 5 * 3 + 1, 一个在DiscardRange内
    ///
    const uint64_t range = max - min + 1;

    /// std::numeric_limit<uint64_t>::max()为2^64 - 1
    /// 减去一个Range, 以防止计算出界
    const uint64_t discard = (MAX_UINT64 - range + 1) % range;

    uint64_t rand_value;
    do
    {
        rand_value = next_uint();
    } while (rand_value < discard);

    return min + (rand_value % range);
}


float
RandomNumber::next_float ()
{
    /// [0,1)
    /// 使用高24位数值
    return (next_uint() >> 40) * (1.0f / (1u << 24));
}


void
RandomNumber::initialize (
    const uint64_t seed)
{
    /// 使用 splitmix64 扩展 seed → 256-bit 状态
    uint64_t x = seed;
    x += 0x9E3779B97f4A7C15ULL;
    state0 = splitmix64(x);
    x += 0x9E3779B97f4A7C15ULL;
    state1 = splitmix64(x);
    x += 0x9E3779B97f4A7C15ULL;
    state2 = splitmix64(x);
    x += 0x9E3779B97f4A7C15ULL;
    state3 = splitmix64(x);

    /// 避免全0状态
    if ((state0 | state1 | state2 | state3) == 0)
    {
        state0 = 1;
    }
}
