#include "python_processor.h"
#include <boost/python.hpp>
#include <sstream>
#include <filesystem>
#include <loguru/loguru.hpp>
#include <mutex>
#include <memory>

namespace bp = boost::python;

// RAII helper for GIL management
class GILGuard {
public:
    GILGuard() : state(PyGILState_Ensure()) {}
    ~GILGuard() { PyGILState_Release(state); }
    
    // Non-copyable, non-movable
    GILGuard(const GILGuard&) = delete;
    GILGuard& operator=(const GILGuard&) = delete;
    GILGuard(GILGuard&&) = delete;
    GILGuard& operator=(GILGuard&&) = delete;
    
private:
    PyGILState_STATE state;
};

// Helper class for creating fresh Python namespaces
class NamespaceHelper {
public:
    static bp::dict createFreshNamespace() {
        bp::dict fresh_namespace;
        bp::object builtins = bp::import("builtins");
        fresh_namespace["__builtins__"] = builtins;
        return fresh_namespace;
    }
};

// Singleton Python manager to handle interpreter lifetime
class PythonManager {
private:
    static std::unique_ptr<PythonManager> instance;
    static std::mutex initMutex;
    bool initialized = false;
    std::string lastError;

    PythonManager() = default;

public:
    static PythonManager& getInstance() {
        std::lock_guard<std::mutex> lock(initMutex);
        if (!instance) {
            instance = std::unique_ptr<PythonManager>(new PythonManager());
        }
        return *instance;
    }

    bool initializePython() {
        std::lock_guard<std::mutex> lock(initMutex);
        
        if (initialized) {
            return true; // Already initialized
        }

        try {
            if (Py_IsInitialized()) {
                LOG_F(INFO, "Python already initialized by another component");
                initialized = true;
                return true;
            }

            LOG_F(INFO, "Starting Python interpreter initialization...");

            PyConfig config;
            PyConfig_InitPythonConfig(&config);

            auto currentPath = std::filesystem::current_path();
            auto venvPath = currentPath / ".venv";
            
            LOG_F(INFO, "Current path: %s", currentPath.string().c_str());
            LOG_F(INFO, "Venv path: %s", venvPath.string().c_str());

            config.isolated = 0;
            config.use_environment = 1;

            LOG_F(INFO, "Initializing Python from config...");
            PyStatus status = Py_InitializeFromConfig(&config);
            PyConfig_Clear(&config);
            
            if (PyStatus_Exception(status)) {
                lastError = "Failed to initialize Python: " + std::string(status.err_msg ? status.err_msg : "Unknown error");
                LOG_F(ERROR, "%s", lastError.c_str());
                return false;
            }

            LOG_F(INFO, "Python initialized successfully");

            // Set up Python paths
            PyGILState_STATE gstate = PyGILState_Ensure();
            
            try {
                LOG_F(INFO, "Setting up Python paths...");

                bp::object sys_module = bp::import("sys");
                bp::list sys_path = bp::extract<bp::list>(sys_module.attr("path"));

                // Helper lambda for adding paths
                auto addPathIfExists = [&sys_path](const std::filesystem::path& path, const std::string& description, bool insertAtBeginning = false) {
                    if (std::filesystem::exists(path)) {
                        LOG_F(INFO, "Adding %s: %s", description.c_str(), path.string().c_str());
                        if (insertAtBeginning) {
                            sys_path.insert(0, path.string());
                        } else {
                            sys_path.append(path.string());
                        }
                        return true;
                    }
                    return false;
                };
                
                // Add python directory to path
                auto pythonPath = currentPath / "python";
                addPathIfExists(pythonPath, "python path");
                
                // Add standard library paths if they're missing
                std::vector<std::string> stdLibPaths = {
                    "/usr/lib/python3.12",
                    "/usr/lib/python3.12/lib-dynload",
                    "/usr/lib/python312.zip"
                };
                
                for (const auto& libPath : stdLibPaths) {
                    addPathIfExists(std::filesystem::path(libPath), "standard library path", true);
                }

                // Add vcpkg paths if they exist
                auto vcpkgDynloadPath = currentPath / ".vcpkg" / "installed" / "x64-linux-dynamic" / "lib" / "python3.12" / "lib-dynload";
                addPathIfExists(vcpkgDynloadPath, "vcpkg lib-dynload path");

                auto vcpkgLibPath = currentPath / ".vcpkg" / "installed" / "x64-linux-dynamic" / "lib" / "python3.12";
                addPathIfExists(vcpkgLibPath, "vcpkg lib path");

                // Add venv site-packages if it exists
                auto venvSitePackages = venvPath / "lib" / "python3.12" / "site-packages";
                addPathIfExists(venvSitePackages, "venv site-packages");
                
                // Test if math module is now available
                try {
                    bp::import("math");
                    LOG_F(INFO, "Math module is now accessible");
                } catch (const bp::error_already_set&) {
                    LOG_F(ERROR, "Math module still not accessible after path setup");
                    PyErr_Clear();
                }

                initialized = true;
                LOG_F(INFO, "Python manager initialization completed successfully");

            } catch (const bp::error_already_set&) {
                PyErr_Print();
                lastError = "Error setting up Python paths";
                LOG_F(ERROR, "%s", lastError.c_str());
                PyGILState_Release(gstate);
                return false;
            }

            PyGILState_Release(gstate);
            return true;

        } catch (const std::exception& e) {
            lastError = "Exception during Python initialization: " + std::string(e.what());
            LOG_F(ERROR, "%s", lastError.c_str());
            return false;
        } catch (...) {
            LOG_F(ERROR, "Unknown exception during Python initialization");
            lastError = "Unknown error during Python initialization";
            return false;
        }
    }

