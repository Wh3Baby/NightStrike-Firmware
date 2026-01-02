#pragma once

#include "core/module_interface.h"
#include "core/errors.h"
#include <vector>
#include <string>
#include <functional>

namespace NightStrike {
namespace Modules {

/**
 * @brief FM Radio module for broadcast operations
 *
 * Features:
 * - FM Broadcast (standard/reserved/stop)
 * - FM Spectrum analyzer
 * - Traffic Announcement hijacking (planned)
 */
class FMModule : public Core::IModule {
public:
    enum class BroadcastType {
        STANDARD,   // Standard FM broadcast (87.5-108.0 MHz)
        RESERVED,   // Reserved band (76.0-87.5 MHz)
        STOP        // Stop broadcast
    };

    FMModule();
    ~FMModule() override = default;

    // IModule interface
    const char* getName() const override { return "FM Radio"; }
    Core::Error initialize() override;
    Core::Error shutdown() override;
    bool isInitialized() const override { return _initialized; }
    bool isSupported() const override;

    // FM Operations
    Core::Error begin();
    Core::Error setFrequency(uint16_t frequency);  // Frequency in 10kHz units (e.g., 10230 = 102.30 MHz)
    Core::Error startBroadcast(BroadcastType type = BroadcastType::STANDARD);
    Core::Error stopBroadcast();
    Core::Error scanFrequency(uint16_t startFreq, uint16_t endFreq, uint16_t& bestFreq);
    Core::Error startSpectrumAnalyzer(std::function<void(uint16_t freq, int16_t rssi)> callback);
    Core::Error stopSpectrumAnalyzer();

    // Status
    bool isBroadcasting() const { return _broadcasting; }
    uint16_t getCurrentFrequency() const { return _currentFrequency; }

private:
    bool _initialized = false;
    bool _broadcasting = false;
    uint16_t _currentFrequency = 10230;  // Default: 102.30 MHz
    bool _spectrumRunning = false;
    std::function<void(uint16_t, int16_t)> _spectrumCallback;

    // Hardware detection
    bool detectSi4713() const;
    void* _radio = nullptr;  // Adafruit_Si4713* (forward declaration to avoid dependency)
};

} // namespace Modules
} // namespace NightStrike

