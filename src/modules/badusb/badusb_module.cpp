#include "modules/badusb_module.h"
#include "modules/ble_module.h"
#include "core/storage.h"
#include <Arduino.h>
#include <LittleFS.h>
#include <map>
#include <sstream>

// Forward declaration
extern NightStrike::Modules::BLEModule* g_bleModule;

namespace NightStrike {
namespace Modules {

BadUSBModule::BadUSBModule() {
}

Core::Error BadUSBModule::initialize() {
    if (_initialized) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }

    // Initialize USB HID (if available)
    // Note: ESP32 doesn't have native USB HID, so this would require
    // external hardware or BLE HID
    
    Serial.println("[BadUSB] Module initialized");
    _initialized = true;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error BadUSBModule::shutdown() {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (_running) {
        _running = false;
    }

    _initialized = false;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

bool BadUSBModule::isSupported() const {
    // BadUSB requires BLE HID or external USB hardware
    // For now, we'll support it via BLE HID
    return true;
}

Core::Error BadUSBModule::executeScript(const std::string& script) {
    return executeDuckyScript(script);
}

Core::Error BadUSBModule::executeDuckyScript(const std::string& script) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (_running) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED, "Script already running");
    }

    _running = true;

    std::vector<std::string> commands;
    Core::Error err = parseDuckyScript(script, commands);
    if (err.isError()) {
        _running = false;
        return err;
    }

    uint32_t total = commands.size();
    uint32_t current = 0;

    for (const auto& command : commands) {
        if (!_running) {
            break;
        }

        err = executeDuckyCommand(command);
        if (err.isError()) {
            Serial.printf("[BadUSB] Error executing command: %s\n", command.c_str());
        }

        current++;
        if (_progressCallback) {
            _progressCallback(current, total);
        }

        delay(10);  // Small delay between commands
    }