    bool isInitialized() const {
        return initialized && Py_IsInitialized();
    }

    const std::string& getLastError() const {
        return lastError;
    }

    ~PythonManager() {
        if (initialized && Py_IsInitialized()) {
            LOG_F(INFO, "Finalizing Python interpreter...");
            Py_Finalize();
            LOG_F(INFO, "Python interpreter finalized successfully");
        }
    }
};

// Static member definitions
std::unique_ptr<PythonManager> PythonManager::instance = nullptr;
std::mutex PythonManager::initMutex;

class PythonProcessor::Impl {
public:
    Impl() : initialized(false) {
        LOG_F(INFO, "Starting Python processor initialization...");
        
        try {
            // Use the singleton PythonManager to ensure Python is initialized only once
            PythonManager& pythonManager = PythonManager::getInstance();
            
            if (!pythonManager.initializePython()) {
                lastError = "Failed to initialize Python: " + pythonManager.getLastError();
                LOG_F(ERROR, "%s", lastError.c_str());
                return;
            }

            LOG_F(INFO, "Acquiring GIL...");
            GILGuard gil;
            
            try {
                // Import the processor module for backward compatibility
                LOG_F(INFO, "Importing processor module...");
                try {
                    processorModule = bp::import("processor");
                    LOG_F(INFO, "Processor module imported successfully");
                    
                    processFunction = processorModule.attr("process_json");
                    LOG_F(INFO, "Process function retrieved successfully");
                    
                    initialized = true;
                    lastError.clear();
                    LOG_F(INFO, "Python processor initialization completed successfully");
                } catch (const bp::error_already_set&) {
                    LOG_F(WARNING, "Failed to import processor module - continuing without it:");
                    PyErr_Print();
                    PyErr_Clear();
                    // Still mark as initialized since we can use the generic methods
                    initialized = true;
                    lastError.clear();
                    LOG_F(INFO, "Python processor initialized in generic mode only");
                }
                
            } catch (const bp::error_already_set&) {
                LOG_F(ERROR, "Failed to set up Python environment:");
                PyErr_Print();
                PyErr_Clear();
                lastError = "Failed to set up Python environment";
            }
            
            LOG_F(INFO, "GIL released automatically by GILGuard");
            
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Exception during Python initialization: %s", e.what());
            lastError = "Failed to initialize Python: " + std::string(e.what());
        } catch (...) {
            LOG_F(ERROR, "Unknown exception during Python initialization");
            lastError = "Unknown error during Python initialization";
        }
        
        LOG_F(INFO, "Python processor initialization finished. Initialized: %s", initialized ? "true" : "false");
    }
    
