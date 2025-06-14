#include <iostream>
#include <string>
#include "python_processor.h"
#include <nlohmann/json.hpp>

int main() {
    std::cout << "Qt JSON Python Processor - Console Test\n";
    std::cout << "========================================\n\n";
    
    // Initialize Python processor
    PythonProcessor processor;
    
    if (!processor.isInitialized()) {
        std::cout << "Error: Python processor failed to initialize\n";
        std::cout << "Error details: " << processor.getLastError() << "\n";
        return 1;
    }
    
    std::cout << "Python processor initialized successfully!\n\n";
    
    // Test different JSON requests
    std::vector<std::string> testRequests = {
        R"({"type": "math", "operation": "add", "numbers": [1, 2, 3, 4, 5]})",
        R"({"type": "text", "operation": "uppercase", "text": "hello world from qt!"})",
        R"({"type": "data", "operation": "stats", "dataset": [10, 20, 30, 40, 50, 25, 35, 45]})",
        R"({"type": "echo", "message": "This is a test from Qt application"})"
    };
    
    for (size_t i = 0; i < testRequests.size(); ++i) {
        std::cout << "--- Test " << (i + 1) << " ---\n";
        std::cout << "Input: " << testRequests[i] << "\n";
        
        std::string result = processor.processJson(testRequests[i]);
        
        // Pretty print JSON result
        try {
            auto jsonDoc = nlohmann::json::parse(result);
            std::cout << "Output: " << jsonDoc.dump(2) << "\n\n";
        } catch (const std::exception&) {
            std::cout << "Output: " << result << "\n\n";
        }
    }
    
    std::cout << "All tests completed successfully!\n";
    return 0;
}
