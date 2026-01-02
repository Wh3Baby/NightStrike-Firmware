#include "modules/interpreter_module.h"
#include "core/storage.h"
#include <Arduino.h>
#include <LittleFS.h>

namespace NightStrike {
namespace Modules {

// Note: This is a framework for JavaScript interpreter
// Full implementation requires Duktape library
// For now, we create the structure that can be extended

InterpreterModule::InterpreterModule() {
}

Core::Error InterpreterModule::initialize() {
    if (_initialized) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }

    // TODO: Initialize Duktape interpreter
    // duk_context *ctx = duk_create_heap(NULL, NULL, NULL, NULL, NULL);
    // if (!ctx) {
    //     return Core::Error(Core::ErrorCode::MODULE_INIT_FAILED);
    // }
    
    Serial.println("[Interpreter] Module initialized (framework)");
    Serial.println("[Interpreter] Note: Duktape library required for full functionality");
    
    _initialized = true;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error InterpreterModule::shutdown() {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    stopScript();
    
    // TODO: Destroy Duktape context
    // duk_destroy_heap(ctx);
    
    _initialized = false;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

bool InterpreterModule::isSupported() const {
    // Check if we have enough memory for interpreter
    // Duktape requires significant RAM
    return ESP.getFreeHeap() > 50000;  // Need at least 50KB free
}

Core::Error InterpreterModule::executeScript(const std::string& script) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (_running) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED, "Script already running");
    }

    _running = true;
    
    Serial.println("[Interpreter] Executing script...");
    
    // TODO: Execute script with Duktape
    // if (duk_peval_string(ctx, script.c_str()) != DUK_EXEC_SUCCESS) {
    //     const char* error = duk_safe_to_string(ctx, -1);
    //     Serial.printf("[Interpreter] Error: %s\n", error);
    //     _running = false;
    //     return Core::Error(Core::ErrorCode::OPERATION_FAILED, error);
    // }
    
    Serial.println("[Interpreter] Script execution completed");
    _running = false;
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error InterpreterModule::executeFile(const std::string& filename) {
    if (!LittleFS.begin(true)) {
        return Core::Error(Core::ErrorCode::STORAGE_NOT_MOUNTED);
    }

    File file = LittleFS.open(filename.c_str(), "r");
    if (!file) {
        return Core::Error(Core::ErrorCode::FILE_NOT_FOUND);
    }

    std::string script;
    while (file.available()) {
        script += (char)file.read();
    }
    file.close();

    return executeScript(script);
}

Core::Error InterpreterModule::stopScript() {
    if (!_running) {
        return Core::Error(Core::ErrorCode::SUCCESS);
    }

    _running = false;
    
    // TODO: Stop/interrupt script execution
    // This might require task cancellation if running in separate task
    
    Serial.println("[Interpreter] Script stopped");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error InterpreterModule::listScripts(std::vector<std::string>& scripts) {
    scripts.clear();

    if (!LittleFS.begin(true)) {
        return Core::Error(Core::ErrorCode::STORAGE_NOT_MOUNTED);
    }

    File root = LittleFS.open("/");
    if (!root) {
        return Core::Error(Core::ErrorCode::STORAGE_NOT_MOUNTED);
    }

    File file = root.openNextFile();
    while (file) {
        std::string name = file.name();
        if (name.find(".js") != std::string::npos || name.find(".bjs") != std::string::npos) {
            scripts.push_back(name);
        }
        file = root.openNextFile();
    }

    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error InterpreterModule::saveScript(const std::string& filename, const std::string& script) {
    if (!LittleFS.begin(true)) {
        return Core::Error(Core::ErrorCode::STORAGE_NOT_MOUNTED);
    }

    File file = LittleFS.open(filename.c_str(), "w");
    if (!file) {
        return Core::Error(Core::ErrorCode::FILE_WRITE_ERROR);
    }

    file.print(script.c_str());
    file.close();

    Serial.printf("[Interpreter] Script saved: %s\n", filename.c_str());
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error InterpreterModule::deleteScript(const std::string& filename) {
    if (!LittleFS.begin(true)) {
        return Core::Error(Core::ErrorCode::STORAGE_NOT_MOUNTED);
    }

    if (!LittleFS.remove(filename.c_str())) {
        return Core::Error(Core::ErrorCode::FILE_DELETE_ERROR);
    }

    Serial.printf("[Interpreter] Script deleted: %s\n", filename.c_str());
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error InterpreterModule::initInterpreter() {
    // TODO: Initialize Duktape context
    // Register global APIs
    registerAPIs();
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

void InterpreterModule::registerAPIs() {
    // TODO: Register JavaScript APIs for modules
    // This would bind C++ functions to JavaScript:
    // - WiFi functions
    // - BLE functions
    // - RF functions
    // - RFID functions
    // - Display functions
    // - Storage functions
    // etc.
    
    Serial.println("[Interpreter] APIs registered");
}

} // namespace Modules
} // namespace NightStrike