    ~Impl() {
        try {
            // Only clean up our own objects, not the entire Python interpreter
            // The singleton PythonManager will handle interpreter cleanup
            if (initialized && Py_IsInitialized()) {
                LOG_F(INFO, "Cleaning up Python processor objects...");
                
                // Acquire GIL before cleanup
                GILGuard gil;
                
                // Clean up boost::python objects while the interpreter is still active
                processorModule = {};
                processFunction = {};
                
                LOG_F(INFO, "Python processor cleanup completed");
            }
        } catch (...) {
            LOG_F(ERROR, "Error during Python processor cleanup");
            // Ignore errors during cleanup to prevent exceptions in destructor
        }
    }
    
    // Helper method to create error JSON responses
    std::string createErrorJson(const std::string& errorMessage) {
        return R"({"success": false, "error": ")" + errorMessage + R"("})";
    }
    
    // Helper method to handle Python errors in JSON processing
    std::string handlePythonJsonError() {
        std::stringstream ss;
        ss << "Python execution error: ";
        
        if (PyErr_Occurred()) {
            PyObject *ptype, *pvalue, *ptraceback;
            PyErr_Fetch(&ptype, &pvalue, &ptraceback);
            
            if (pvalue) {
                PyObject* str = PyObject_Str(pvalue);
                if (str) {
                    const char* errorStr = PyUnicode_AsUTF8(str);
                    if (errorStr) {
                        ss << errorStr;
                    }
                    Py_DECREF(str);
                }
            }
            
            cleanupErrorObjects(ptype, pvalue, ptraceback);
        } else {
            ss << "Unknown Python error";
        }
        
        PyErr_Clear();
        return createErrorJson(ss.str());
    }
    
    std::string processJson(const std::string& jsonInput) {
        LOG_F(INFO, "Processing JSON input: %s...", jsonInput.substr(0, 100).c_str());
        
        if (!initialized) {
            LOG_F(ERROR, "Python processor not initialized: %s", lastError.c_str());
            return createErrorJson("Python processor not initialized: " + lastError);
        }
        
        if (!processFunction) {
            LOG_F(ERROR, "Processor module not available");
            return createErrorJson("Processor module not available. Use generic executeCommand method instead.");
        }
        
        LOG_F(INFO, "Acquiring GIL for processing...");
        PyGILState_STATE gstate = PyGILState_Ensure();
        
        try {
            bp::object result;
            {
                // Call the Python function
                LOG_SCOPE_F(INFO, "Executing Python function");
                result = processFunction(jsonInput);
            }
            // Extract the result as a string
            std::string resultStr = bp::extract<std::string>(result);
            lastError.clear();
            
            PyGILState_Release(gstate);
            LOG_F(INFO, "JSON processing completed successfully");
            return resultStr;
            
        } catch (const bp::error_already_set&) {
            LOG_F(ERROR, "Python execution error occurred");
            std::string errorResult = handlePythonJsonError();
            lastError = errorResult;
            PyGILState_Release(gstate);
            LOG_F(ERROR, "Python error: %s", lastError.c_str());
            return errorResult;
            
        } catch (const std::exception& e) {
            PyGILState_Release(gstate);
            LOG_F(ERROR, "C++ exception: %s", e.what());
            lastError = createErrorJson("C++ exception: " + std::string(e.what()));
            return lastError;
        } catch (...) {
            PyGILState_Release(gstate);
            LOG_F(ERROR, "Unknown C++ exception");
            lastError = createErrorJson("Unknown C++ exception");
            return lastError;
        }
    }
    
    // Helper method to safely import a module
    bp::object safeImportModule(const std::string& moduleName, PythonResult& result) {
        try {
            auto module = bp::import(moduleName.c_str());
            LOG_F(INFO, "Module '%s' imported successfully", moduleName.c_str());
            result.success = true;  // Set success flag on successful import
            return module;
        } catch (const bp::error_already_set&) {
            result.error = "Failed to import module: " + moduleName;
            result.success = false;
            LOG_F(ERROR, "%s", result.error.c_str());
            PyErr_Clear();
            return bp::object(); // Return empty object to signal failure
        }
    }
    