    _running = false;
    Serial.println("[BadUSB] Script execution completed");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error BadUSBModule::parseDuckyScript(const std::string& script, std::vector<std::string>& commands) {
    commands.clear();

    std::istringstream stream(script);
    std::string line;

    while (std::getline(stream, line)) {
        // Remove comments
        size_t commentPos = line.find("REM ");
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        // Trim whitespace
        while (!line.empty() && (line[0] == ' ' || line[0] == '\t')) {
            line = line.substr(1);
        }
        while (!line.empty() && (line.back() == ' ' || line.back() == '\t' || line.back() == '\r')) {
            line.pop_back();
        }

        if (!line.empty()) {
            commands.push_back(line);
        }
    }

    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error BadUSBModule::executeDuckyCommand(const std::string& command) {
    if (command.empty()) {
        return Core::Error(Core::ErrorCode::SUCCESS);
    }

    // Parse Ducky commands
    if (command.find("STRING ") == 0) {
        std::string text = command.substr(7);
        return typeString(text);
    } else if (command.find("DELAY ") == 0) {
        uint32_t ms = std::stoul(command.substr(6));
        return delay(ms);
    } else if (command.find("GUI ") == 0) {
        std::string key = command.substr(4);
        uint8_t keyCode = getKeyCode(key);
        return pressKey(keyCode, 0x08);  // GUI modifier
    } else if (command.find("ALT ") == 0) {
        std::string key = command.substr(4);
        uint8_t keyCode = getKeyCode(key);
        return pressKey(keyCode, 0x04);  // ALT modifier
    } else if (command.find("CTRL ") == 0) {
        std::string key = command.substr(5);
        uint8_t keyCode = getKeyCode(key);
        return pressKey(keyCode, 0x01);  // CTRL modifier
    } else if (command.find("SHIFT ") == 0) {
        std::string key = command.substr(6);
        uint8_t keyCode = getKeyCode(key);
        return pressKey(keyCode, 0x02);  // SHIFT modifier
    } else if (command == "ENTER") {
        return pressKey(0x28, 0);  // Enter key
    } else if (command == "TAB") {
        return pressKey(0x2B, 0);  // Tab key
    } else if (command == "ESC") {
        return pressKey(0x29, 0);  // Escape key
    } else if (command == "SPACE") {
        return pressKey(0x2C, 0);  // Space key
    } else if (command == "UP") {
        return pressKey(0x52, 0);  // Up arrow
    } else if (command == "DOWN") {
        return pressKey(0x51, 0);  // Down arrow
    } else if (command == "LEFT") {
        return pressKey(0x50, 0);  // Left arrow
    } else if (command == "RIGHT") {
        return pressKey(0x4F, 0);  // Right arrow
    } else if (command == "DELETE") {
        return pressKey(0x4C, 0);  // Delete key
    } else if (command == "BACKSPACE") {
        return pressKey(0x2A, 0);  // Backspace key
    } else if (command == "HOME") {
        return pressKey(0x4A, 0);  // Home key
    } else if (command == "END") {
        return pressKey(0x4D, 0);  // End key
    } else if (command == "PAGEUP") {
        return pressKey(0x4B, 0);  // Page Up
    } else if (command == "PAGEDOWN") {
        return pressKey(0x4E, 0);  // Page Down
    } else if (command.find("F") == 0 && command.length() <= 3) {
        // Function keys F1-F12
        uint8_t fNum = std::stoul(command.substr(1));
        if (fNum >= 1 && fNum <= 12) {
            return pressKey(0x3A + (fNum - 1), 0);
        }
    } else if (command.find("DEFAULT_DELAY ") == 0) {
        // Set default delay between commands
        uint32_t ms = std::stoul(command.substr(14));
        _defaultDelay = ms;
        return Core::Error(Core::ErrorCode::SUCCESS);
    } else if (command.find("REPEAT ") == 0) {
        // Repeat last command N times
        uint32_t count = std::stoul(command.substr(7));
        // TODO: Store last command and repeat
        return Core::Error(Core::ErrorCode::SUCCESS);
    } else if (command.find("SHIFT ") == 0) {
        std::string key = command.substr(6);
        uint8_t keyCode = getKeyCode(key);
        return pressKey(keyCode, 0x02);  // SHIFT modifier
    } else if (command == "ENTER") {
        return pressKey(0x28, 0);  // Enter key
    } else if (command == "TAB") {
        return pressKey(0x2B, 0);  // Tab key
    } else if (command == "ESC" || command == "ESCAPE") {
        return pressKey(0x29, 0);  // Escape key
    } else if (command == "SPACE") {
        return pressKey(0x2C, 0);  // Space key
    } else if (command == "UP") {
        return pressKey(0x52, 0);  // Up arrow
    } else if (command == "DOWN") {
        return pressKey(0x51, 0);  // Down arrow
    } else if (command == "LEFT") {
        return pressKey(0x50, 0);  // Left arrow
    } else if (command == "RIGHT") {
        return pressKey(0x4F, 0);  // Right arrow
    } else if (command == "DELETE") {
        return pressKey(0x4C, 0);  // Delete key
    } else if (command == "BACKSPACE") {
        return pressKey(0x2A, 0);  // Backspace key
    } else if (command == "HOME") {
        return pressKey(0x4A, 0);  // Home key
    } else if (command == "END") {
        return pressKey(0x4D, 0);  // End key
    } else if (command == "PAGEUP") {
        return pressKey(0x4B, 0);  // Page Up
    } else if (command == "PAGEDOWN") {
        return pressKey(0x4E, 0);  // Page Down
    } else if (command.length() == 2 && command[0] == 'F' && isdigit(command[1])) {
        // Function keys F1-F9
        uint8_t fNum = command[1] - '0';
        if (fNum >= 1 && fNum <= 9) {
            return pressKey(0x3A + (fNum - 1), 0);
        }
    } else if (command.length() == 3 && command[0] == 'F' && 
               isdigit(command[1]) && isdigit(command[2])) {
        // Function keys F10-F12
        uint8_t fNum = (command[1] - '0') * 10 + (command[2] - '0');
        if (fNum >= 10 && fNum <= 12) {
            return pressKey(0x3A + (fNum - 1), 0);
        }
    } else if (command.find("DEFAULT_DELAY ") == 0) {
        // Set default delay between commands
        uint32_t ms = std::stoul(command.substr(14));
        _defaultDelay = ms;
        return Core::Error(Core::ErrorCode::SUCCESS);
    }

    return Core::Error(Core::ErrorCode::INVALID_PARAMETER, "Unknown command");
}

Core::Error BadUSBModule::typeString(const std::string& text) {
    // Use BLE HID if available
    if (g_bleModule && g_bleModule->isInitialized()) {
        // Ensure BLE keyboard is active
        if (!_bleKeyboardActive) {
            auto err = g_bleModule->startKeyboard("NightStrike BadUSB");
            if (err.isError()) {
                return err;
            }
            _bleKeyboardActive = true;
        }
        
        // Send text via BLE HID
        return g_bleModule->sendKeys(text);
    }
    
    Serial.printf("[BadUSB] Type string: %s - BLE not available\n", text.c_str());
    return Core::Error(Core::ErrorCode::SUCCESS);
    // Type each character
    for (char c : text) {
        uint8_t keyCode = getKeyCode(std::string(1, c));
        if (keyCode != 0) {
            pressKey(keyCode, 0);
            delay(50);
            releaseKey(keyCode);
            delay(10);
        }
    }
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error BadUSBModule::pressKey(uint8_t key, uint8_t modifiers) {
    // Use BLE HID if available
    if (g_bleModule && g_bleModule->isInitialized()) {
        // Ensure BLE keyboard is active
        if (!_bleKeyboardActive) {
            auto err = g_bleModule->startKeyboard("NightStrike BadUSB");
            if (err.isError()) {
                return err;
            }
            _bleKeyboardActive = true;
        }
        
        // Send raw HID key code via BLE
        return g_bleModule->sendRawHID(key, modifiers);
    }
    
    Serial.printf("[BadUSB] Press key: 0x%02X (modifiers: 0x%02X) - BLE not available\n", key, modifiers);
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error BadUSBModule::releaseKey(uint8_t key) {
    // Key release is handled automatically in BLE HID sendKeys
    Serial.printf("[BadUSB] Release key: 0x%02X\n", key);
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error BadUSBModule::delay(uint32_t ms) {
    ::delay(ms);
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error BadUSBModule::loadScriptFromFile(const std::string& filename) {
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

    return executeDuckyScript(script);
}

Core::Error BadUSBModule::saveScriptToFile(const std::string& filename, const std::string& script) {
    if (!LittleFS.begin(true)) {
        return Core::Error(Core::ErrorCode::STORAGE_NOT_MOUNTED);
    }

    File file = LittleFS.open(filename.c_str(), "w");
    if (!file) {
        return Core::Error(Core::ErrorCode::FILE_WRITE_ERROR);
    }

    file.print(script.c_str());
    file.close();

    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error BadUSBModule::listScripts(std::vector<std::string>& scripts) {
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
        if (name.find(".ducky") != std::string::npos || name.find(".txt") != std::string::npos) {
            scripts.push_back(name);
        }
        file = root.openNextFile();
    }

    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error BadUSBModule::deleteScript(const std::string& filename) {
    if (!LittleFS.begin(true)) {
        return Core::Error(Core::ErrorCode::STORAGE_NOT_MOUNTED);
    }

    if (!LittleFS.remove(filename.c_str())) {
        return Core::Error(Core::ErrorCode::FILE_DELETE_ERROR);
    }

    return Core::Error(Core::ErrorCode::SUCCESS);
}

uint8_t BadUSBModule::getKeyCode(const std::string& key) {
    // HID key code mapping
    static std::map<std::string, uint8_t> keyMap = {
        {"a", 0x04}, {"b", 0x05}, {"c", 0x06}, {"d", 0x07}, {"e", 0x08},
        {"f", 0x09}, {"g", 0x0A}, {"h", 0x0B}, {"i", 0x0C}, {"j", 0x0D},
        {"k", 0x0E}, {"l", 0x0F}, {"m", 0x10}, {"n", 0x11}, {"o", 0x12},
        {"p", 0x13}, {"q", 0x14}, {"r", 0x15}, {"s", 0x16}, {"t", 0x17},
        {"u", 0x18}, {"v", 0x19}, {"w", 0x1A}, {"x", 0x1B}, {"y", 0x1C},
        {"z", 0x1D},
        {"1", 0x1E}, {"2", 0x1F}, {"3", 0x20}, {"4", 0x21}, {"5", 0x22},
        {"6", 0x23}, {"7", 0x24}, {"8", 0x25}, {"9", 0x26}, {"0", 0x27},
    };

    std::string lowerKey = key;
    std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);

    auto it = keyMap.find(lowerKey);
    if (it != keyMap.end()) {
        return it->second;
    }

    return 0;
}

uint8_t BadUSBModule::getModifierCode(const std::string& modifier) {
    static std::map<std::string, uint8_t> modMap = {
        {"CTRL", 0x01}, {"SHIFT", 0x02}, {"ALT", 0x04}, {"GUI", 0x08}
    };

    auto it = modMap.find(modifier);
    if (it != modMap.end()) {
        return it->second;
    }

    return 0;
}

} // namespace Modules
} // namespace NightStrike

