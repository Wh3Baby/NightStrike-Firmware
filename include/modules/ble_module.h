#pragma once

#include "core/module_interface.h"
#include "core/errors.h"
#include <vector>
#include <string>
#include <functional>

namespace NightStrike {
namespace Modules {

/**
 * @brief BLE module for offensive operations
 *
 * Features:
 * - BLE scanning
 * - Spam attacks (iOS, Android, Windows, Samsung)
 * - Keyboard injection
 * - Device enumeration
 */
class BLEModule : public Core::IModule {
public:
    struct BLEDeviceInfo {
        std::string address;
        std::string name;
        int8_t rssi;
        bool connectable;
    };

    BLEModule();
    ~BLEModule() override = default;

    // IModule interface
    const char* getName() const override { return "BLE"; }
    Core::Error initialize() override;
    Core::Error shutdown() override;
    bool isInitialized() const override { return _initialized; }
    bool isSupported() const override { return true; }

    // Scanning
    Core::Error scanDevices(std::vector<BLEDeviceInfo>& devices, uint32_t duration = 5000);
    Core::Error stopScan();

    // Spam attacks
    Core::Error spamIOS(const std::string& name);
    Core::Error spamAndroid(const std::string& name);
    Core::Error spamWindows(const std::string& name);
    Core::Error spamSamsung(const std::string& name);
    Core::Error spamAll(const std::vector<std::string>& names);

    // Keyboard injection
    Core::Error startKeyboard(const std::string& deviceName);
    Core::Error sendKeys(const std::string& text);
    Core::Error stopKeyboard();

private:
    bool _scanning = false;
    bool _keyboardActive = false;
};

} // namespace Modules
} // namespace NightStrike