    // Helper method to safely get a function from a module
    bp::object safeGetFunction(const bp::object& module, const std::string& functionName, const std::string& moduleName, PythonResult& result) {
        try {
            auto function = module.attr(functionName.c_str());
            LOG_F(INFO, "Function '%s' retrieved successfully", functionName.c_str());
            result.success = true;  // Set success flag on successful retrieval
            return function;
        } catch (const bp::error_already_set&) {
            result.error = "Function '" + functionName + "' not found in module '" + moduleName + "'";
            result.success = false;
            LOG_F(ERROR, "%s", result.error.c_str());
            PyErr_Clear();
            return bp::object(); // Return empty object to signal failure
        }
    }
    
    PythonResult executeCommand(const PythonCommand& command) {
        LOG_F(INFO, "Executing command: %s.%s", command.module.c_str(), command.function.c_str());
        
        return executePythonOperation<PythonResult>("executeCommand", [&]() -> PythonResult {
            PythonResult result;
            result.success = false;  // Initialize properly
            LOG_F(INFO, "Lambda starting executeCommand");
            
            // Import the module
            bp::object module = safeImportModule(command.module, result);
            LOG_F(INFO, "After safeImportModule, success=%s", result.success ? "true" : "false");
            if (!result.success) return result; // Early return on failure
            
            // Get the function
            bp::object function = safeGetFunction(module, command.function, command.module, result);
            LOG_F(INFO, "After safeGetFunction, success=%s", result.success ? "true" : "false");
            if (!result.success) return result; // Early return on failure
            
            // Convert arguments and call function or get attribute
            LOG_F(INFO, "About to call executeCallableOrGetAttribute");
            bp::object pyResult = executeCallableOrGetAttribute(function, command);
            LOG_F(INFO, "After executeCallableOrGetAttribute");
            
            // Convert result and validate type
            LOG_F(INFO, "About to call convertPythonToResult");
            result = convertPythonToResult(pyResult, command.expectedType);
            LOG_F(INFO, "After convertPythonToResult, success=%s", result.success ? "true" : "false");
            return result;
        });
    }
    
    PythonResult callFunction(const std::string& module, const std::string& function, 
                             const std::vector<std::any>& args) {
        PythonCommand command(module, function, args);
        return executeCommand(command);
    }
    
    PythonResult callModuleFunction(const std::string& moduleDotFunction, 
                                   const std::vector<std::any>& args) {
        size_t lastDot = moduleDotFunction.find_last_of('.');
        if (lastDot == std::string::npos) {
            PythonResult result;
            result.success = false;
            result.error = "Invalid module.function format: " + moduleDotFunction;
            return result;
        }
        
        std::string module = moduleDotFunction.substr(0, lastDot);
        std::string function = moduleDotFunction.substr(lastDot + 1);
        
        return callFunction(module, function, args);
    }
    
    PythonResult executeCode(const std::string& pythonCode) {
        LOG_F(INFO, "Executing Python code: %s...", pythonCode.substr(0, 100).c_str());
        
        return executePythonOperation<PythonResult>("executeCode", [&]() -> PythonResult {
            PythonResult result;
            
            // Create a fresh namespace for each execution to ensure isolation
            bp::dict fresh_namespace = NamespaceHelper::createFreshNamespace();
            
            bp::object pyResult = executeCodeWithNamespace(pythonCode, fresh_namespace);
            
            result = convertPythonToResult(pyResult, "any");
            return result;
        });
    }
    
    PythonResult evaluateExpression(const std::string& expression) {
        LOG_F(INFO, "Evaluating Python expression: %s...", expression.substr(0, 100).c_str());
        
        return executePythonOperation<PythonResult>("evaluateExpression", [&]() -> PythonResult {
            PythonResult result;
            
            // Create a fresh namespace for each execution to ensure isolation
            bp::dict fresh_namespace = NamespaceHelper::createFreshNamespace();
            
            bp::object pyResult = bp::eval(expression.c_str(), fresh_namespace, fresh_namespace);
            
            result = convertPythonToResult(pyResult, "any");
            return result;
        });
    }
    
