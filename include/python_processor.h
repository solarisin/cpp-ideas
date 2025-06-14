#pragma once

#include <string>
#include <memory>
#include <stdexcept>

namespace py {
    class Exception : public std::runtime_error {
    public:
        explicit Exception(const std::string& message) : std::runtime_error(message) {}
    };
}

class PythonProcessor {
public:
    PythonProcessor();
    ~PythonProcessor();
    
    // Process JSON string through Python script and return result
    std::string processJson(const std::string& jsonInput);
    
    // Check if Python environment is properly initialized
    bool isInitialized() const;
    
    // Get last error message
    std::string getLastError() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};
