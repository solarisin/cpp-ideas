#include "python_processor.h"
#include <nlohmann/json.hpp>
#include <loguru/loguru.hpp>
#include <iostream>
#include <any>
#include <vector>

void printResult(const PythonResult& result, const std::string& description) {
    std::cout << "\n=== " << description << " ===" << std::endl;
    std::cout << "Success: " << (result.success ? "true" : "false") << std::endl;
    
    if (result.success) {
        std::cout << "Result Type: " << result.actualType << std::endl;
        std::cout << "String Value: " << result.stringValue << std::endl;
        
        // Try to extract specific types
        try {
            if (auto strPtr = std::any_cast<std::string>(&result.value)) {
                std::cout << "String Result: \"" << *strPtr << "\"" << std::endl;
            } else if (auto intPtr = std::any_cast<int>(&result.value)) {
                std::cout << "Integer Result: " << *intPtr << std::endl;
            } else if (auto doublePtr = std::any_cast<double>(&result.value)) {
                std::cout << "Double Result: " << *doublePtr << std::endl;
            } else if (auto boolPtr = std::any_cast<bool>(&result.value)) {
                std::cout << "Boolean Result: " << (*boolPtr ? "true" : "false") << std::endl;
            }
        } catch (const std::bad_any_cast&) {
            std::cout << "Raw Value: [complex type]" << std::endl;
        }
    } else {
        std::cout << "Error: " << result.error << std::endl;
    }
    std::cout << std::endl;
}

void demonstrateGenericCommands() {
    std::cout << "=== Python Generic Command Demonstration ===" << std::endl;
    
    PythonProcessor processor;
    
    if (!processor.isInitialized()) {
        std::cerr << "Failed to initialize Python processor: " << processor.getLastError() << std::endl;
        return;
    }
    
    std::cout << "Python processor initialized successfully!" << std::endl;
    
    // 1. Test module import capability
    std::cout << "\n--- Testing Module Import ---" << std::endl;
    std::vector<std::string> modules = {"math", "os", "sys", "demo_module", "processor"};
    for (const auto& module : modules) {
        bool canImport = processor.canImportModule(module);
        std::cout << "Can import '" << module << "': " << (canImport ? "YES" : "NO") << std::endl;
        
        if (canImport) {
            auto functions = processor.getModuleFunctions(module);
            std::cout << "  Available functions (" << functions.size() << "): ";
            for (size_t i = 0; i < std::min(size_t(5), functions.size()); ++i) {
                std::cout << functions[i];
                if (i < std::min(size_t(4), functions.size() - 1)) std::cout << ", ";
            }
            if (functions.size() > 5) std::cout << "...";
            std::cout << std::endl;
        }
    }
    
    // 2. Test simple function calls with different argument types
    std::cout << "\n--- Testing Simple Function Calls ---" << std::endl;
    
    // String function
    auto result1 = processor.callFunction("demo_module", "greet", {std::string("World")});
    printResult(result1, "demo_module.greet('World')");
    
    // Function with multiple string arguments
    auto result2 = processor.callFunction("demo_module", "greet", {
        std::string("Smith"), std::string("Dr.")
    });
    printResult(result2, "demo_module.greet('Smith', 'Dr.')");
    
    // Math function with numbers
    auto result3 = processor.callFunction("demo_module", "add_numbers", {5.5, 3.2});
    printResult(result3, "demo_module.add_numbers(5.5, 3.2)");
    
    // Function returning integer
    auto result4 = processor.callFunction("demo_module", "fibonacci", {8});
    printResult(result4, "demo_module.fibonacci(8)");
    
    // 3. Test math module functions
    std::cout << "\n--- Testing Built-in Math Module ---" << std::endl;
    
    auto result5 = processor.callFunction("math", "sqrt", {16.0});
    printResult(result5, "math.sqrt(16.0)");
    
    auto result6 = processor.callFunction("math", "pow", {2.0, 8.0});
    printResult(result6, "math.pow(2.0, 8.0)");
    
    auto result7 = processor.callFunction("math", "factorial", {5});
    printResult(result7, "math.factorial(5)");
    
    // 4. Test module.function syntax
    std::cout << "\n--- Testing Module.Function Syntax ---" << std::endl;
    
    auto result8 = processor.callModuleFunction("demo_module.get_math_constants");
    printResult(result8, "demo_module.get_math_constants()");
    
    auto result9 = processor.callModuleFunction("os.getcwd");
    printResult(result9, "os.getcwd()");
    
    // 5. Test PythonCommand structure
    std::cout << "\n--- Testing PythonCommand Structure ---" << std::endl;
    
    PythonCommand cmd1("demo_module", "execute_math_operation", {
        std::string("add"), 1.5, 2.5, 3.0, 4.0
    }, "float");
    auto result10 = processor.executeCommand(cmd1);
    printResult(result10, "PythonCommand: execute_math_operation('add', 1.5, 2.5, 3.0, 4.0)");
    
    PythonCommand cmd2("demo_module", "get_system_info", {}, "dict");
    auto result11 = processor.executeCommand(cmd2);
    printResult(result11, "PythonCommand: get_system_info() -> dict");
    
    // 6. Test direct code execution
    std::cout << "\n--- Testing Direct Code Execution ---" << std::endl;
    
    auto result13 = processor.executeCode("sum([1, 2, 3, 4, 5])");
    printResult(result13, "Direct code: sum([1, 2, 3, 4, 5])");
    
    auto result14 = processor.executeCode("list(range(10))");
    printResult(result14, "Direct code: list(range(10))");
    
    auto result15 = processor.executeCode("{'name': 'test', 'value': 42}");
    printResult(result15, "Direct code: create dictionary");
    
    // Note: datetime module execution skipped due to crash in this context
    std::cout << "Note: Skipping datetime test due to known issue with boost::python\n";
    
    // 7. Test error handling
    std::cout << "\n--- Testing Error Handling ---" << std::endl;
    
    // Non-existent module
    auto error1 = processor.callFunction("non_existent_module", "some_function");
    printResult(error1, "Error: Non-existent module");
    
    // Non-existent function
    auto error2 = processor.callFunction("math", "non_existent_function");
    printResult(error2, "Error: Non-existent function");
    
    // Invalid arguments
    auto error3 = processor.callFunction("math", "sqrt", {std::string("not_a_number")});
    printResult(error3, "Error: Invalid argument type");
    
    // Code with syntax error
    auto error4 = processor.executeCode("invalid python syntax !!!");
    printResult(error4, "Error: Invalid Python syntax");
    
    // 8. Test original JSON processing for backward compatibility
    std::cout << "\n--- Testing Backward Compatibility ---" << std::endl;
    
    std::string jsonInput = R"({
        "type": "math",
        "operation": "add",
        "numbers": [10, 20, 30, 40, 50]
    })";
    
    std::string jsonResult = processor.processJson(jsonInput);
    std::cout << "Original JSON processing:" << std::endl;
    std::cout << "Input: " << jsonInput << std::endl;
    std::cout << "Output: " << jsonResult << std::endl;
}

int main(int argc, char* argv[]) {
    // Initialize logging
    loguru::init(argc, argv);
    loguru::g_stderr_verbosity = 1; // Show INFO level and above
    
    std::cout << "Starting Python Generic Command Demo..." << std::endl;
    
    try {
        demonstrateGenericCommands();
        std::cout << "\nDemo completed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
