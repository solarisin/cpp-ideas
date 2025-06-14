#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <loguru/loguru.hpp>


TEST_CASE("Test random stuff", "[stuff]")
{
    SECTION("Test 1")
    {
        LOG_F(INFO, "The magic number is %d", 42);
        int i = 2;
        REQUIRE(i == 2);
    }
}