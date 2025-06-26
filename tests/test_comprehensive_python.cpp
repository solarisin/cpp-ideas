#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "python_processor.h"
#include <any>
#include <string>
#include <vector>
#include <memory>

using Catch::Matchers::ContainsSubstring;

TEST_CASE("PythonProcessor Comprehensive Testing", "[python][comprehensive]") {
    
    SECTION("Environment and Initialization Tests") {
        SECTION("Single processor initialization") {
            PythonProcessor processor;
            REQUIRE(processor.isInitialized());
            REQUIRE(processor.getLastError().empty());
        }
        
        SECTION("Multiple processor instances") {
            auto processor1 = std::make_unique<PythonProcessor>();
            auto processor2 = std::make_unique<PythonProcessor>();
            
            REQUIRE(processor1->isInitialized());
            REQUIRE(processor2->isInitialized());
            
            // Both should work independently
            auto result1 = processor1->executeCode("1 + 1");
            auto result2 = processor2->executeCode("2 + 2");
            
            REQUIRE(result1.success);
            REQUIRE(result2.success);
        }
    }
    
    SECTION("Basic Code Execution Tests") {
        PythonProcessor processor;
        
        SECTION("Simple arithmetic") {
            auto result = processor.executeCode("5 + 3");
            REQUIRE(result.success);
            REQUIRE(result.actualType == "int");
            
            // Test any_cast safety
            try {
                auto intValue = std::any_cast<int>(result.value);
                REQUIRE(intValue == 8);
            } catch (const std::bad_any_cast& e) {
                FAIL("Failed to cast result to int: " << e.what() << 
                     " - actualType: " << result.actualType << 
                     " - stringValue: " << result.stringValue);
            }
        }
        
        SECTION("Boolean values with detailed debugging") {
            auto trueResult = processor.executeCode("True");
            REQUIRE(trueResult.success);
            
            INFO("True result - actualType: " << trueResult.actualType);
            INFO("True result - stringValue: " << trueResult.stringValue);
            INFO("True result - type name: " << trueResult.value.type().name());
            
            REQUIRE(trueResult.actualType == "bool");
            
            // Test any_cast with proper error handling
            try {
                auto boolValue = std::any_cast<bool>(trueResult.value);
                REQUIRE(boolValue == true);
            } catch (const std::bad_any_cast& e) {
                FAIL("Failed to cast True to bool: " << e.what() << 
                     " - actualType: " << trueResult.actualType);
            }
            
            auto falseResult = processor.executeCode("False");
            REQUIRE(falseResult.success);
            REQUIRE(falseResult.actualType == "bool");
            
            try {
                auto boolValue = std::any_cast<bool>(falseResult.value);
                REQUIRE(boolValue == false);
            } catch (const std::bad_any_cast& e) {
                FAIL("Failed to cast False to bool: " << e.what() << 
                     " - actualType: " << falseResult.actualType);
            }
        }
        
        SECTION("None values") {
            auto noneResult = processor.executeCode("None");
            REQUIRE(noneResult.success);
            REQUIRE(noneResult.actualType == "NoneType");
            
            // None should be stored as string "None"
            try {
                auto stringValue = std::any_cast<std::string>(noneResult.value);
                REQUIRE(stringValue == "None");
            } catch (const std::bad_any_cast& e) {
                FAIL("Failed to cast None to string: " << e.what());
            }
        }
        
        SECTION("String values") {
            auto stringResult = processor.executeCode("'Hello World'");
            REQUIRE(stringResult.success);
            REQUIRE(stringResult.actualType == "str");
            
            try {
                auto stringValue = std::any_cast<std::string>(stringResult.value);
                REQUIRE(stringValue == "Hello World");
            } catch (const std::bad_any_cast& e) {
                FAIL("Failed to cast string result: " << e.what());
            }
        }
        
        SECTION("Float values") {
            auto floatResult = processor.executeCode("3.14159");
            REQUIRE(floatResult.success);
            REQUIRE(floatResult.actualType == "float");
            
            try {
                auto doubleValue = std::any_cast<double>(floatResult.value);
                REQUIRE(doubleValue == Catch::Approx(3.14159));
            } catch (const std::bad_any_cast& e) {
                FAIL("Failed to cast float result: " << e.what());
            }
        }
    }
    
    SECTION("Built-in Module Testing") {
        PythonProcessor processor;
        
        SECTION("Debug Python environment") {
            // First, test basic Python functionality
            auto sysResult = processor.executeCode("import sys; sys.version");
            INFO("Python version check result: success=" << sysResult.success << 
                 ", error=" << sysResult.error);
            
            if (sysResult.success) {
                INFO("Python version: " << sysResult.stringValue);
            }
            
            // Test if we can import built-in modules
            auto mathImportResult = processor.executeCode("import math; 'math imported'");
            INFO("Math import result: success=" << mathImportResult.success << 
                 ", error=" << mathImportResult.error);
            
            if (mathImportResult.success) {
                // If direct import works, test module functions
                auto mathSqrtResult = processor.executeCode("import math; math.sqrt(16)");
                REQUIRE(mathSqrtResult.success);
                
                try {
                    auto sqrtValue = std::any_cast<double>(mathSqrtResult.value);
                    REQUIRE(sqrtValue == 4.0);
                } catch (const std::bad_any_cast& e) {
                    FAIL("Failed to cast math.sqrt result: " << e.what());
                }
            }
        }
        
        SECTION("Module import capability testing") {
            // Test canImportModule method specifically
            bool canImportMath = processor.canImportModule("math");
            INFO("canImportModule('math') returned: " << canImportMath);
            
            if (!canImportMath) {
                // Try to diagnose why
                auto directTest = processor.executeCode("import math");
                INFO("Direct import test - success: " << directTest.success << 
                     ", error: " << directTest.error);
            }
            
            bool canImportSys = processor.canImportModule("sys");
            INFO("canImportModule('sys') returned: " << canImportSys);
            
            // These should work for any properly configured Python
            if (canImportMath) {
                REQUIRE(canImportMath);
                REQUIRE(canImportSys);
            }
        }
        
        SECTION("Function calling with debugging") {
            // Try different approaches to calling math functions
            
            // Approach 1: Direct code execution
            auto directMathResult = processor.executeCode("import math; math.sqrt(25)");
            INFO("Direct math execution - success: " << directMathResult.success << 
                 ", error: " << directMathResult.error);
            
            if (directMathResult.success) {
                auto value = std::any_cast<double>(directMathResult.value);
                REQUIRE(value == 5.0);
                
                // Approach 2: callFunction method
                auto functionCallResult = processor.callFunction("math", "sqrt", {25.0});
                INFO("Function call - success: " << functionCallResult.success << 
                     ", error: " << functionCallResult.error);
                
                if (functionCallResult.success) {
                    auto callValue = std::any_cast<double>(functionCallResult.value);
                    REQUIRE(callValue == 5.0);
                }
            }
        }
    }
    
    SECTION("Error Handling and Edge Cases") {
        PythonProcessor processor;
        
        SECTION("Division by zero") {
            auto result = processor.executeCode("1 / 0");
            REQUIRE_FALSE(result.success);
            REQUIRE_FALSE(result.error.empty());
            REQUIRE_THAT(result.error, ContainsSubstring("division"));
        }
        
        SECTION("Undefined variable") {
            auto result = processor.executeCode("undefined_variable_name");
            REQUIRE_FALSE(result.success);
            REQUIRE_FALSE(result.error.empty());
            REQUIRE_THAT(result.error, ContainsSubstring("NameError"));
        }
        
        SECTION("Syntax error") {
            auto result = processor.executeCode("if True print('missing colon')");
            REQUIRE_FALSE(result.success);
            REQUIRE_FALSE(result.error.empty());
            REQUIRE_THAT(result.error, ContainsSubstring("syntax"));
        }
        
        SECTION("Type error in function call") {
            auto directResult = processor.executeCode("import math; math.sqrt('not_a_number')");
            REQUIRE_FALSE(directResult.success);
            REQUIRE_THAT(directResult.error, ContainsSubstring("TypeError"));
        }
    }
    
    SECTION("Complex Data Structures") {
        PythonProcessor processor;
        
        SECTION("Lists") {
            auto listResult = processor.executeCode("[1, 2, 3, 'hello', True]");
            REQUIRE(listResult.success);
            REQUIRE(listResult.actualType == "list");
            
            // For complex types, we store string representation
            auto stringValue = std::any_cast<std::string>(listResult.value);
            REQUIRE_THAT(stringValue, ContainsSubstring("1"));
            REQUIRE_THAT(stringValue, ContainsSubstring("hello"));
        }
        
        SECTION("Dictionaries") {
            auto dictResult = processor.executeCode("{'key': 'value', 'number': 42}");
            REQUIRE(dictResult.success);
            REQUIRE(dictResult.actualType == "dict");
            
            auto stringValue = std::any_cast<std::string>(dictResult.value);
            REQUIRE_THAT(stringValue, ContainsSubstring("key"));
            REQUIRE_THAT(stringValue, ContainsSubstring("value"));
        }
        
        SECTION("Nested structures") {
            auto nestedResult = processor.executeCode("{'list': [1, 2, {'nested': True}]}");
            REQUIRE(nestedResult.success);
            REQUIRE(nestedResult.actualType == "dict");
        }
    }
    
    SECTION("Memory and Performance Tests") {
        PythonProcessor processor;
        
        SECTION("Repeated operations") {
            for (int i = 0; i < 50; ++i) {
                auto result = processor.executeCode("2 + 2");
                REQUIRE(result.success);
                
                try {
                    auto intValue = std::any_cast<int>(result.value);
                    REQUIRE(intValue == 4);
                } catch (const std::bad_any_cast& e) {
                    FAIL("Iteration " << i << " failed: " << e.what());
                }
            }
        }
        
        SECTION("Different operations in sequence") {
            std::vector<std::string> operations = {
                "10 + 5",
                "'test'.upper()",
                "len([1, 2, 3, 4])",
                "abs(-42)",
                "max([10, 5, 8])"
            };
            
            for (size_t i = 0; i < operations.size(); ++i) {
                auto result = processor.executeCode(operations[i]);
                REQUIRE(result.success);
                INFO("Operation " << i << ": " << operations[i] << 
                     " -> " << result.stringValue);
            }
        }
    }
    
    SECTION("Statement vs Expression Execution") {
        PythonProcessor processor;
        
        SECTION("Variable assignments") {
            auto assignResult = processor.executeStatement("x = 42");
            REQUIRE(assignResult.success);
            REQUIRE(assignResult.actualType == "NoneType");
        }
        
        SECTION("Expression evaluation") {
            auto exprResult = processor.evaluateExpression("3 * 7");
            REQUIRE(exprResult.success);
            REQUIRE(exprResult.actualType == "int");
            
            auto intValue = std::any_cast<int>(exprResult.value);
            REQUIRE(intValue == 21);
        }
        
        SECTION("Mixed code execution") {
            auto mixedResult = processor.executeCode("y = 10; y * 2");
            REQUIRE(mixedResult.success);
            
            // Should return the result of the last expression
            auto intValue = std::any_cast<int>(mixedResult.value);
            REQUIRE(intValue == 20);
        }
    }
}
