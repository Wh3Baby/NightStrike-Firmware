#pragma once

#include "core/module_interface.h"
#include "core/errors.h"
#include <vector>
#include <string>
#include <functional>

namespace NightStrike {
namespace Modules {

/**
 * @brief NRF24 module for 2.4GHz operations
 *
 * Features:
 * - 2.4GHz spectrum analyzer
 * - NRF24 jammer
 * - Channel hopping
 * - Mousejacking (framework)
 */
class NRF24Module : public Core::IModule {
public:
    struct ChannelInfo {
        uint8_t channel;
        uint8_t signal;
        bool active;
    };

    NRF24Module();
    ~NRF24Module() override = default;

    // IModule interface
    const char* getName() const override { return "NRF24"; }
    Core::Error initialize() override;
    Core::Error shutdown() override;
    bool isInitialized() const override { return _initialized; }
    bool isSupported() const override;

    // Spectrum analyzer
    Core::Error scanSpectrum(std::vector<ChannelInfo>& channels);
    Core::Error startSpectrumAnalyzer(std::function<void(const std::vector<ChannelInfo>&)> callback);
    Core::Error stopSpectrumAnalyzer();

    // Jammer
    Core::Error startJammer(uint8_t channel = 0);  // 0 = all channels
    Core::Error stopJammer();
    Core::Error startChannelHopper(uint32_t interval = 100);  // ms

    // Configuration
    Core::Error setCEPin(uint8_t pin);
    Core::Error setCSPin(uint8_t pin);
    Core::Error setChannel(uint8_t channel);

    // Status
    bool isJamming() const { return _jamming; }
    bool isScanning() const { return _scanning; }

private:
    bool _initialized = false;
    bool _jamming = false;
    bool _scanning = false;
    bool _channelHopping = false;
    
    uint8_t _cePin = 4;   // Default GPIO4
    uint8_t _csPin = 5;   // Default GPIO5
    uint8_t _currentChannel = 0;
    
    std::function<void(const std::vector<ChannelInfo>&)> _spectrumCallback;
    
    // Internal methods
    Core::Error initRadio();
    void jamChannel(uint8_t channel);
};

} // namespace Modules
} // namespace NightStrike

