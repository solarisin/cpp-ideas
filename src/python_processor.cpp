#include "python_processor.h"
#include <boost/python.hpp>
#include <sstream>
#include <filesystem>
#include <loguru/loguru.hpp>

namespace bp = boost::python;

class PythonProcessor::Impl {
public:
    Impl() : initialized(false) {
        LOG_F(INFO, "Starting Python processor initialization...");
        
        try {
            if (!Py_IsInitialized()) {
                LOG_F(INFO, "Python not initialized, setting up configuration...");
                
                // Use PyConfig for Python 3.8+ initialization
                PyConfig config;
                PyConfig_InitPythonConfig(&config);
                
                // For embedded Python, we should use the system Python paths
                // but add our venv site-packages to the path later
                std::filesystem::path currentPath = std::filesystem::current_path();
                std::filesystem::path venvPath = currentPath / ".venv";
                
                LOG_F(INFO, "Current path: %s", currentPath.c_str());
                LOG_F(INFO, "Venv path: %s", venvPath.c_str());
                
                // Don't set PYTHONHOME to venv - let it use system defaults
                // We'll add the venv site-packages directory to sys.path later
                
                LOG_F(INFO, "Initializing Python from config...");
                // Initialize Python with config
                PyStatus status = Py_InitializeFromConfig(&config);
                PyConfig_Clear(&config);
                
                if (PyStatus_Exception(status)) {
                    LOG_F(ERROR, "Python initialization failed");
                    lastError = "Failed to initialize Python from config";
                    return;
                } else {
                    LOG_F(INFO, "Python initialized successfully");
                }
            } else {
                LOG_F(INFO, "Python already initialized");
            }
            
            LOG_F(INFO, "Acquiring GIL...");
            // Ensure we have the main thread state
            PyGILState_STATE gstate = PyGILState_Ensure();
            
            try {
                LOG_F(INFO, "Setting up Python paths...");
                // Add current directory to Python path
                bp::object sys = bp::import("sys");
                bp::list path = bp::extract<bp::list>(sys.attr("path"));
                
                // Add the src directory to Python path
                std::filesystem::path currentPath = std::filesystem::current_path();
                std::filesystem::path srcPath = currentPath / "src";
                
                if (std::filesystem::exists(srcPath)) {
                    LOG_F(INFO, "Adding src path: %s", srcPath.c_str());
                    path.append(srcPath.string());
                } else {
                    LOG_F(INFO, "Adding relative paths...");
                    // Try relative path
                    path.append("./src");
                    path.append(".");
                }
                
                // Add vcpkg Python paths for extension modules
                std::filesystem::path vcpkgPath = currentPath / ".vcpkg" / "installed" / "x64-linux";
                std::filesystem::path libDynload = vcpkgPath / "lib" / "python3.12" / "lib-dynload";
                std::filesystem::path libPath = vcpkgPath / "lib" / "python3.12";
                
                if (std::filesystem::exists(libDynload)) {
                    LOG_F(INFO, "Adding vcpkg lib-dynload path: %s", libDynload.c_str());
                    path.append(libDynload.string());
                }
                
                if (std::filesystem::exists(libPath)) {
                    LOG_F(INFO, "Adding vcpkg lib path: %s", libPath.c_str());
                    path.append(libPath.string());
                }
                
                // Add venv site-packages if it exists
                std::filesystem::path venvPath = currentPath / ".venv";
                std::filesystem::path venvSitePackages = venvPath / "lib" / "python3.12" / "site-packages";
                if (std::filesystem::exists(venvSitePackages)) {
                    LOG_F(INFO, "Adding venv site-packages: %s", venvSitePackages.c_str());
                    path.append(venvSitePackages.string());
                } else {
                    LOG_F(WARNING, "Venv site-packages not found: %s", venvSitePackages.c_str());
                }
                
                // Import the processor module
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
                    LOG_F(ERROR, "Failed to import processor module:");
                    PyErr_Print();
                    PyErr_Clear();
                    lastError = "Failed to import processor module. Make sure processor.py is in the src directory.";
                }
                
            } catch (const bp::error_already_set&) {
                LOG_F(ERROR, "Failed to set up Python environment:");
                PyErr_Print();
                PyErr_Clear();
                lastError = "Failed to set up Python environment";
            }
            
            LOG_F(INFO, "Releasing GIL...");
            PyGILState_Release(gstate);
            
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
            if (Py_IsInitialized()) {
                // Don't finalize Python as it might be used elsewhere
                // Py_Finalize();
            }
        } catch (...) {
            // Ignore errors during cleanup
        }
    }
    
    std::string processJson(const std::string& jsonInput) {
        LOG_F(INFO, "Processing JSON input: %s...", jsonInput.substr(0, 100).c_str());
        
        if (!initialized) {
            LOG_F(ERROR, "Python processor not initialized: %s", lastError.c_str());
            return R"({"success": false, "error": "Python processor not initialized: )" + lastError + R"("})";
        }
        
        LOG_F(INFO, "Acquiring GIL for processing...");
        PyGILState_STATE gstate = PyGILState_Ensure();
        
        try {
            LOG_F(INFO, "Calling Python function...");
            // Call the Python function
            bp::object result = processFunction(jsonInput);
            
            LOG_F(INFO, "Python function completed successfully");
            // Extract the result as a string
            std::string resultStr = bp::extract<std::string>(result);
            lastError.clear();
            
            PyGILState_Release(gstate);
            LOG_F(INFO, "JSON processing completed successfully");
            return resultStr;
            
        } catch (const bp::error_already_set&) {
            LOG_F(ERROR, "Python execution error occurred");
            // Handle Python exceptions
            std::stringstream ss;
            ss << R"({"success": false, "error": "Python execution error: )";
            
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
                
                if (ptype) Py_DECREF(ptype);
                if (pvalue) Py_DECREF(pvalue);
                if (ptraceback) Py_DECREF(ptraceback);
            } else {
                ss << "Unknown Python error";
            }
            
            ss << R"("})";
            lastError = ss.str();
            PyErr_Clear();
            PyGILState_Release(gstate);
            LOG_F(ERROR, "Python error: %s", lastError.c_str());
            return lastError;
            
        } catch (const std::exception& e) {
            PyGILState_Release(gstate);
            LOG_F(ERROR, "C++ exception: %s", e.what());
            lastError = R"({"success": false, "error": "C++ exception: )" + std::string(e.what()) + R"("})";
            return lastError;
        } catch (...) {
            PyGILState_Release(gstate);
            LOG_F(ERROR, "Unknown C++ exception");
            lastError = R"({"success": false, "error": "Unknown C++ exception"})";
            return lastError;
        }
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
};

// PythonProcessor implementation
PythonProcessor::PythonProcessor() : pImpl(std::make_unique<Impl>()) {}

PythonProcessor::~PythonProcessor() = default;

std::string PythonProcessor::processJson(const std::string& jsonInput) {
    return pImpl->processJson(jsonInput);
}

bool PythonProcessor::isInitialized() const {
    return pImpl->isInitialized();
}

std::string PythonProcessor::getLastError() const {
    return pImpl->getLastError();
}
