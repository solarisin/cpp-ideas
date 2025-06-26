#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "python_processor.h"
#include <any>
#include <string>
#include <vector>

TEST_CASE("PythonProcessor Generic Commands", "[python][generic]") {
    PythonProcessor processor;
    
    SECTION("Processor initialization") {
        REQUIRE(processor.isInitialized());
    }
    
    SECTION("Module import capability") {
        // Test standard modules
        REQUIRE(processor.canImportModule("math"));
        REQUIRE(processor.canImportModule("os"));
        REQUIRE(processor.canImportModule("sys"));
        
        // Test non-existent module
        REQUIRE_FALSE(processor.canImportModule("definitely_not_a_real_module"));
    }
    
    SECTION("Function listing") {
        auto mathFunctions = processor.getModuleFunctions("math");
        REQUIRE_FALSE(mathFunctions.empty());
        
        // Check for some common math functions
        bool foundSqrt = false, foundPow = false;
        for (const auto& func : mathFunctions) {
            if (func == "sqrt") foundSqrt = true;
            if (func == "pow") foundPow = true;
        }
        REQUIRE(foundSqrt);
        REQUIRE(foundPow);
    }
    
    SECTION("Simple function calls") {
        // Test math.sqrt
        auto result = processor.callFunction("math", "sqrt", {16.0});
        REQUIRE(result.success);
        REQUIRE(result.actualType == "float");
        
        auto floatValue = std::any_cast<double>(result.value);
        REQUIRE(floatValue == 4.0);
    }
    
    SECTION("Function calls with multiple arguments") {
        // Test math.pow
        auto result = processor.callFunction("math", "pow", {2.0, 8.0});
        REQUIRE(result.success);
        REQUIRE(result.actualType == "float");
        
        auto floatValue = std::any_cast<double>(result.value);
        REQUIRE(floatValue == 256.0);
    }
    
    SECTION("Module.function syntax") {
        auto result = processor.callModuleFunction("os.getcwd");
        REQUIRE(result.success);
        REQUIRE(result.actualType == "str");
        
        auto stringValue = std::any_cast<std::string>(result.value);
        REQUIRE_FALSE(stringValue.empty());
    }
    
    SECTION("PythonCommand structure") {
        PythonCommand cmd("math", "factorial", {5}, "int");
        auto result = processor.executeCommand(cmd);
        
        REQUIRE(result.success);
        REQUIRE(result.actualType == "int");
        
        auto intValue = std::any_cast<int>(result.value);
        REQUIRE(intValue == 120);
    }
    
    SECTION("Direct code execution") {
        auto result = processor.executeCode("2 + 3");
        REQUIRE(result.success);
        REQUIRE(result.actualType == "int");
        
        auto intValue = std::any_cast<int>(result.value);
        REQUIRE(intValue == 5);
    }
    
    SECTION("String operations") {
        auto result = processor.executeCode("'hello'.upper()");
        REQUIRE(result.success);
        REQUIRE(result.actualType == "str");
        
        auto stringValue = std::any_cast<std::string>(result.value);
        REQUIRE(stringValue == "HELLO");
    }
    
    SECTION("List operations") {
        auto result = processor.executeCode("list(range(5))");
        REQUIRE(result.success);
        REQUIRE(result.actualType == "list");
        REQUIRE(result.stringValue == "[0, 1, 2, 3, 4]");
    }
    
    SECTION("Error handling - non-existent module") {
        auto result = processor.callFunction("non_existent_module", "some_function");
        REQUIRE_FALSE(result.success);
        REQUIRE_FALSE(result.error.empty());
    }
    
    SECTION("Error handling - non-existent function") {
        auto result = processor.callFunction("math", "non_existent_function");
        REQUIRE_FALSE(result.success);
        REQUIRE_FALSE(result.error.empty());
    }
    
    SECTION("Error handling - invalid arguments") {
        auto result = processor.callFunction("math", "sqrt", {std::string("not_a_number")});
        REQUIRE_FALSE(result.success);
        REQUIRE_FALSE(result.error.empty());
    }
    
    SECTION("Error handling - syntax error in code") {
        auto result = processor.executeCode("invalid python syntax !!!");
        REQUIRE_FALSE(result.success);
        REQUIRE_FALSE(result.error.empty());
    }
    
    SECTION("Demo module functions - if available") {
        if (processor.canImportModule("demo_module")) {
            // Test simple greeting function
            auto result = processor.callFunction("demo_module", "greet", {std::string("Test")});
            REQUIRE(result.success);
            REQUIRE(result.actualType == "str");
            
            auto greeting = std::any_cast<std::string>(result.value);
            REQUIRE(greeting == "Hello, Test!");
            
            // Test add_numbers function
            auto result2 = processor.callFunction("demo_module", "add_numbers", {5.5, 3.2});
            REQUIRE(result2.success);
            REQUIRE(result2.actualType == "float");
            
            auto sum = std::any_cast<double>(result2.value);
            REQUIRE(sum == Catch::Approx(8.7));
        }
    }
}
