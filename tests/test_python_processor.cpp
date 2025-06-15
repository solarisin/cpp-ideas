#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <nlohmann/json.hpp>
#include "python_processor.h"
#include <loguru/loguru.hpp>

using json = nlohmann::json;
using Catch::Matchers::ContainsSubstring;

TEST_CASE("Python Processor Initialization", "[python][processor]")
{
    SECTION("Python processor initializes successfully")
    {
        PythonProcessor processor;
        
        REQUIRE(processor.isInitialized());
        REQUIRE(processor.getLastError().empty());
    }
}

TEST_CASE("Python Processor JSON Processing", "[python][processor][json]")
{
    PythonProcessor processor;
    REQUIRE(processor.isInitialized()); // Prerequisite
    
    SECTION("Math operations")
    {
        SECTION("Addition operation")
        {
            std::string request = R"({"type": "math", "operation": "add", "numbers": [1, 2, 3, 4, 5]})";
            std::string result = processor.processJson(request);
            
            REQUIRE_FALSE(result.empty());
            
            auto jsonResult = json::parse(result);
            REQUIRE(jsonResult["success"] == true);
            REQUIRE(jsonResult["result"] == 15);
            REQUIRE(jsonResult["operation"] == "add");
            REQUIRE(jsonResult["input_numbers"] == json::array({1, 2, 3, 4, 5}));
        }
        
        SECTION("Multiplication operation")
        {
            std::string request = R"({"type": "math", "operation": "multiply", "numbers": [2, 3, 4]})";
            std::string result = processor.processJson(request);
            
            auto jsonResult = json::parse(result);
            REQUIRE(jsonResult["success"] == true);
            REQUIRE(jsonResult["result"] == 24);
            REQUIRE(jsonResult["operation"] == "multiply");
        }
        
        SECTION("Mean operation")
        {
            std::string request = R"({"type": "math", "operation": "mean", "numbers": [10, 20, 30, 40, 50]})";
            std::string result = processor.processJson(request);
            
            auto jsonResult = json::parse(result);
            REQUIRE(jsonResult["success"] == true);
            REQUIRE(jsonResult["result"] == 30.0);
            REQUIRE(jsonResult["operation"] == "mean");
        }
        
        SECTION("Power operation")
        {
            std::string request = R"({"type": "math", "operation": "power", "numbers": [2, 3]})";
            std::string result = processor.processJson(request);
            
            auto jsonResult = json::parse(result);
            REQUIRE(jsonResult["success"] == true);
            REQUIRE(jsonResult["result"] == 8.0);
            REQUIRE(jsonResult["operation"] == "power");
        }
        
        SECTION("Empty numbers array")
        {
            std::string request = R"({"type": "math", "operation": "add", "numbers": []})";
            std::string result = processor.processJson(request);
            
            auto jsonResult = json::parse(result);
            REQUIRE(jsonResult["success"] == false);
            REQUIRE_THAT(jsonResult["error"].get<std::string>(), ContainsSubstring("Numbers array is required"));
        }
    }
    
    SECTION("Text operations")
    {
        SECTION("Uppercase operation")
        {
            std::string request = R"({"type": "text", "operation": "uppercase", "text": "hello world from qt!"})";
            std::string result = processor.processJson(request);
            
            auto jsonResult = json::parse(result);
            REQUIRE(jsonResult["success"] == true);
            REQUIRE(jsonResult["result"] == "HELLO WORLD FROM QT!");
            REQUIRE(jsonResult["operation"] == "uppercase");
            REQUIRE(jsonResult["input_text"] == "hello world from qt!");
        }
        
        SECTION("Lowercase operation")
        {
            std::string request = R"({"type": "text", "operation": "lowercase", "text": "HELLO WORLD"})";
            std::string result = processor.processJson(request);
            
            auto jsonResult = json::parse(result);
            REQUIRE(jsonResult["success"] == true);
            REQUIRE(jsonResult["result"] == "hello world");
            REQUIRE(jsonResult["operation"] == "lowercase");
        }
        
        SECTION("Reverse operation")
        {
            std::string request = R"({"type": "text", "operation": "reverse", "text": "hello"})";
            std::string result = processor.processJson(request);
            
            auto jsonResult = json::parse(result);
            REQUIRE(jsonResult["success"] == true);
            REQUIRE(jsonResult["result"] == "olleh");
            REQUIRE(jsonResult["operation"] == "reverse");
        }
        
        SECTION("Empty text")
        {
            std::string request = R"({"type": "text", "operation": "uppercase", "text": ""})";
            std::string result = processor.processJson(request);
            
            auto jsonResult = json::parse(result);
            REQUIRE(jsonResult["success"] == true);
            REQUIRE(jsonResult["result"] == "");
        }
    }
    
    SECTION("Data operations")
    {
        SECTION("Statistics calculation")
        {
            std::string request = R"({"type": "data", "operation": "stats", "dataset": [10, 20, 30, 40, 50, 25, 35, 45]})";
            std::string result = processor.processJson(request);
            
            auto jsonResult = json::parse(result);
            REQUIRE(jsonResult["success"] == true);
            REQUIRE(jsonResult["operation"] == "stats");
            
            auto stats = jsonResult["result"];
            REQUIRE(stats["count"] == 8);
            REQUIRE(stats["sum"] == 255);
            REQUIRE(stats["mean"] == 31.875);
            REQUIRE(stats["min"] == 10);
            REQUIRE(stats["max"] == 50);
            REQUIRE(stats["range"] == 40);
        }
        
        SECTION("Statistics with single value")
        {
            std::string request = R"({"type": "data", "operation": "stats", "dataset": [42]})";
            std::string result = processor.processJson(request);
            
            auto jsonResult = json::parse(result);
            REQUIRE(jsonResult["success"] == true);
            
            auto stats = jsonResult["result"];
            REQUIRE(stats["count"] == 1);
            REQUIRE(stats["sum"] == 42);
            REQUIRE(stats["mean"] == 42);
            REQUIRE(stats["min"] == 42);
            REQUIRE(stats["max"] == 42);
            REQUIRE(stats["range"] == 0);
        }
        
        SECTION("Empty dataset")
        {
            std::string request = R"({"type": "data", "operation": "stats", "dataset": []})";
            std::string result = processor.processJson(request);
            
            auto jsonResult = json::parse(result);
            REQUIRE(jsonResult["success"] == false);
            REQUIRE_THAT(jsonResult["error"].get<std::string>(), ContainsSubstring("empty"));
        }
    }
    
    SECTION("Echo operations")
    {
        SECTION("Basic echo")
        {
            std::string request = R"({"type": "echo", "message": "This is a test from Qt application"})";
            std::string result = processor.processJson(request);
            
            auto jsonResult = json::parse(result);
            REQUIRE(jsonResult["success"] == true);
            REQUIRE(jsonResult["result"] == "Echo successful");
            REQUIRE(jsonResult["echoed_data"]["message"] == "This is a test from Qt application");
        }
        
        SECTION("Echo with complex data")
        {
            std::string request = R"({"type": "echo", "data": {"nested": {"value": 123, "array": [1, 2, 3]}}})";
            std::string result = processor.processJson(request);
            
            auto jsonResult = json::parse(result);
            REQUIRE(jsonResult["success"] == true);
            REQUIRE(jsonResult["result"] == "Echo successful");
            REQUIRE(jsonResult["echoed_data"]["data"]["nested"]["value"] == 123);
        }
    }
    
    SECTION("Error handling")
    {
        SECTION("Invalid JSON")
        {
            std::string request = R"({"type": "math", "operation": "add", "numbers": [1, 2, 3})"; // Invalid JSON - missing ]
            std::string result = processor.processJson(request);
            
            REQUIRE_FALSE(result.empty());
            auto jsonResult = json::parse(result);
            REQUIRE(jsonResult["success"] == false);
            REQUIRE(jsonResult.contains("error"));
        }
        
        SECTION("Unknown operation type")
        {
            std::string request = R"({"type": "unknown", "operation": "test"})";
            std::string result = processor.processJson(request);
            
            auto jsonResult = json::parse(result);
            REQUIRE(jsonResult["success"] == false);
            REQUIRE_THAT(jsonResult["error"].get<std::string>(), ContainsSubstring("Unknown"));
        }
        
        SECTION("Missing required fields")
        {
            std::string request = R"({"type": "math"})"; // Missing operation and numbers
            std::string result = processor.processJson(request);
            
            auto jsonResult = json::parse(result);
            REQUIRE(jsonResult["success"] == false);
            REQUIRE(jsonResult.contains("error"));
        }
    }
    
    SECTION("Response structure validation")
    {
        SECTION("All responses have required fields")
        {
            std::vector<std::string> testRequests = {
                R"({"type": "math", "operation": "add", "numbers": [1, 2, 3]})",
                R"({"type": "text", "operation": "uppercase", "text": "test"})",
                R"({"type": "data", "operation": "stats", "dataset": [1, 2, 3]})",
                R"({"type": "echo", "message": "test"})"
            };
            
            for (const auto& request : testRequests) {
                std::string result = processor.processJson(request);
                auto jsonResult = json::parse(result);
                
                // All responses should have these fields
                REQUIRE(jsonResult.contains("success"));
                REQUIRE(jsonResult.contains("timestamp"));
                
                if (jsonResult["success"] == true) {
                    REQUIRE(jsonResult.contains("result"));
                } else {
                    REQUIRE(jsonResult.contains("error"));
                }
            }
        }
    }
}

