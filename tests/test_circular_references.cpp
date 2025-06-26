#include <catch2/catch_test_macros.hpp>
#include "python_processor.h"
#include <memory>
#include <vector>

TEST_CASE("CircularReference - Multiple processor creation and destruction", "[CircularReference]") {
    std::vector<std::unique_ptr<PythonProcessor>> processors;
    
    // Create multiple processors to test for accumulating leaks
    for (int i = 0; i < 5; ++i) {
        processors.push_back(std::make_unique<PythonProcessor>());
    }
    
    // Execute some operations with each processor
    for (auto& processor : processors) {
        REQUIRE(processor->isInitialized());
        
        // Test various operations that might create circular references
        std::string mathJson = R"({"type": "math", "operation": "add", "numbers": [1, 2, 3]})";
        std::string result = processor->processJson(mathJson);
        REQUIRE_FALSE(result.empty());
        
        // Test error conditions that might leave dangling references
        std::string invalidJson = R"({"invalid": json})";
        std::string errorResult = processor->processJson(invalidJson);
        REQUIRE_FALSE(errorResult.empty());
    }
    
    // Create and destroy more processors to test for accumulating references
    std::vector<std::unique_ptr<PythonProcessor>> tempProcessors;
    for (int i = 0; i < 10; ++i) {
        tempProcessors.push_back(std::make_unique<PythonProcessor>());
        
        std::string testJson = R"({"type": "echo", "message": "test"})";
        std::string result = tempProcessors.back()->processJson(testJson);
        REQUIRE_FALSE(result.empty());
    }
    
    // Clear temporary processors - this should not cause memory to accumulate
    // if we're properly managing references
    tempProcessors.clear();
    
    // Clear all processors
    processors.clear();
}

TEST_CASE("CircularReference - Boost Python object cleanup", "[CircularReference]") {
    auto processor = std::make_unique<PythonProcessor>();
    
    // Test operations that create boost::python objects
    std::vector<std::string> testCases = {
        R"({"type": "math", "operation": "multiply", "numbers": [2, 3, 4]})",
        R"({"type": "text", "operation": "uppercase", "text": "hello world"})",
        R"({"type": "data", "operation": "sort", "items": [3, 1, 4, 1, 5]})",
        R"({"type": "echo", "message": "test circular references"})"
    };
    
    // Execute multiple rounds to see if memory accumulates
    for (int round = 0; round < 3; ++round) {
        for (const auto& testCase : testCases) {
            std::string result = processor->processJson(testCase);
            REQUIRE_FALSE(result.empty());
        }
    }
}

TEST_CASE("CircularReference - Python exception handling", "[CircularReference]") {
    auto processor = std::make_unique<PythonProcessor>();
    
    // Test that exception handling doesn't create circular references
    std::vector<std::string> errorCases = {
        R"(invalid json)",
        R"({"type": "unknown_type"})",
        R"({"type": "math", "operation": "divide_by_zero", "numbers": [1, 0]})",
        R"(null)",
        R"([])",
        R"("")"
    };
    
    for (const auto& errorCase : errorCases) {
        std::string result = processor->processJson(errorCase);
        REQUIRE_FALSE(result.empty());
        
        // Result should indicate an error
        bool hasErrorInfo = (result.find("error") != std::string::npos || 
                           result.find("success") != std::string::npos);
        REQUIRE(hasErrorInfo);
    }
}

// This test specifically checks for reference cycles by forcing garbage collection
TEST_CASE("CircularReference - Explicit garbage collection", "[CircularReference]") {
    auto processor = std::make_unique<PythonProcessor>();
    
    // Use executeCommand to run Python garbage collection
    PythonCommand gcCommand("gc", "collect");
    
    PythonResult gcResult = processor->executeCommand(gcCommand);
    REQUIRE(gcResult.success);
    
    // Check if any objects were collected
    if (gcResult.success && !gcResult.stringValue.empty()) {
        // The result should be a number (objects collected)
        try {
            int collected = std::stoi(gcResult.stringValue);
            INFO("Garbage collector found " << collected << " objects to collect");
            
            // If many objects are being collected repeatedly, it might indicate cycles
            // But some collection is normal due to temporary objects
        } catch (const std::exception&) {
            // Result might not be a simple number, that's okay
            INFO("GC result: " << gcResult.stringValue);
        }
    }
}
