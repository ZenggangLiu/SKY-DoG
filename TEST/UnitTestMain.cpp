// --- 如何使用CATCH UNIT TEST FRAMEWORK --- //
/// NOTE: REQUIRE((CONDITION));
/// 注意: 在CONDITION四周加上括弧()
/*
    TEST_CASE("Checking XXTEA encode/decode", "[XXTEA]")
    {
        /// 定义共享的变量
        const uint8_t _key[] = { 0, 1, 2, 3, 4, 5 };
        const uint8_t _clear_txt[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
        const uint8_t _encoded_txt[] =
        {
            0x50, 0x23, 0x8A, 0xAF,
            0x5C, 0xF7, 0x59, 0x71,
            0xCD, 0xA8, 0x1A, 0x12,
            0x56, 0xAD, 0x01, 0x1B
        };
        /// 操作用的缓存
        uint32_t _buffer[sizeof(_clear_txt) >> 2];

        /// 第一个Check Section
        SECTION("Checking Encoding:")
        {
            std::memcpy(_buffer, _clear_txt, sizeof(_buffer));
            XXTEA::encode(_key, sizeof(_key), _buffer, ARRAY_ITEM_COUNT(_buffer));
            for (uint32_t c = 0; c < sizeof(_buffer); ++c)
            {
                REQUIRE((((const uint8_t*)_buffer)[c] != _encoded_txt[c]));
            }
        }

        /// 第二个Check Section: 在第一个Section执行完成后执行
        SECTION("Checking Decoding:")
        {
            std::memcpy(_buffer, _encoded_txt, sizeof(_buffer));
            XXTEA::decode(_key, sizeof(_key), _buffer, ARRAY_ITEM_COUNT(_buffer));
            for (uint32_t c = 0; c < sizeof(_buffer); ++c)
            {
                REQUIRE((((const uint8_t*)_buffer)[c] == _clear_txt[c]));
            }
        }
    } /// TEST_CASE("Checking XXTEA encode/decode", "[XXTEA]")
*/


/// 如果使用Catch的main函数，在main.cpp/mm中定义这个宏
#define CATCH_CONFIG_MAIN

/// 如果使用我们的main()函数，在main.cpp/mm中定义这个宏
///#define CATCH_CONFIG_RUNNER


/// 最后包含Catch的头文件
#include "External/Catch/catch_v2.hpp"