    PythonResult executeStatement(const std::string& statement) {
        LOG_F(INFO, "Executing Python statement: %s...", statement.substr(0, 100).c_str());
        
        return executePythonOperation<PythonResult>("executeStatement", [&]() -> PythonResult {
            PythonResult result;
            
            // Create a fresh namespace for each execution to ensure isolation
            bp::dict fresh_namespace = NamespaceHelper::createFreshNamespace();
            
            bp::exec(statement.c_str(), fresh_namespace, fresh_namespace);
            
            // For statements, return success with None value
            result.actualType = "NoneType";
            result.stringValue = "None";
            result.value = std::string("None");
            return result;
        });
    }
    
    bool canImportModule(const std::string& module) {
        return executePythonBoolOperation("canImportModule", [&]() -> bool {
            bp::import(module.c_str());
            return true;
        });
    }
    
    std::vector<std::string> getModuleFunctions(const std::string& module) {
        std::vector<std::string> functions;
        
        if (!initialized) {
            LOG_F(ERROR, "getModuleFunctions failed: not initialized");
            return functions;
        }
        
        try {
            GILGuard gil;
            
            bp::object mod = bp::import(module.c_str());
            bp::object main_module = bp::import("__main__");
            bp::object main_namespace = main_module.attr("__dict__");
            bp::dict local_dict;
            local_dict["mod"] = mod;
            bp::object dir_result = bp::eval("dir(mod)", main_namespace, local_dict);
            
            bp::list items = bp::extract<bp::list>(dir_result);
            for (int i = 0; i < bp::len(items); ++i) {
                std::string name = bp::extract<std::string>(items[i]);
                // Filter out private attributes and check if it's callable
                if (!name.empty() && name[0] != '_') {
                    try {
                        bp::object attr = mod.attr(name.c_str());
                        if (PyCallable_Check(attr.ptr())) {
                            functions.push_back(name);
                        }
                    } catch (...) {
                        // Skip if we can't access the attribute
                    }
                }
            }
            
        } catch (const bp::error_already_set&) {
            LOG_F(ERROR, "Error getting module functions: %s", extractPythonError().c_str());
            PyErr_Clear();
        } catch (...) {
            LOG_F(ERROR, "Exception in getModuleFunctions");
        }
        
        return functions;
    }
    
    bool isInitialized() const {
        return initialized;
    }
    
    std::string getLastError() const {
        return lastError;
    }

private:
    bool initialized;
    std::string lastError;
    bp::object processorModule;
    bp::object processFunction;
    
    // Helper method to check initialization and return early if not initialized
    template<typename ResultType>
    bool checkInitialization(ResultType& result, const std::string& operation) {
        if (!initialized) {
            result.error = "Python processor not initialized: " + lastError;
            LOG_F(ERROR, "Operation '%s' failed: %s", operation.c_str(), result.error.c_str());
            result.success = false;
            return false;
        }
        return true;
    }
    
    // Template method for executing Python operations with common error handling
    template<typename ResultType, typename Operation>
    ResultType executePythonOperation(const std::string& operationName, Operation&& operation) {
        ResultType result;
        result.success = false;
        
        if (!checkInitialization(result, operationName)) {
            return result;
        }
        
        try {
            GILGuard gil;
            result = operation();
            // Don't override result.success - let the operation set it
            if (result.success) {
                lastError.clear();
                LOG_F(INFO, "%s completed successfully", operationName.c_str());
            }
        } catch (const bp::error_already_set&) {
            result.error = "Error in " + operationName + ": " + extractPythonError();
            LOG_F(ERROR, "%s", result.error.c_str());
            PyErr_Clear();
        } catch (const std::exception& e) {
            result.error = "C++ exception in " + operationName + ": " + std::string(e.what());
            LOG_F(ERROR, "%s", result.error.c_str());
        } catch (...) {
            result.error = "Unknown exception in " + operationName;
            LOG_F(ERROR, "%s", result.error.c_str());
        }
        
        return result;
    }
    
