#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

TEST_CASE("JSON parsing and manipulation", "[json]")
{
    SECTION("Basic JSON creation and access")
    {
        json config = {
            {"name", "test"},
            {"version", 1},
            {"active", true}
        };

        REQUIRE(config["name"] == "test");
        REQUIRE(config["version"] == 1);
        REQUIRE(config["active"] == true);
    }

    SECTION("JSON serialization")
    {
        json data = {
            {"pi", 3.141},
            {"happy", true},
            {"nothing", nullptr},
            {"answer", {
                {"everything", 42}
            }},
            {"list", {1, 0, 2}},
            {"object", {
                {"currency", "USD"},
                {"value", 42.99}
            }}
        };

        std::string serialized = data.dump();
        REQUIRE(!serialized.empty());
        
        // Parse it back
        json parsed = json::parse(serialized);
        REQUIRE(parsed["pi"] == 3.141);
        REQUIRE(parsed["answer"]["everything"] == 42);
    }

    SECTION("JSON validation")
    {
        std::string valid_json = R"({"name": "test", "value": 123})";
        std::string invalid_json = R"({"name": "test", "value": })";

        REQUIRE_NOTHROW(json::parse(valid_json));
        REQUIRE_THROWS(json::parse(invalid_json));
    }
}
