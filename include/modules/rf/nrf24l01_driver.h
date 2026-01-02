#pragma once

#include "modules/rf/rf_driver_interface.h"
#include <Arduino.h>
#include <SPI.h>
#include <stdint.h>

namespace NightStrike {
namespace Modules {

/**
 * @brief NRF24L01 2.4GHz RF transceiver driver
 * 
 * NRF24L01 is a single chip 2.4GHz transceiver with an embedded baseband protocol engine.
 * Supports frequencies 2400-2525 MHz (126 channels).
 * 
 * Popular modules:
 * - NRF24L01+ Module
 * - NRF24L01+ PA/LNA (with external amplifier)
 * - Various breakout boards
 */
class NRF24L01Driver : public IRFDriver {
public:
    NRF24L01Driver(uint8_t cePin, uint8_t csnPin);
    ~NRF24L01Driver() = default;

    // IRFDriver interface
    bool begin() override;
    void end() override;
    bool setFrequency(uint32_t frequencyHz) override;
    bool setPower(uint8_t power) override;  // 0-3 (0 = -18dBm, 3 = 0dBm)
    bool setDataRate(uint32_t baudRate) override;
    bool setModulation(uint8_t modType) override;
    bool transmit(const uint8_t* data, size_t len) override;
    size_t receive(uint8_t* buffer, size_t maxLen, uint32_t timeout = 5000) override;
    bool setReceiveMode() override;
    bool setTransmitMode() override;
    bool setIdleMode() override;
    int8_t getRSSI() override;
    uint8_t getStatus() override;
    bool isIdle() override;
    bool isTransmitting() override;
    bool isReceiving() override;
    int8_t scanFrequency(uint32_t frequencyHz) override;
    const char* getModuleName() const override { return "NRF24L01"; }
    uint32_t getMinFrequency() const override { return 2400000000; }  // 2400 MHz
    uint32_t getMaxFrequency() const override { return 2525000000; }  // 2525 MHz

private:
    uint8_t _cePin;
    uint8_t _csnPin;
    bool _initialized = false;

    void writeRegister(uint8_t address, uint8_t value);
    uint8_t readRegister(uint8_t address);
    void writeBurst(uint8_t address, const uint8_t* data, size_t len);
    void readBurst(uint8_t address, uint8_t* data, size_t len);
    void flushTX();
    void flushRX();
    void powerUp();
    void powerDown();
};

} // namespace Modules
} // namespace NightStrike

