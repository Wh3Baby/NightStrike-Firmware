#include "modules/rf/nrf24l01_driver.h"
#include <SPI.h>
#include <Arduino.h>

namespace NightStrike {
namespace Modules {

NRF24L01Driver::NRF24L01Driver(uint8_t cePin, uint8_t csnPin)
    : _cePin(cePin), _csnPin(csnPin) {
}

bool NRF24L01Driver::begin() {
    SPI.begin();
    pinMode(_cePin, OUTPUT);
    pinMode(_csnPin, OUTPUT);
    digitalWrite(_cePin, LOW);
    digitalWrite(_csnPin, HIGH);
    
    // Reset and configure
    powerUp();
    delay(5);
    
    _initialized = true;
    Serial.println("[NRF24L01] Initialized");
    return true;
}

void NRF24L01Driver::end() {
    if (!_initialized) return;
    powerDown();
    SPI.end();
    _initialized = false;
}

bool NRF24L01Driver::setFrequency(uint32_t frequencyHz) {
    // NRF24L01 uses channels 0-125 (2400-2525 MHz)
    uint8_t channel = (frequencyHz - 2400000000) / 1000000;
    if (channel > 125) channel = 125;
    writeRegister(0x05, channel);
    return true;
}

bool NRF24L01Driver::setPower(uint8_t power) {
    uint8_t rfSetup = readRegister(0x06);
    rfSetup = (rfSetup & 0xF9) | ((power & 0x03) << 1);
    writeRegister(0x06, rfSetup);
    return true;
}

bool NRF24L01Driver::setDataRate(uint32_t baudRate) {
    uint8_t rfSetup = readRegister(0x06);
    if (baudRate >= 2000000) {
        rfSetup |= 0x08;  // 2 Mbps
    } else {
        rfSetup &= 0xF7;  // 1 Mbps
    }
    writeRegister(0x06, rfSetup);
    return true;
}

bool NRF24L01Driver::setModulation(uint8_t modType) {
    // NRF24L01 only supports GFSK
    return true;
}

bool NRF24L01Driver::transmit(const uint8_t* data, size_t len) {
    if (!_initialized || len == 0 || len > 32) return false;
    
    flushTX();
    writeBurst(0xA0, data, len);  // W_TX_PAYLOAD
    setTransmitMode();
    
    // Wait for transmission
    uint32_t start = millis();
    while (isTransmitting() && (millis() - start < 100)) {
        delay(1);
    }
    
    setIdleMode();
    return true;
}

size_t NRF24L01Driver::receive(uint8_t* buffer, size_t maxLen, uint32_t timeout) {
    if (!_initialized) return 0;
    
    setReceiveMode();
    
    uint32_t start = millis();
    while (!isReceiving() && (millis() - start < timeout)) {
        delay(1);
    }
    
    if (isReceiving()) {
        delay(1);  // Wait for packet
        uint8_t status = readRegister(0x07);
        if (status & 0x40) {  // RX_DR
            size_t len = readRegister(0x60);  // R_RX_PL_WID
            if (len > maxLen) len = maxLen;
            readBurst(0x61, buffer, len);  // R_RX_PAYLOAD
            writeRegister(0x07, 0x40);  // Clear RX_DR
            setIdleMode();
            return len;
        }
    }
    
    setIdleMode();
    return 0;
}

bool NRF24L01Driver::setReceiveMode() {
    digitalWrite(_cePin, LOW);
    writeRegister(0x00, 0x0F);  // CONFIG: PWR_UP, PRIM_RX
    digitalWrite(_cePin, HIGH);
    return true;
}

bool NRF24L01Driver::setTransmitMode() {
    digitalWrite(_cePin, LOW);
    writeRegister(0x00, 0x0E);  // CONFIG: PWR_UP
    digitalWrite(_cePin, HIGH);
    return true;
}

bool NRF24L01Driver::setIdleMode() {
    digitalWrite(_cePin, LOW);
    return true;
}

int8_t NRF24L01Driver::getRSSI() {
    uint8_t rpd = readRegister(0x09);
    return (rpd & 0x01) ? -64 : -84;  // Simplified
}

uint8_t NRF24L01Driver::getStatus() {
    return readRegister(0x07);
}

bool NRF24L01Driver::isIdle() {
    return !digitalRead(_cePin);
}

bool NRF24L01Driver::isTransmitting() {
    uint8_t status = getStatus();
    return (status & 0x20) != 0;  // TX_DS
}

bool NRF24L01Driver::isReceiving() {
    uint8_t status = getStatus();
    return (status & 0x40) != 0;  // RX_DR
}

int8_t NRF24L01Driver::scanFrequency(uint32_t frequencyHz) {
    setFrequency(frequencyHz);
    setReceiveMode();
    delay(10);
    int8_t rssi = getRSSI();
    setIdleMode();
    return rssi;
}

void NRF24L01Driver::writeRegister(uint8_t address, uint8_t value) {
    digitalWrite(_csnPin, LOW);
    SPI.transfer(address | 0x20);
    SPI.transfer(value);
    digitalWrite(_csnPin, HIGH);
}

uint8_t NRF24L01Driver::readRegister(uint8_t address) {
    digitalWrite(_csnPin, LOW);
    SPI.transfer(address);
    uint8_t value = SPI.transfer(0x00);
    digitalWrite(_csnPin, HIGH);
    return value;
}

void NRF24L01Driver::writeBurst(uint8_t address, const uint8_t* data, size_t len) {
    digitalWrite(_csnPin, LOW);
    SPI.transfer(address | 0x20);
    for (size_t i = 0; i < len; ++i) {
        SPI.transfer(data[i]);
    }
    digitalWrite(_csnPin, HIGH);
}

void NRF24L01Driver::readBurst(uint8_t address, uint8_t* data, size_t len) {
    digitalWrite(_csnPin, LOW);
    SPI.transfer(address);
    for (size_t i = 0; i < len; ++i) {
        data[i] = SPI.transfer(0x00);
    }
    digitalWrite(_csnPin, HIGH);
}

void NRF24L01Driver::flushTX() {
    digitalWrite(_csnPin, LOW);
    SPI.transfer(0xE1);
    digitalWrite(_csnPin, HIGH);
}

void NRF24L01Driver::flushRX() {
    digitalWrite(_csnPin, LOW);
    SPI.transfer(0xE2);
    digitalWrite(_csnPin, HIGH);
}

void NRF24L01Driver::powerUp() {
    writeRegister(0x00, 0x0E);  // CONFIG: PWR_UP
    delay(2);
}

void NRF24L01Driver::powerDown() {
    writeRegister(0x00, 0x00);  // CONFIG: Power down
}

} // namespace Modules
} // namespace NightStrike

