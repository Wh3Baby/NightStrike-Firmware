#pragma once

#include "core/module_interface.h"
#include "core/errors.h"
#include <vector>
#include <string>
#include <functional>

namespace NightStrike {
namespace Modules {

/**
 * @brief BadUSB module for HID attacks
 *
 * Features:
 * - Ducky script execution
 * - Keyboard injection
 * - Mouse control (if supported)
 * - Script storage and management
 */
class BadUSBModule : public Core::IModule {
public:
    BadUSBModule();
    ~BadUSBModule() override = default;

    // IModule interface
    const char* getName() const override { return "BadUSB"; }
    Core::Error initialize() override;
    Core::Error shutdown() override;
    bool isInitialized() const override { return _initialized; }
    bool isSupported() const override;

    // Script execution
    Core::Error executeScript(const std::string& script);
    Core::Error executeDuckyScript(const std::string& script);
    Core::Error loadScriptFromFile(const std::string& filename);
    Core::Error saveScriptToFile(const std::string& filename, const std::string& script);

    // Keyboard operations
    Core::Error typeString(const std::string& text);
    Core::Error pressKey(uint8_t key, uint8_t modifiers = 0);
    Core::Error releaseKey(uint8_t key);
    Core::Error delay(uint32_t ms);

    // Script management
    Core::Error listScripts(std::vector<std::string>& scripts);
    Core::Error deleteScript(const std::string& filename);

    // Status
    bool isRunning() const { return _running; }
    void setProgressCallback(std::function<void(uint32_t, uint32_t)> callback) {
        _progressCallback = callback;
    }

private:
    bool _initialized = false;
    bool _running = false;
    uint32_t _defaultDelay = 0;
    std::function<void(uint32_t, uint32_t)> _progressCallback;

    // Ducky script parser
    Core::Error parseDuckyScript(const std::string& script, std::vector<std::string>& commands);
    Core::Error executeDuckyCommand(const std::string& command);
    
    // Key mapping
    uint8_t getKeyCode(const std::string& key);
    uint8_t getModifierCode(const std::string& modifier);
};

} // namespace Modules
} // namespace NightStrike

