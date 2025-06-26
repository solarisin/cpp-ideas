#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "python_processor.h"
#include <any>
#include <string>
#include <vector>
#include <map>

TEST_CASE("PythonProcessor Extended Functionality", "[python][extended]") {
    PythonProcessor processor;
    
    SECTION("Processor lifecycle") {
        REQUIRE(processor.isInitialized());
        
        // Test that processor can be used multiple times
        auto result1 = processor.executeCode("1 + 1");
        auto result2 = processor.executeCode("2 + 2");
        
        REQUIRE(result1.success);
        REQUIRE(result2.success);
        REQUIRE(std::any_cast<int>(result1.value) == 2);
        REQUIRE(std::any_cast<int>(result2.value) == 4);
    }
    
    SECTION("Type conversion comprehensive tests") {
        // Test integer values
        auto intResult = processor.executeCode("42");
        REQUIRE(intResult.success);
        REQUIRE(intResult.actualType == "int");
        REQUIRE(std::any_cast<int>(intResult.value) == 42);
        
        // Test float values
        auto floatResult = processor.executeCode("3.14159");
        REQUIRE(floatResult.success);
        REQUIRE(floatResult.actualType == "float");
        REQUIRE(std::any_cast<double>(floatResult.value) == Catch::Approx(3.14159));
        
        // Test boolean values
        auto boolResult = processor.executeCode("True");
        REQUIRE(boolResult.success);
        REQUIRE(boolResult.actualType == "bool");
        REQUIRE(std::any_cast<bool>(boolResult.value) == true);
        
        auto boolFalseResult = processor.executeCode("False");
        REQUIRE(boolFalseResult.success);
        REQUIRE(boolFalseResult.actualType == "bool");
        REQUIRE(std::any_cast<bool>(boolFalseResult.value) == false);
        
        // Test string values
        auto stringResult = processor.executeCode("'Hello, World!'");
        REQUIRE(stringResult.success);
        REQUIRE(stringResult.actualType == "str");
        REQUIRE(std::any_cast<std::string>(stringResult.value) == "Hello, World!");
        
        // Test None values
        auto noneResult = processor.executeCode("None");
        REQUIRE(noneResult.success);
        REQUIRE(noneResult.actualType == "NoneType");
    }
    
    SECTION("Complex data structures") {
        // Test list with mixed types
        auto listResult = processor.executeCode("[1, 2.5, 'test', True, None]");
        REQUIRE(listResult.success);
        REQUIRE(listResult.actualType == "list");
        REQUIRE(listResult.stringValue == "[1, 2.5, 'test', True, None]");
        
        // Test dictionary
        auto dictResult = processor.executeCode("{'name': 'John', 'age': 30, 'active': True}");
        REQUIRE(dictResult.success);
        REQUIRE(dictResult.actualType == "dict");
        REQUIRE_FALSE(dictResult.stringValue.empty());
        
        // Test nested structures
        auto nestedResult = processor.executeCode("{'users': [{'id': 1, 'name': 'Alice'}, {'id': 2, 'name': 'Bob'}]}");
        REQUIRE(nestedResult.success);
        REQUIRE(nestedResult.actualType == "dict");
    }
    
    SECTION("Mathematical operations and precision") {
        // Test integer arithmetic
        auto addResult = processor.executeCode("123 + 456");
        REQUIRE(addResult.success);
        REQUIRE(std::any_cast<int>(addResult.value) == 579);
        
        // Test floating point arithmetic
        auto floatCalc = processor.executeCode("0.1 + 0.2");
        REQUIRE(floatCalc.success);
        auto floatVal = std::any_cast<double>(floatCalc.value);
        REQUIRE(floatVal == Catch::Approx(0.3).epsilon(0.00001));
        
        // Test large numbers
        auto largeNum = processor.executeCode("2**63 - 1");
        REQUIRE(largeNum.success);
        
        // Test division
        auto divResult = processor.executeCode("10 / 3");
        REQUIRE(divResult.success);
        REQUIRE(divResult.actualType == "float");
    }
    
    SECTION("String manipulation edge cases") {
        // Test empty string
        auto emptyString = processor.executeCode("''");
        REQUIRE(emptyString.success);
        REQUIRE(std::any_cast<std::string>(emptyString.value) == "");
        
        // Test string with special characters
        auto specialChars = processor.executeCode("'Hello\\nWorld\\t!'");
        REQUIRE(specialChars.success);
        REQUIRE(std::any_cast<std::string>(specialChars.value) == "Hello\nWorld\t!");
        
        // Test unicode string
        auto unicodeString = processor.executeCode("'Hello 世界! 🌍'");
        REQUIRE(unicodeString.success);
        REQUIRE(std::any_cast<std::string>(unicodeString.value) == "Hello 世界! 🌍");
        
        // Test string concatenation
        auto concatResult = processor.executeCode("'Hello' + ' ' + 'World'");
        REQUIRE(concatResult.success);
        REQUIRE(std::any_cast<std::string>(concatResult.value) == "Hello World");
    }
    
    SECTION("Function calls with various argument types") {
        // Test accessing a constant (not a function)
        auto piResult = processor.callModuleFunction("math.pi");
        REQUIRE(piResult.success);
        REQUIRE(piResult.actualType == "float");
        REQUIRE(std::any_cast<double>(piResult.value) == Catch::Approx(3.14159).margin(0.001));
        
        // Test function with string argument
        if (processor.canImportModule("demo_module")) {
            auto stringArgResult = processor.callFunction("demo_module", "greet", {std::string("TestUser")});
            REQUIRE(stringArgResult.success);
            REQUIRE(stringArgResult.actualType == "str");
        }
        
        // Test function with multiple numeric arguments
        auto multiArgResult = processor.callFunction("math", "pow", {2.0, 10.0});
        REQUIRE(multiArgResult.success);
        REQUIRE(std::any_cast<double>(multiArgResult.value) == 1024.0);
    }
    
    SECTION("Error handling comprehensive") {
        // Test division by zero
        auto divZeroResult = processor.executeCode("1 / 0");
        REQUIRE_FALSE(divZeroResult.success);
        REQUIRE_FALSE(divZeroResult.error.empty());
        
        // Test undefined variable
        auto undefVarResult = processor.executeCode("undefined_variable");
        REQUIRE_FALSE(undefVarResult.success);
        REQUIRE_FALSE(undefVarResult.error.empty());
        
        // Test syntax error with multiple lines
        auto multiLineSyntaxError = processor.executeCode("if True\n  print('missing colon')");
        REQUIRE_FALSE(multiLineSyntaxError.success);
        
        // Test runtime error in function call
        auto runtimeErrorResult = processor.callFunction("math", "sqrt", {-1});
        REQUIRE_FALSE(runtimeErrorResult.success);
        
        // Test importing non-existent module in executeCode
        auto importErrorResult = processor.executeCode("import non_existent_module_xyz");
        REQUIRE_FALSE(importErrorResult.success);
    }
    
    SECTION("PythonCommand structure validation") {
        // Test command with correct expected type
        PythonCommand cmd1("math", "sqrt", {25.0}, "float");
        auto result1 = processor.executeCommand(cmd1);
        REQUIRE(result1.success);
        REQUIRE(result1.actualType == "float");
        
        // Test command with different expected type (should still work)
        PythonCommand cmd2("math", "sqrt", {36.0}, "int");  // Different type hint
        auto result2 = processor.executeCommand(cmd2);
        REQUIRE(result2.success);
        REQUIRE(result2.actualType == "float");  // Actual type is still float
        
        // Test command with empty arguments
        PythonCommand cmd3("os", "getcwd", {}, "str");
        auto result3 = processor.executeCommand(cmd3);
        REQUIRE(result3.success);
        REQUIRE(result3.actualType == "str");
    }
    
    SECTION("Module function listing edge cases") {
        // Test getting functions from built-in module
        auto builtinFunctions = processor.getModuleFunctions("builtins");
        REQUIRE_FALSE(builtinFunctions.empty());
        
        // Check for common built-in functions
        bool foundLen = false, foundStr = false, foundInt = false;
        for (const auto& func : builtinFunctions) {
            if (func == "len") foundLen = true;
            if (func == "str") foundStr = true;
            if (func == "int") foundInt = true;
        }
        REQUIRE(foundLen);
        REQUIRE(foundStr);
        REQUIRE(foundInt);
        
        // Test getting functions from non-existent module
        auto noFunctions = processor.getModuleFunctions("definitely_not_a_module");
        REQUIRE(noFunctions.empty());
    }
    
    SECTION("Expression vs Statement execution") {
        // Test expression evaluation
        auto exprResult = processor.evaluateExpression("2 + 3 * 4");
        REQUIRE(exprResult.success);
        REQUIRE(std::any_cast<int>(exprResult.value) == 14);
        
        // Test statement execution (should return None for assignments)
        auto stmtResult = processor.executeStatement("x = 42");
        REQUIRE(stmtResult.success);
        REQUIRE(stmtResult.actualType == "NoneType");
        
        // Test executeCode with expression
        auto codeExprResult = processor.executeCode("5 * 6");
        REQUIRE(codeExprResult.success);
        REQUIRE(std::any_cast<int>(codeExprResult.value) == 30);
        
        // Test executeCode with statement
        auto codeStmtResult = processor.executeCode("y = 'hello'");
        REQUIRE(codeStmtResult.success);
        REQUIRE(codeStmtResult.actualType == "NoneType");
    }
    
    SECTION("Memory and performance considerations") {
        // Test repeated function calls don't leak memory significantly
        for (int i = 0; i < 100; ++i) {
            auto result = processor.executeCode("2 + 2");
            REQUIRE(result.success);
            REQUIRE(std::any_cast<int>(result.value) == 4);
        }
        
        // Test calling different functions in sequence
        std::vector<std::string> expressions = {
            "1 + 1",
            "'test'.upper()",
            "len([1, 2, 3])",
            "abs(-42)",
            "max([1, 5, 3])"
        };
        
        for (const auto& expr : expressions) {
            auto result = processor.executeCode(expr);
            REQUIRE(result.success);
        }
    }
    
    SECTION("Backward compatibility validation") {
        // Test that original JSON processing still works
        std::string jsonInput = R"({
            "type": "math",
            "operation": "add",
            "numbers": [1, 2, 3]
        })";
        
        auto jsonResult = processor.processJson(jsonInput);
        REQUIRE_FALSE(jsonResult.empty());
        
        // Parse result to verify it contains expected fields
        REQUIRE(jsonResult.find("success") != std::string::npos);
        REQUIRE(jsonResult.find("result") != std::string::npos);
    }
}

TEST_CASE("PythonProcessor Thread Safety Basics", "[python][threading]") {
    // Note: This is a basic test since the processor uses GIL properly
    PythonProcessor processor;
    
    SECTION("Sequential operations maintain state isolation") {
        // Each operation should be independent
        auto result1 = processor.executeCode("x = 10; x");
        auto result2 = processor.executeCode("y = 20; y");
        
        REQUIRE(result1.success);
        REQUIRE(result2.success);
        
        // Debug output to see what types we're getting
        INFO("result1.actualType: " << result1.actualType);
        INFO("result1.stringValue: " << result1.stringValue);
        INFO("result2.actualType: " << result2.actualType);
        INFO("result2.stringValue: " << result2.stringValue);
        
        REQUIRE(std::any_cast<int>(result1.value) == 10);
        REQUIRE(std::any_cast<int>(result2.value) == 20);
        
        // Variables from previous operations should not persist
        auto result3 = processor.executeCode("x");  // Should fail - x not defined in this context
        REQUIRE_FALSE(result3.success);
    }
}
