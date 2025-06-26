#pragma once

#include <string>
#include <memory>
#include <stdexcept>
#include <vector>
#include <map>
#include <any>

namespace py {
    class Exception : public std::runtime_error {
    public:
        explicit Exception(const std::string& message) : std::runtime_error(message) {}
    };
}

// Structure to define a generic Python command
struct PythonCommand {
    std::string module;        // Python module name (e.g., "processor", "math", "os.path")
    std::string function;      // Function name to call
    std::vector<std::any> args; // Arguments to pass to the function
    std::string expectedType;  // Expected return type: "string", "int", "float", "bool", "list", "dict", "any"
    
    // Constructor for convenience
    PythonCommand(const std::string& mod, const std::string& func, 
                  const std::vector<std::any>& arguments = {}, 
                  const std::string& retType = "any") 
        : module(mod), function(func), args(arguments), expectedType(retType) {}
};

// Result structure for generic command execution
struct PythonResult {
    bool success;
    std::any value;           // The actual return value
    std::string stringValue;  // String representation of the value
    std::string error;        // Error message if success is false
    std::string actualType;   // Actual type of the returned value
};

class PythonProcessor {
public:
    PythonProcessor();
    ~PythonProcessor();
    
    // Process JSON string through Python script and return result
    std::string processJson(const std::string& jsonInput);
    
    // New generic command execution methods
    PythonResult executeCommand(const PythonCommand& command);
    
    // Convenience methods for common use cases
    PythonResult callFunction(const std::string& module, const std::string& function, 
                             const std::vector<std::any>& args = {});
    
    PythonResult callModuleFunction(const std::string& moduleDotFunction, 
                                   const std::vector<std::any>& args = {});
    
    // Execute Python code directly
    PythonResult executeCode(const std::string& pythonCode);
    
    // Evaluate Python expression (returns result)
    PythonResult evaluateExpression(const std::string& expression);
    
    // Execute Python statement (returns None/success indicator)
    PythonResult executeStatement(const std::string& statement);
    
    // Check if a module can be imported
    bool canImportModule(const std::string& module);
    
    // List available functions in a module
    std::vector<std::string> getModuleFunctions(const std::string& module);
    
    // Check if Python environment is properly initialized
    bool isInitialized() const;
    
    // Get last error message
    std::string getLastError() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};
