#include "External/Catch/catch_v2.hpp"
#include "UT_Config.hpp"

#if (RING_BUFFER_CHECKING == 1)
/// System headers
#include <cstdio>   /// std::printf
#include <cstring>  /// std::strlen
#include <stdint.h> /// uint32_t,...
/// Libray headers
#include "Container/RingBuffer.hpp"


/// TEST_CASE( name [, tags ] )
TEST_CASE("Checking RingBuffer", "[RingBuffer]")
{

#define TEST_READ(rb, read_bytes)  \
((rb.m_data_read_pos - rb.m_saved_data_head.load(std::memory_order_relaxed)) == read_bytes)
#define TEST_SAVE(rb, saved_bytes) \
((rb.m_data_save_pos - rb.m_saved_data_tail.load(std::memory_order_relaxed)) == saved_bytes)
    typedef std::string StdStringT;

    // -- 输出数据 -- //
    /// Primitive types
    const int8_t     sbyte_out   = 0x1A;
    const int16_t    sshort_out  = 0x1ABB;
    const int32_t    sint_out    = 0x1ABBCCDD;
    const int64_t    slong_out   = 0x1ABBCCDDEEFF1122;
    const uint8_t    ubyte_out   = 0x0A;
    const uint16_t   ushort_out  = 0x0ABB;
    const uint32_t   uint_out    = 0x0ABBCCDD;
    const uint64_t   ulong_out   = 0x0ABBCCDDEEFF1122;
    const char *     cstring_out = "SOMETHING";
    const StdStringT stdstring_out("OTHERTHING");

    /// Custum types
    struct CustomType
    {
        uint8_t array[16];
    };
    const CustomType array_out = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF };
    const CustomType * const array_ptr_out = &array_out;

    // --- 输入数据 --- //
    /// Primitive types
    int8_t     sbyte_in  = 0;
    int16_t    sshort_in = 0;
    int32_t    sint_in   = 0;
    int64_t    slong_in  = 0;
    uint8_t    ubyte_in  = 0;
    uint16_t   ushort_in = 0;
    uint32_t   uint_in   = 0;
    uint64_t   ulong_in  = 0;
    StdStringT cstring_in;
    StdStringT stdstring_in;

    /// Custum types
    CustomType array_in{ 0 };
    const CustomType * array_ptr_in = nullptr;

    // --- RingBuffer实例 --- //
    static constexpr uint32_t BUFFER_BYTE_SIZE = 1024;
    RingBuffer ring_buffer(BUFFER_BYTE_SIZE);

    SECTION("Checking RingBuffer")
    {
        std::printf("--- Checking RingBuffer...\n");

        // --- SAVE OUT --- //
        bool isSuccess = ring_buffer.save_data(sbyte_out);
        isSuccess = isSuccess && TEST_SAVE(ring_buffer, sizeof(sbyte_out));
        ring_buffer.finish_save();
        isSuccess = isSuccess && ring_buffer.save_data(sshort_out);
        isSuccess = isSuccess && TEST_SAVE(ring_buffer, sizeof(sshort_out));
        ring_buffer.finish_save();
        isSuccess = isSuccess && ring_buffer.save_data(sint_out);
        isSuccess = isSuccess && TEST_SAVE(ring_buffer, sizeof(sint_out));
        ring_buffer.finish_save();
        isSuccess = isSuccess && ring_buffer.save_data(slong_out);
        isSuccess = isSuccess && TEST_SAVE(ring_buffer, sizeof(slong_out));
        ring_buffer.finish_save();
        isSuccess = isSuccess && ring_buffer.save_data(ubyte_out);
        isSuccess = isSuccess && TEST_SAVE(ring_buffer, sizeof(ubyte_out));
        ring_buffer.finish_save();
        isSuccess = isSuccess && ring_buffer.save_data(ushort_out);
        isSuccess = isSuccess && TEST_SAVE(ring_buffer, sizeof(ushort_out));
        ring_buffer.finish_save();
        isSuccess = isSuccess && ring_buffer.save_data(uint_out);
        isSuccess = isSuccess && TEST_SAVE(ring_buffer, sizeof(uint_out));
        ring_buffer.finish_save();
        isSuccess = isSuccess && ring_buffer.save_data(ulong_out);
        isSuccess = isSuccess && TEST_SAVE(ring_buffer, sizeof(ulong_out));
        ring_buffer.finish_save();
        isSuccess = isSuccess && ring_buffer.save_data(cstring_out);
        isSuccess = isSuccess && TEST_SAVE(ring_buffer, std::strlen(cstring_out)+2);
        ring_buffer.finish_save();
        isSuccess = isSuccess && ring_buffer.save_data(stdstring_out);
        isSuccess = isSuccess && TEST_SAVE(ring_buffer, stdstring_out.size()+2);
        ring_buffer.finish_save();
        isSuccess = isSuccess && ring_buffer.save_data(array_out);
        isSuccess = isSuccess && TEST_SAVE(ring_buffer, sizeof(array_out));
        ring_buffer.finish_save();
        isSuccess = isSuccess && ring_buffer.save_data(array_ptr_out);
        isSuccess = isSuccess && TEST_SAVE(ring_buffer, sizeof(array_ptr_out));
        ring_buffer.finish_save();
        REQUIRE((isSuccess));

        // --- READ IN --- //
        isSuccess = true;
        isSuccess = isSuccess && ring_buffer.read_data(sbyte_in);
        isSuccess = isSuccess && TEST_READ(ring_buffer, sizeof(sbyte_in));
        isSuccess = isSuccess && sbyte_in == sbyte_out;
        ring_buffer.finish_read();
        isSuccess = isSuccess && ring_buffer.read_data(sshort_in);
        isSuccess = isSuccess && TEST_READ(ring_buffer, sizeof(sshort_in));
        isSuccess = isSuccess && sshort_in == sshort_out;
        ring_buffer.finish_read();
        isSuccess = isSuccess && ring_buffer.read_data(sint_in);
        isSuccess = isSuccess && TEST_READ(ring_buffer, sizeof(sint_in));
        isSuccess = isSuccess && sint_in == sint_out;
        ring_buffer.finish_read();
        isSuccess = isSuccess && ring_buffer.read_data(slong_in);
        isSuccess = isSuccess && TEST_READ(ring_buffer, sizeof(slong_in));
        isSuccess = isSuccess && slong_in == slong_out;
        ring_buffer.finish_read();
        isSuccess = isSuccess && ring_buffer.read_data(ubyte_in);
        isSuccess = isSuccess && TEST_READ(ring_buffer, sizeof(ubyte_in));
        isSuccess = isSuccess && ubyte_in == ubyte_out;
        ring_buffer.finish_read();
        isSuccess = isSuccess && ring_buffer.read_data(ushort_in);
        isSuccess = isSuccess && TEST_READ(ring_buffer, sizeof(ushort_in));
        isSuccess = isSuccess && ushort_in == ushort_out;
        ring_buffer.finish_read();
        isSuccess = isSuccess && ring_buffer.read_data(uint_in);
        isSuccess = isSuccess && TEST_READ(ring_buffer, sizeof(uint_in));
        isSuccess = isSuccess && uint_in == uint_out;
        ring_buffer.finish_read();
        isSuccess = isSuccess && ring_buffer.read_data(ulong_in);
        isSuccess = isSuccess && TEST_READ(ring_buffer, sizeof(ulong_in));
        isSuccess = isSuccess && ulong_in == ulong_out;
        ring_buffer.finish_read();
        isSuccess = isSuccess && ring_buffer.read_data(cstring_in);
        isSuccess = isSuccess && TEST_READ(ring_buffer, cstring_in.size()+2);
        isSuccess = isSuccess && std::strcmp(cstring_in.c_str(), cstring_out) == 0;
        ring_buffer.finish_read();
        isSuccess = isSuccess && ring_buffer.read_data(stdstring_in);
        isSuccess = isSuccess && TEST_READ(ring_buffer, stdstring_in.size()+2);
        isSuccess = isSuccess && std::strcmp(stdstring_in.c_str(), stdstring_out.c_str()) == 0;
        ring_buffer.finish_read();
        isSuccess = isSuccess && ring_buffer.read_data(array_in);
        isSuccess = isSuccess && TEST_READ(ring_buffer, sizeof(array_in));
        isSuccess = isSuccess && std::memcmp(&array_in, &array_out, sizeof(array_in)) == 0;
        ring_buffer.finish_read();
        isSuccess = isSuccess && ring_buffer.read_data(array_ptr_in);
        isSuccess = isSuccess && TEST_READ(ring_buffer, sizeof(array_ptr_in));
        isSuccess = isSuccess && (array_ptr_in == array_ptr_out);
        ring_buffer.finish_read();
        REQUIRE((isSuccess));

        std::printf("--- Checking RingBuffer: OK!\n");
    } /// SECTION("Checking RingBuffer")

} /// TEST_CASE("Checking RingBuffer", "[RingBuffer]")

#endif /// (RING_BUFFER_CHECKING == 1)