    // Specialized template for boolean operations
    template<typename Operation>
    bool executePythonBoolOperation(const std::string& operationName, Operation&& operation) {
        if (!initialized) {
            LOG_F(ERROR, "Operation '%s' failed: not initialized", operationName.c_str());
            return false;
        }
        
        try {
            GILGuard gil;
            return operation();
        } catch (const bp::error_already_set&) {
            LOG_F(ERROR, "Error in %s: %s", operationName.c_str(), extractPythonError().c_str());
            PyErr_Clear();
            return false;
        } catch (...) {
            LOG_F(ERROR, "Exception in %s", operationName.c_str());
            return false;
        }
    }
    
    // Helper method to execute callable or get attribute
    bp::object executeCallableOrGetAttribute(const bp::object& function, const PythonCommand& command) {
        LOG_F(INFO, "executeCallableOrGetAttribute called for function: %s", command.function.c_str());
        LOG_F(INFO, "Function pointer valid: %s", function.ptr() ? "true" : "false");
        
        if (PyCallable_Check(function.ptr())) {
            LOG_F(INFO, "Function is callable");
            // It's a function, call it
            if (command.args.empty()) {
                LOG_F(INFO, "Calling function with no arguments");
                auto result = function();
                LOG_F(INFO, "Function executed successfully");
                return result;
            } else {
                LOG_F(INFO, "Calling function with %zu arguments", command.args.size());
                // Convert std::any arguments to boost::python objects
                bp::list pyArgs;
                for (const auto& arg : command.args) {
                    pyArgs.append(convertAnyToPython(arg));
                }
                
                // Call with unpacked arguments
                auto result = function(*bp::tuple(pyArgs));
                LOG_F(INFO, "Function executed successfully");
                return result;
            }
        } else {
            LOG_F(INFO, "Function is not callable - treating as attribute");
            // It's a constant/attribute, just return its value
            if (!command.args.empty()) {
                throw std::runtime_error("Cannot call non-callable attribute '" + command.function + "' with arguments");
            }
            LOG_F(INFO, "Attribute '%s' retrieved successfully", command.function.c_str());
            return function;
        }
    }
    
    // Helper method to execute code with a given namespace
    bp::object executeCodeWithNamespace(const std::string& pythonCode, bp::dict& namespace_dict) {
        // Try eval first (for expressions), fall back to exec (for statements)
        try {
            auto result = bp::eval(pythonCode.c_str(), namespace_dict, namespace_dict);
            LOG_F(INFO, "Code evaluated as expression successfully");
            return result;
        } catch (const bp::error_already_set&) {
            PyErr_Clear(); // Clear the eval error
            // If eval failed, try exec (returns None but executes statements)
            auto result = bp::exec(pythonCode.c_str(), namespace_dict, namespace_dict);
            LOG_F(INFO, "Code executed as statement successfully");
            
            // For statements like "x = 10; x", try to get the result of the last expression
            if (pythonCode.find(';') != std::string::npos || pythonCode.find('=') != std::string::npos) {
                // Extract the last part after semicolon or newline
                size_t lastSemicolon = pythonCode.find_last_of(';');
                if (lastSemicolon != std::string::npos && lastSemicolon + 1 < pythonCode.length()) {
                    std::string lastExpr = pythonCode.substr(lastSemicolon + 1);
                    // Trim whitespace
                    lastExpr.erase(0, lastExpr.find_first_not_of(" \t\n\r"));
                    lastExpr.erase(lastExpr.find_last_not_of(" \t\n\r") + 1);
                    
                    if (!lastExpr.empty()) {
                        try {
                            auto lastResult = bp::eval(lastExpr.c_str(), namespace_dict, namespace_dict);
                            LOG_F(INFO, "Successfully evaluated last expression: %s", lastExpr.c_str());
                            return lastResult;
                        } catch (const bp::error_already_set&) {
                            PyErr_Clear();
                            // Keep the None result from exec
                        }
                    }
                }
            }
            return result;
        }
    }
    
