#include <catch2/catch_test_macros.hpp>
#include "star/core/types.hpp"

TEST_CASE("Type aliases have correct byte sizes", "[core][types]") {
    REQUIRE(sizeof(i8)   == 1);
    REQUIRE(sizeof(i16)  == 2);
    REQUIRE(sizeof(i32)  == 4);
    REQUIRE(sizeof(i64)  == 8);
    REQUIRE(sizeof(u8)   == 1);
    REQUIRE(sizeof(u16)  == 2);
    REQUIRE(sizeof(u32)  == 4);
    REQUIRE(sizeof(u64)  == 8);
    REQUIRE(sizeof(f32)  == 4);
    REQUIRE(sizeof(f64)  == 8);
    REQUIRE(sizeof(byte) == 1);
}

TEST_CASE("Signed integer ranges are correct", "[core][types]") {
    constexpr i8  i8_max  = std::numeric_limits<i8>::max();
    constexpr i16 i16_max = std::numeric_limits<i16>::max();
    constexpr i32 i32_max = std::numeric_limits<i32>::max();
    constexpr i64 i64_max = std::numeric_limits<i64>::max();

    REQUIRE(i8_max  == 127);
    REQUIRE(i16_max == 32767);
    REQUIRE(i32_max == 2147483647);
    REQUIRE(i64_max == 9223372036854775807LL);
}

TEST_CASE("Unsigned integer ranges are correct", "[core][types]") {
    constexpr u8  u8_max  = std::numeric_limits<u8>::max();
    constexpr u16 u16_max = std::numeric_limits<u16>::max();
    constexpr u32 u32_max = std::numeric_limits<u32>::max();
    constexpr u64 u64_max = std::numeric_limits<u64>::max();

    REQUIRE(u8_max  == 255u);
    REQUIRE(u16_max == 65535u);
    REQUIRE(u32_max == 4294967295u);
    REQUIRE(u64_max == 18446744073709551615ull);
}

TEST_CASE("Float types satisfy standard precision requirements", "[core][types]") {
    REQUIRE(std::numeric_limits<f32>::digits10 >= 6);
    REQUIRE(std::numeric_limits<f64>::digits10 >= 15);
}