TEST_CASE("Python Processor Performance", "[python][processor][performance]")
{
    PythonProcessor processor;
    REQUIRE(processor.isInitialized());
    
    SECTION("Multiple sequential requests")
    {
        const int numRequests = 100;
        std::string request = R"({"type": "math", "operation": "add", "numbers": [1, 2, 3]})";
        
        for (int i = 0; i < numRequests; ++i) {
            std::string result = processor.processJson(request);
            auto jsonResult = json::parse(result);
            REQUIRE(jsonResult["success"] == true);
            REQUIRE(jsonResult["result"] == 6);
        }
    }
}

TEST_CASE("Python Processor Thread Safety", "[python][processor][threading]")
{
    PythonProcessor processor;
    REQUIRE(processor.isInitialized());
    
    SECTION("Single processor instance handles multiple requests")
    {
        // Test that the processor can handle requests sequentially
        // (Full thread safety testing would require more complex setup)
        std::vector<std::string> requests = {
            R"({"type": "math", "operation": "add", "numbers": [1, 2]})",
            R"({"type": "text", "operation": "uppercase", "text": "test"})",
            R"({"type": "echo", "message": "test"})"
        };
        
        for (const auto& request : requests) {
            std::string result = processor.processJson(request);
            auto jsonResult = json::parse(result);
            REQUIRE(jsonResult["success"] == true);
        }
    }
}