    // Helper methods for type conversion
    bp::object convertAnyToPython(const std::any& value) {
        // Try to cast to common types
        try {
            if (auto strPtr = std::any_cast<std::string>(&value)) {
                return bp::object(*strPtr);
            } else if (auto intPtr = std::any_cast<int>(&value)) {
                return bp::object(*intPtr);
            } else if (auto doublePtr = std::any_cast<double>(&value)) {
                return bp::object(*doublePtr);
            } else if (auto floatPtr = std::any_cast<float>(&value)) {
                return bp::object(*floatPtr);
            } else if (auto boolPtr = std::any_cast<bool>(&value)) {
                return bp::object(*boolPtr);
            } else if (auto cstrPtr = std::any_cast<const char*>(&value)) {
                return bp::object(std::string(*cstrPtr));
            } else {
                // Try to convert to string as fallback
                throw std::runtime_error("Unsupported argument type");
            }
        } catch (const std::bad_any_cast&) {
            throw std::runtime_error("Failed to convert argument to Python object");
        }
    }
    
    // Helper method to get Python object type name
    std::string getPythonTypeName(const bp::object& pyObj) {
        PyObject* type = PyObject_Type(pyObj.ptr());
        if (!type) return "unknown";
        
        PyObject* typeName = PyObject_GetAttrString(type, "__name__");
        std::string result = "unknown";
        if (typeName) {
            const char* name = PyUnicode_AsUTF8(typeName);
            if (name) result = name;
            Py_DECREF(typeName);
        }
        Py_DECREF(type);
        return result;
    }
    
    // Helper methods for specific type conversions
    void convertNoneType(PythonResult& result) {
        result.value = std::string("None");
        result.actualType = "NoneType";
    }
    
    void convertBoolType(const bp::object& pyObj, PythonResult& result) {
        bool bool_val = PyObject_IsTrue(pyObj.ptr()) == 1;
        result.value = bool_val;
        result.actualType = "bool";
        LOG_F(INFO, "Converted Python bool to C++ bool: %s", bool_val ? "true" : "false");
    }
    
    void convertStringType(const bp::object& pyObj, PythonResult& result) {
        std::string str_val = bp::extract<std::string>(pyObj);
        result.value = str_val;
        result.actualType = "str";
    }
    
    void convertIntType(const bp::object& pyObj, PythonResult& result) {
        int int_val = bp::extract<int>(pyObj);
        result.value = int_val;
        result.actualType = "int";
    }
    
    void convertFloatType(const bp::object& pyObj, PythonResult& result) {
        double double_val = bp::extract<double>(pyObj);
        result.value = double_val;
        result.actualType = "float";
    }
    
    void convertSequenceType(const bp::object& pyObj, PythonResult& result) {
        result.value = result.stringValue;
        result.actualType = PyList_Check(pyObj.ptr()) ? "list" : "tuple";
    }
    
    void convertDictType(const bp::object& pyObj, PythonResult& result) {
        result.value = result.stringValue;
        result.actualType = "dict";
    }
    
    void convertGenericType(const bp::object& pyObj, PythonResult& result) {
        result.value = result.stringValue;
        result.actualType = "object";
    }
    
