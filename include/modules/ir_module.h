#pragma once

#include "core/module_interface.h"
#include "core/errors.h"
#include <vector>
#include <string>

namespace NightStrike {
namespace Modules {

/**
 * @brief IR (Infrared) module for remote control operations
 *
 * Features:
 * - TV-B-Gone (universal TV off)
 * - IR receiver/recording
 * - IR transmitter/replay
 * - Custom protocol support (NEC, RC5, RC6, SIRC, etc.)
 * - IR jammer
 */
class IRModule : public Core::IModule {
public:
    struct IRCode {
        std::string protocol;  // NEC, RC5, RC6, SIRC, etc.
        uint32_t address;
        uint32_t command;
        std::vector<uint16_t> rawTimings;  // Raw timing data
    };

    IRModule();
    ~IRModule() override = default;

    // IModule interface
    const char* getName() const override { return "IR"; }
    Core::Error initialize() override;
    Core::Error shutdown() override;
    bool isInitialized() const override { return _initialized; }
    bool isSupported() const override;

    // IR operations
    Core::Error sendCode(const IRCode& code);
    Core::Error sendRaw(const std::vector<uint16_t>& timings, uint32_t frequency = 38000);
    Core::Error receiveCode(IRCode& code, uint32_t timeout = 5000);
    Core::Error recordCode(IRCode& code, uint32_t timeout = 10000);

    // TV-B-Gone
    Core::Error tvBGone(bool region = false);  // false = US, true = EU

    // IR Jammer
    Core::Error startJammer(uint32_t frequency = 38000);
    Core::Error stopJammer();

    // Protocol support
    Core::Error sendNEC(uint32_t address, uint32_t command);
    Core::Error sendNECext(uint32_t address, uint32_t command);  // Extended NEC
    Core::Error sendRC5(uint32_t address, uint32_t command);
    Core::Error sendRC5X(uint32_t address, uint32_t command);   // Extended RC5
    Core::Error sendRC6(uint32_t address, uint32_t command);
    Core::Error sendSIRC(uint32_t address, uint32_t command);
    Core::Error sendSIRC15(uint32_t address, uint32_t command);  // 15-bit SIRC
    Core::Error sendSIRC20(uint32_t address, uint32_t command);    // 20-bit SIRC
    Core::Error sendSamsung32(uint32_t address, uint32_t command); // Samsung 32-bit
    Core::Error sendSony(uint32_t address, uint32_t command, uint8_t bits = 12);  // Sony (12/15/20 bit)

    // Configuration
    Core::Error setTXPin(uint8_t pin);
    Core::Error setRXPin(uint8_t pin);
    Core::Error setFrequency(uint32_t frequency);

private:
    bool _initialized = false;
    bool _jamming = false;
    uint8_t _txPin = 4;  // Default GPIO4
    uint8_t _rxPin = 5;  // Default GPIO5
    uint32_t _frequency = 38000;  // 38kHz default
};

} // namespace Modules
} // namespace NightStrike

