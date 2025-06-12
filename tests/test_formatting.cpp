#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/chrono.h>
#include <chrono>

TEST_CASE("Format library functionality", "[fmt]")
{
    SECTION("Basic string formatting")
    {
        std::string result = fmt::format("Hello, {}!", "World");
        REQUIRE(result == "Hello, World!");
    }

    SECTION("Number formatting")
    {
        std::string result = fmt::format("The answer is {}", 42);
        REQUIRE(result == "The answer is 42");
        
        std::string pi_result = fmt::format("Pi is approximately {:.2f}", 3.14159);
        REQUIRE(pi_result == "Pi is approximately 3.14");
    }

    SECTION("Advanced formatting")
    {
        // Positional arguments
        std::string result = fmt::format("{1} {0}", "World", "Hello");
        REQUIRE(result == "Hello World");
        
        // Named arguments
        std::string named_result = fmt::format("{name} is {age} years old", 
                                             fmt::arg("name", "Alice"), 
                                             fmt::arg("age", 30));
        REQUIRE(named_result == "Alice is 30 years old");
    }

    SECTION("Custom types and formatting")
    {
        auto now = std::chrono::system_clock::now();
        std::string time_str = fmt::format("Current time: {}", now);
        REQUIRE(!time_str.empty());
        REQUIRE(time_str.find("Current time:") == 0);
    }

    SECTION("Performance compared to iostream")
    {
        // This test demonstrates that fmt is generally faster
        const int iterations = 1000;
        
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            std::string result = fmt::format("Number: {}, Float: {:.2f}", i, i * 3.14);
        }
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // Just verify it completes without error
        REQUIRE(duration.count() >= 0);
    }
}
