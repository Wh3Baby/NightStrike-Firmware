#pragma once

#include "core/module_interface.h"
#include "core/errors.h"
#include <vector>
#include <string>

namespace NightStrike {
namespace Modules {

/**
 * @brief Other tools module
 *
 * Features:
 * - iButton (1-Wire) support
 * - QR Code generation
 * - Reverse Shell
 * - Audio playback
 */
class OthersModule : public Core::IModule {
public:
    OthersModule();
    ~OthersModule() override = default;

    // IModule interface
    const char* getName() const override { return "Others"; }
    Core::Error initialize() override;
    Core::Error shutdown() override;
    bool isInitialized() const override { return _initialized; }
    bool isSupported() const override { return true; }

    // iButton (1-Wire)
    Core::Error readiButton(std::string& id);
    Core::Error writeiButton(const std::string& id);

    // QR Code
    Core::Error generateQRCode(const std::string& data, const std::string& filename = "");
    Core::Error displayQRCode(const std::string& data);

    // Reverse Shell
    Core::Error startReverseShell(const std::string& host, uint16_t port);
    Core::Error stopReverseShell();
    Core::Error sendCommand(const std::string& command);

    // Audio
    Core::Error playAudio(const std::string& filename);
    Core::Error stopAudio();

private:
    bool _initialized = false;
    bool _reverseShellActive = false;
    uint8_t _iButtonPin = 0;  // GPIO pin for 1-Wire
};

} // namespace Modules
} // namespace NightStrike