    PythonResult convertPythonToResult(const bp::object& pyObj, const std::string& expectedType) {
        PythonResult result;
        result.success = true;
        
        try {
            // Get string representation first
            result.stringValue = bp::extract<std::string>(bp::str(pyObj));
            
            // Determine and set actual type
            result.actualType = getPythonTypeName(pyObj);
            
            // Convert to appropriate C++ type based on Python type
            if (pyObj.ptr() == Py_None) {
                convertNoneType(result);
            } else if (PyBool_Check(pyObj.ptr())) {
                // Handle Python booleans first (before integers, since bool is a subclass of int in Python)
                convertBoolType(pyObj, result);
            } else if (PyUnicode_Check(pyObj.ptr())) {
                convertStringType(pyObj, result);
            } else if (PyLong_Check(pyObj.ptr())) {
                convertIntType(pyObj, result);
            } else if (PyFloat_Check(pyObj.ptr())) {
                convertFloatType(pyObj, result);
            } else if (PyList_Check(pyObj.ptr()) || PyTuple_Check(pyObj.ptr())) {
                convertSequenceType(pyObj, result);
            } else if (PyDict_Check(pyObj.ptr())) {
                convertDictType(pyObj, result);
            } else {
                convertGenericType(pyObj, result);
            }
            
        } catch (const bp::error_already_set&) {
            result.success = false;
            result.error = "Failed to convert Python result: " + extractPythonError();
            PyErr_Clear();
        } catch (const std::exception& e) {
            result.success = false;
            result.error = "Conversion error: " + std::string(e.what());
        }
        
        return result;
    }
    
    // Helper methods for Python error extraction
    std::string getExceptionTypeName(PyObject* ptype) {
        if (!ptype) return "";
        
        PyObject* typeNameObj = PyObject_GetAttrString(ptype, "__name__");
        if (!typeNameObj) return "";
        
        const char* typeName = PyUnicode_AsUTF8(typeNameObj);
        std::string result = typeName ? std::string(typeName) + ": " : "";
        Py_DECREF(typeNameObj);
        return result;
    }
    
    std::string getExceptionMessage(PyObject* pvalue) {
        if (!pvalue) return "";
        
        PyObject* str = PyObject_Str(pvalue);
        if (!str) return "";
        
        const char* errorStr = PyUnicode_AsUTF8(str);
        std::string result = errorStr ? errorStr : "";
        Py_DECREF(str);
        return result;
    }
    
    void cleanupErrorObjects(PyObject* ptype, PyObject* pvalue, PyObject* ptraceback) {
        if (ptype) Py_DECREF(ptype);
        if (pvalue) Py_DECREF(pvalue);
        if (ptraceback) Py_DECREF(ptraceback);
    }
    
    std::string extractPythonError() {
        if (!PyErr_Occurred()) {
            return "Unknown Python error";
        }
        
        PyObject *ptype, *pvalue, *ptraceback;
        PyErr_Fetch(&ptype, &pvalue, &ptraceback);
        
        std::string error = getExceptionTypeName(ptype) + getExceptionMessage(pvalue);
        
        if (error.empty()) {
            error = "Python error";
        }
        
        cleanupErrorObjects(ptype, pvalue, ptraceback);
        return error;
    }
};

// PythonProcessor implementation
PythonProcessor::PythonProcessor() : pImpl(std::make_unique<Impl>()) {}

PythonProcessor::~PythonProcessor() = default;

std::string PythonProcessor::processJson(const std::string& jsonInput) {
    return pImpl->processJson(jsonInput);
}

PythonResult PythonProcessor::executeCommand(const PythonCommand& command) {
    return pImpl->executeCommand(command);
}

PythonResult PythonProcessor::callFunction(const std::string& module, const std::string& function, 
                                         const std::vector<std::any>& args) {
    return pImpl->callFunction(module, function, args);
}

PythonResult PythonProcessor::callModuleFunction(const std::string& moduleDotFunction, 
                                               const std::vector<std::any>& args) {
    return pImpl->callModuleFunction(moduleDotFunction, args);
}

PythonResult PythonProcessor::executeCode(const std::string& pythonCode) {
    return pImpl->executeCode(pythonCode);
}

PythonResult PythonProcessor::evaluateExpression(const std::string& expression) {
    return pImpl->evaluateExpression(expression);
}

PythonResult PythonProcessor::executeStatement(const std::string& statement) {
    return pImpl->executeStatement(statement);
}

bool PythonProcessor::canImportModule(const std::string& module) {
    return pImpl->canImportModule(module);
}

std::vector<std::string> PythonProcessor::getModuleFunctions(const std::string& module) {
    return pImpl->getModuleFunctions(module);
}

bool PythonProcessor::isInitialized() const {
    return pImpl->isInitialized();
}

std::string PythonProcessor::getLastError() const {
    return pImpl->getLastError();
}
