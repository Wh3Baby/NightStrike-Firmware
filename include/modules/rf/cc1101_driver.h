#pragma once

#include "modules/rf/rf_driver_interface.h"
#include <Arduino.h>
#include <SPI.h>
#include <stdint.h>

namespace NightStrike {
namespace Modules {

/**
 * @brief CC1101 RF transceiver driver
 * 
 * CC1101 is a low-cost sub-1 GHz RF transceiver designed for very low-power
 * wireless applications. Supports frequencies 300-348 MHz, 387-464 MHz, and 779-928 MHz.
 * 
 * Popular modules:
 * - CC1101 Module (various manufacturers)
 * - JAM Module with CC1101
 * - RF1101SE
 */
class CC1101Driver : public IRFDriver {
public:
    CC1101Driver(uint8_t csPin, uint8_t gdo0Pin = 0, uint8_t gdo2Pin = 0);
    ~CC1101Driver() = default;

    // Initialization
    bool begin() override;
    void end() override;

    // Configuration
    bool setFrequency(uint32_t frequencyHz) override;
    bool setPower(uint8_t power) override;  // 0-7 (0 = min, 7 = max)
    bool setDataRate(uint32_t baudRate) override;
    bool setModulation(uint8_t modType) override;  // 0=2-FSK, 1=GFSK, 2=ASK/OOK, 3=4-FSK, 4=MSK

    // Operations
    bool transmit(const uint8_t* data, size_t len) override;
    size_t receive(uint8_t* buffer, size_t maxLen, uint32_t timeout = 5000) override;
    bool setReceiveMode() override;
    bool setTransmitMode() override;
    bool setIdleMode() override;

    // Status
    int8_t getRSSI() override;
    uint8_t getStatus() override;
    bool isIdle() override;
    bool isTransmitting() override;
    bool isReceiving() override;

    // Spectrum analyzer
    int8_t scanFrequency(uint32_t frequencyHz) override;

    // Module info
    const char* getModuleName() const override { return "CC1101"; }
    uint32_t getMinFrequency() const override { return 300000000; }  // 300 MHz
    uint32_t getMaxFrequency() const override { return 928000000; }  // 928 MHz

private:
    uint8_t _csPin;
    uint8_t _gdo0Pin;
    uint8_t _gdo2Pin;
    bool _initialized = false;

    // SPI communication
    void writeRegister(uint8_t address, uint8_t value);
    uint8_t readRegister(uint8_t address);
    void writeBurst(uint8_t address, const uint8_t* data, size_t len);
    void readBurst(uint8_t address, uint8_t* data, size_t len);
    void strobe(uint8_t command);

    // Configuration helpers
    void reset();
    void configureRegisters();
    uint32_t frequencyToRegisters(uint32_t frequencyHz);
};

} // namespace Modules
} // namespace NightStrike

