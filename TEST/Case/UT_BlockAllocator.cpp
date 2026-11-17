#include "External/Catch/catch_v2.hpp"
#include "UT_Config.hpp"

#if (BLOCK_ALLOCATOR_CHECKING == 1)
/// System headers
#include <cstdio>   /// std::printf
#include <stdint.h> /// uint8_t,...
#include <vector>
/// Library headers
#include "Memory/BlockAllocator.hpp"


namespace
{

/// 用一个固定大小的伪类型驱动BlockAllocator<T>生成不同的Block size
template < uint32_t ByteSize >
struct SizedType
{
    uint8_t bytes[ByteSize];
};


/// 检查一个刚初始化完成的BlockAllocator是否处于完整空闲状态
template < typename AllocType >
static
void
check_fresh_allocator_stats (
    AllocType &     allocator,
    const uint16_t  expected_block_size)
{
    REQUIRE((allocator.initialize(1, 2)));
    REQUIRE((allocator.block_size() == expected_block_size));
    REQUIRE((allocator.block_count() > 0));
    REQUIRE((allocator.free_count() == allocator.block_count()));
    REQUIRE((allocator.used_count() == 0));
    REQUIRE((allocator.allocated_bytes() >= BLOCK_ALLOCATOR_PAGE_SIZE));
}


/// 在分配完所有Block后，再申请分配一个Block，以确认BlockAllocator可以自动增长
template < typename AllocType >
static
void
check_fill_and_grow (
    AllocType & allocator)
{
    const uint32_t first_block_count = allocator.block_count();
    std::vector<uint8_t*> blocks;
    blocks.reserve(first_block_count + 1);

    for (uint32_t i = 0; i < first_block_count; ++i)
    {
        uint8_t * const block = allocator.allocate();
        REQUIRE((block != nullptr));
        blocks.push_back(block);
        REQUIRE((allocator.used_count() == i + 1));
        REQUIRE((allocator.free_count() == first_block_count - i - 1));
    }

    REQUIRE((allocator.used_count() == first_block_count));
    REQUIRE((allocator.free_count() == 0));

    /// 当前Page已经耗尽，下一次申请将触发新的Page分配
    uint8_t * const grown_block = allocator.allocate();
    REQUIRE((grown_block != nullptr));
    blocks.push_back(grown_block);
    REQUIRE((allocator.block_count() > first_block_count));
    REQUIRE((allocator.used_count() == first_block_count + 1));

    for (auto block : blocks)
    {
        REQUIRE((allocator.deallocate(block)));
    }
    REQUIRE((allocator.used_count() == 0));
    REQUIRE((allocator.free_count() == allocator.block_count()));
}


/// 检查释放后的block可以复用，同时非法地址和重复释放会被拒绝
template < typename AllocType >
static
void
check_reuse_and_invalid_deallocate (
    AllocType & allocator)
{
    uint8_t * const block = allocator.allocate();
    REQUIRE((block != nullptr));
    REQUIRE((allocator.used_count() == 1));

    REQUIRE_FALSE((allocator.deallocate(nullptr)));
    REQUIRE_FALSE((allocator.deallocate(block + 1)));

    REQUIRE((allocator.deallocate(block)));
    REQUIRE((allocator.used_count() == 0));
    REQUIRE_FALSE((allocator.deallocate(block)));

    uint8_t * const reused_block = allocator.allocate();
    REQUIRE((reused_block == block));
    REQUIRE((allocator.deallocate(reused_block)));
}

} /// namespace


/// TEST_CASE( name [, tags ] )
TEST_CASE("Checking BlockAllocator", "[BlockAllocator]")
{
    static constexpr uint32_t TEST_OBJECT_COUNT = 1u;

    SECTION("Checking block size rounding")
    {
        std::printf("--- Checking BlockAllocator block size rounding...\n");

        /// BlockAllocator会将类型大小round up到合法block size。
        auto & allocator_01 =
            BlockAllocator<SizedType<1>, TEST_OBJECT_COUNT>::ref();
        auto & allocator_16 =
            BlockAllocator<SizedType<16>, TEST_OBJECT_COUNT>::ref();
        auto & allocator_17 =
            BlockAllocator<SizedType<17>, TEST_OBJECT_COUNT>::ref();
        auto & allocator_128 =
            BlockAllocator<SizedType<128>, TEST_OBJECT_COUNT>::ref();
        auto & allocator_129 =
            BlockAllocator<SizedType<129>, TEST_OBJECT_COUNT>::ref();
        auto & allocator_249 =
            BlockAllocator<SizedType<249>, TEST_OBJECT_COUNT>::ref();
        auto & allocator_256 =
            BlockAllocator<SizedType<256>, TEST_OBJECT_COUNT>::ref();

        check_fresh_allocator_stats(allocator_01, 16);
        check_fresh_allocator_stats(allocator_16, 16);
        check_fresh_allocator_stats(allocator_17, 24);
        check_fresh_allocator_stats(allocator_128, 128);
        check_fresh_allocator_stats(allocator_129, 136);
        check_fresh_allocator_stats(allocator_249, 256);
        check_fresh_allocator_stats(allocator_256, 256);

        std::printf("--- Checking BlockAllocator block size rounding: OK!\n");
    }

    SECTION("Checking allocation, growth and full release")
    {
        std::printf("--- Checking BlockAllocator allocation/growth/release...\n");

        /// 使用最小block size覆盖单页填满、自动扩展、release后重建。
        auto & allocator =
            BlockAllocator<SizedType<16>, TEST_OBJECT_COUNT>::ref();
        check_fresh_allocator_stats(allocator, 16);
        check_fill_and_grow(allocator);

        allocator.release();
        REQUIRE((allocator.block_count() == 0));
        REQUIRE((allocator.free_count() == 0));
        REQUIRE((allocator.used_count() == 0));
        REQUIRE((allocator.allocated_bytes() == 0));

        check_fresh_allocator_stats(allocator, 16);

        std::printf("--- Checking BlockAllocator allocation/growth/release: OK!\n");
    }

    SECTION("Checking reuse and invalid deallocate")
    {
        std::printf("--- Checking BlockAllocator reuse/invalid deallocate...\n");

        /// 覆盖正常释放、重复释放、非block起始地址和外部地址。
        auto & allocator =
            BlockAllocator<SizedType<64>, TEST_OBJECT_COUNT>::ref();
        check_fresh_allocator_stats(allocator, 64);
        check_reuse_and_invalid_deallocate(allocator);

        int stack_value = 0;
        REQUIRE_FALSE((allocator.deallocate(&stack_value)));

        std::printf("--- Checking BlockAllocator reuse/invalid deallocate: OK!\n");
    }

    SECTION("Checking 256-byte blocks")
    {
        std::printf("--- Checking BlockAllocator 256-byte blocks...\n");

        /// 256是当前最大block size，单独覆盖上限路径。
        auto & allocator =
            BlockAllocator<SizedType<256>, TEST_OBJECT_COUNT>::ref();
        check_fresh_allocator_stats(allocator, 256);
        check_fill_and_grow(allocator);
        check_reuse_and_invalid_deallocate(allocator);

        std::printf("--- Checking BlockAllocator 256-byte blocks: OK!\n");
    }
} /// TEST_CASE("Checking BlockAllocator", "[BlockAllocator]")

#endif /// (BLOCK_ALLOCATOR_CHECKING == 1)
