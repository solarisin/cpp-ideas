#include "python_processor.h"
#include <boost/python.hpp>
#include <sstream>
#include <filesystem>
#include <loguru/loguru.hpp>
#include <mutex>
#include <memory>

namespace bp = boost::python;

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

                // Add python directory to path
                auto pythonPath = currentPath / "python";
                if (std::filesystem::exists(pythonPath)) {
                    LOG_F(INFO, "Adding python path: %s", pythonPath.string().c_str());
                    sys_path.append(pythonPath.string());
                }

                // Add vcpkg paths if they exist
                auto vcpkgDynloadPath = currentPath / ".vcpkg" / "installed" / "x64-linux-dynamic" / "lib" / "python3.12" / "lib-dynload";
                if (std::filesystem::exists(vcpkgDynloadPath)) {
                    LOG_F(INFO, "Adding vcpkg lib-dynload path: %s", vcpkgDynloadPath.string().c_str());
                    sys_path.append(vcpkgDynloadPath.string());
                }

                auto vcpkgLibPath = currentPath / ".vcpkg" / "installed" / "x64-linux-dynamic" / "lib" / "python3.12";
                if (std::filesystem::exists(vcpkgLibPath)) {
                    LOG_F(INFO, "Adding vcpkg lib path: %s", vcpkgLibPath.string().c_str());
                    sys_path.append(vcpkgLibPath.string());
                }

                // Add venv site-packages if it exists
                auto venvSitePackages = venvPath / "lib" / "python3.12" / "site-packages";
                if (std::filesystem::exists(venvSitePackages)) {
                    LOG_F(INFO, "Adding venv site-packages: %s", venvSitePackages.string().c_str());
                    sys_path.append(venvSitePackages.string());
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
            PyGILState_STATE gstate = PyGILState_Ensure();
            
            try {
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
                    lastError = "Failed to import processor module. Make sure processor.py is in the python directory.";
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
            // Only clean up our own objects, not the entire Python interpreter
            // The singleton PythonManager will handle interpreter cleanup
            if (initialized && Py_IsInitialized()) {
                LOG_F(INFO, "Cleaning up Python processor objects...");
                
                // Acquire GIL before cleanup
                PyGILState_STATE gstate = PyGILState_Ensure();
                
                // Clean up boost::python objects while the interpreter is still active
                processorModule = {};
                processFunction = {};
                
                PyGILState_Release(gstate);
                LOG_F(INFO, "Python processor cleanup completed");
            }
        } catch (...) {
            LOG_F(ERROR, "Error during Python processor cleanup");
            // Ignore errors during cleanup to prevent exceptions in destructor
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
