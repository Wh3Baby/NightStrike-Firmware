#pragma once

#include <stdint.h>
#include <stddef.h>

namespace NightStrike {
namespace Modules {

/**
 * @brief Universal RF driver interface
 * 
 * Abstract interface for different RF transceiver modules:
 * - CC1101 (Sub-GHz)
 * - NRF24L01 (2.4 GHz)
 * - SX1278/SX1276 (LoRa)
 * - RFM69/RFM95 (Sub-GHz/LoRa)
 * - And others
 */
class IRFDriver {
public:
    virtual ~IRFDriver() = default;

    // Initialization
    virtual bool begin() = 0;
    virtual void end() = 0;

    // Configuration
    virtual bool setFrequency(uint32_t frequencyHz) = 0;
    virtual bool setPower(uint8_t power) = 0;  // 0-7 or 0-15 depending on module
    virtual bool setDataRate(uint32_t baudRate) = 0;
    virtual bool setModulation(uint8_t modType) = 0;

    // Operations
    virtual bool transmit(const uint8_t* data, size_t len) = 0;
    virtual size_t receive(uint8_t* buffer, size_t maxLen, uint32_t timeout = 5000) = 0;
    virtual bool setReceiveMode() = 0;
    virtual bool setTransmitMode() = 0;
    virtual bool setIdleMode() = 0;

    // Status
    virtual int8_t getRSSI() = 0;
    virtual uint8_t getStatus() = 0;
    virtual bool isIdle() = 0;
    virtual bool isTransmitting() = 0;
    virtual bool isReceiving() = 0;

    // Spectrum analyzer
    virtual int8_t scanFrequency(uint32_t frequencyHz) = 0;

    // Module info
    virtual const char* getModuleName() const = 0;
    virtual uint32_t getMinFrequency() const = 0;
    virtual uint32_t getMaxFrequency() const = 0;
};

} // namespace Modules
} // namespace NightStrike

