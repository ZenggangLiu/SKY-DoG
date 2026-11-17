/// Self header
#include "Hashing/HashUtilities.hpp"


uint32_t
HashUtility::split_mix32 (
    const uint32_t value)
{
    uint32_t x = value;
    x ^= x >> 16;
    x *= 0x85EBCA6BU;

    x ^= x >> 13;
    x *= 0xC2B2AE35U;

    x ^= x >> 16;
    return x;
}


uint64_t
HashUtility::split_mix64 (
    const uint64_t value)
{
    uint64_t x = value;
    x *= 0xBF58476D1CE4E5B9ULL;

    x ^= x >> 27;
    x *= 0x94D049BB133111EBULL;

    x ^= x >> 31;
    return x;
}
