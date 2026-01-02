#pragma once

#include "core/module_interface.h"
#include "core/errors.h"
#include <vector>
#include <string>
#include <functional>

namespace NightStrike {
namespace Modules {

/**
 * @brief RF module for Sub-GHz operations
 *
 * Features:
 * - Sub-GHz transmission/reception
 * - Jammer (full and intermittent)
 * - Spectrum analyzer
 * - Protocol support (433MHz, 868MHz, 915MHz)
 */
class RFModule : public Core::IModule {
public:
    enum class Frequency {
        F433 = 433920000,   // 433.92 MHz
        F868 = 868350000,   // 868.35 MHz
        F915 = 915000000    // 915.00 MHz
    };

    struct RFCode {
        std::vector<uint8_t> data;
        uint32_t frequency;
        uint32_t protocol;
        std::string name;
    };

    RFModule();
    ~RFModule();

    // IModule interface
    const char* getName() const override { return "RF"; }
    Core::Error initialize() override;
    Core::Error shutdown() override;
    bool isInitialized() const override { return _initialized; }
    bool isSupported() const override;

    // Configuration
    Core::Error setFrequency(Frequency freq);
    Core::Error setTXPin(uint8_t pin);
    Core::Error setRXPin(uint8_t pin);
    
    // RF Module configuration (supports multiple modules)
    enum class RFModuleType {
        NONE,
        CC1101,      // Sub-GHz (300-928 MHz) - JAM модули
        NRF24L01,    // 2.4 GHz (2400-2525 MHz)
        AUTO_DETECT  // Try to detect automatically
    };

    Core::Error setRFModule(RFModuleType type, uint8_t csPin, uint8_t pin1 = 0, uint8_t pin2 = 0);
    Core::Error enableRFModule(bool enable = true);
    Core::Error detectRFModule(RFModuleType& detectedType);
    
    // Legacy CC1101 methods (for compatibility)
    Core::Error setCC1101Pins(uint8_t csPin, uint8_t gdo0Pin = 0, uint8_t gdo2Pin = 0);
    Core::Error enableCC1101(bool enable = true);

    // Operations
    Core::Error transmit(const RFCode& code);
    Core::Error receive(RFCode& code, uint32_t timeout = 5000);
    Core::Error startJammer(bool intermittent = false);
    Core::Error stopJammer();
    Core::Error startSpectrumAnalyzer(std::function<void(uint32_t freq, int8_t rssi)> callback);
    Core::Error stopSpectrumAnalyzer();

    // Code management
    Core::Error saveCode(const RFCode& code, const std::string& name);
    Core::Error loadCode(const std::string& name, RFCode& code);
    Core::Error listCodes(std::vector<std::string>& names);

    // Protocol support
    Core::Error setProtocol(const std::string& protocolName);
    Core::Error listProtocols(std::vector<std::string>& protocols);
    Core::Error transmitWithProtocol(const std::vector<uint8_t>& data, const std::string& protocol);
    Core::Error receiveWithProtocol(std::vector<uint8_t>& data, const std::string& protocol, uint32_t timeout = 5000);

private:
    bool _initialized = false;
    bool _jamming = false;
    bool _spectrumActive = false;
    bool _intermittent = false;
    bool _rfModuleEnabled = false;
    RFModuleType _rfModuleType = RFModuleType::NONE;
    Frequency _currentFreq = Frequency::F433;
    uint8_t _txPin = 0;
    uint8_t _rxPin = 0;
    uint8_t _rfCSPin = 0;
    uint8_t _rfPin1 = 0;
    uint8_t _rfPin2 = 0;
    std::function<void(uint32_t, int8_t)> _spectrumCallback;
    
    // RF driver pointer (defined in .cpp to avoid include)
    void* _rfDriver = nullptr;
}; // class RFModule

} // namespace Modules
} // namespace NightStrike

