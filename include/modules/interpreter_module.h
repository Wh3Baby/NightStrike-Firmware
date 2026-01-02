#pragma once

#include "core/module_interface.h"
#include "core/errors.h"
#include <vector>
#include <string>
#include <functional>

namespace NightStrike {
namespace Modules {

/**
 * @brief JavaScript interpreter module
 *
 * Features:
 * - JavaScript script execution
 * - Script file management
 * - API bindings for modules
 * - Error handling
 *
 * Note: Requires Duktape library for full functionality
 */
class InterpreterModule : public Core::IModule {
public:
    InterpreterModule();
    ~InterpreterModule() override = default;

    // IModule interface
    const char* getName() const override { return "Interpreter"; }
    Core::Error initialize() override;
    Core::Error shutdown() override;
    bool isInitialized() const override { return _initialized; }
    bool isSupported() const override;

    // Script execution
    Core::Error executeScript(const std::string& script);
    Core::Error executeFile(const std::string& filename);
    Core::Error stopScript();

    // Script management
    Core::Error listScripts(std::vector<std::string>& scripts);
    Core::Error saveScript(const std::string& filename, const std::string& script);
    Core::Error deleteScript(const std::string& filename);

    // Status
    bool isRunning() const { return _running; }
    void setOutputCallback(std::function<void(const std::string&)> callback) {
        _outputCallback = callback;
    }

private:
    bool _initialized = false;
    bool _running = false;
    std::function<void(const std::string&)> _outputCallback;
    
    // Internal methods
    Core::Error initInterpreter();
    void registerAPIs();  // Register module APIs to JavaScript
};

} // namespace Modules
} // namespace NightStrike

