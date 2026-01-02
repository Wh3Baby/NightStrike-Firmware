#include "modules/rf/cc1101_driver.h"
#include <SPI.h>
#include <Arduino.h>

namespace NightStrike {
namespace Modules {

// CC1101 Register addresses
#define CC1101_IOCFG2     0x00
#define CC1101_IOCFG1     0x01
#define CC1101_IOCFG0     0x02
#define CC1101_FIFOTHR    0x03
#define CC1101_SYNC1      0x04
#define CC1101_SYNC0      0x05
#define CC1101_PKTLEN     0x06
#define CC1101_PKTCTRL1   0x07
#define CC1101_PKTCTRL0   0x08
#define CC1101_ADDR       0x09
#define CC1101_CHANNR     0x0A
#define CC1101_FSCTRL1    0x0B
#define CC1101_FSCTRL0    0x0C
#define CC1101_FREQ2      0x0D
#define CC1101_FREQ1      0x0E
#define CC1101_FREQ0      0x0F
#define CC1101_MDMCFG4    0x10
#define CC1101_MDMCFG3    0x11
#define CC1101_MDMCFG2    0x12
#define CC1101_MDMCFG1    0x13
#define CC1101_MDMCFG0    0x14
#define CC1101_DEVIATN    0x15
#define CC1101_MCSM2      0x16
#define CC1101_MCSM1      0x17
#define CC1101_MCSM0      0x18
#define CC1101_FOCCFG     0x19
#define CC1101_BSCFG      0x1A
#define CC1101_AGCTRL2    0x1B
#define CC1101_AGCTRL1    0x1C
#define CC1101_AGCTRL0    0x1D
#define CC1101_WOREVT1    0x1E
#define CC1101_WOREVT0    0x1F
#define CC1101_WORCTRL    0x20
#define CC1101_FREND1     0x21
#define CC1101_FREND0     0x22
#define CC1101_FSCAL3     0x23
#define CC1101_FSCAL2     0x24
#define CC1101_FSCAL1     0x25
#define CC1101_FSCAL0     0x26
#define CC1101_RCCTRL1    0x27
#define CC1101_RCCTRL0    0x28
#define CC1101_FSTEST     0x29
#define CC1101_PTEST      0x2A
#define CC1101_AGCTEST    0x2B
#define CC1101_TEST2      0x2C
#define CC1101_TEST1      0x2D
#define CC1101_TEST0      0x2E

// CC1101 Strobe commands
#define CC1101_SRES       0x30  // Reset
#define CC1101_SFSTXON    0x31  // Enable TX oscillator
#define CC1101_SXOFF      0x32  // Turn off crystal oscillator
#define CC1101_SCAL       0x33  // Calibrate frequency synthesizer
#define CC1101_SRX        0x34  // Enable RX
#define CC1101_STX        0x35  // Enable TX
#define CC1101_SIDLE      0x36  // Exit RX/TX, turn off frequency synthesizer
#define CC1101_SWOR       0x38  // Start automatic RX polling sequence
#define CC1101_SPWD       0x39  // Enter power down mode
#define CC1101_SFRX       0x3A  // Flush RX FIFO
#define CC1101_SFTX       0x3B  // Flush TX FIFO
#define CC1101_SWORRST    0x3C  // Reset real time clock
#define CC1101_SNOP       0x3D  // No operation

// CC1101 Status registers
#define CC1101_PARTNUM    0xF0
#define CC1101_VERSION    0xF1
#define CC1101_FREQEST    0xF2
#define CC1101_LQI        0xF3
#define CC1101_RSSI       0xF4
#define CC1101_MARCSTATE  0xF5
#define CC1101_WORTIME1   0xF6
#define CC1101_WORTIME0   0xF7
#define CC1101_PKTSTATUS  0xF8
#define CC1101_VCO_VC_DAC 0xF9
#define CC1101_TXBYTES    0xFA
#define CC1101_RXBYTES    0xFB

CC1101Driver::CC1101Driver(uint8_t csPin, uint8_t gdo0Pin, uint8_t gdo2Pin)
    : _csPin(csPin), _gdo0Pin(gdo0Pin), _gdo2Pin(gdo2Pin) {
}

bool CC1101Driver::begin() {
    // Initialize SPI
    SPI.begin();
    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, HIGH);

    if (_gdo0Pin > 0) {
        pinMode(_gdo0Pin, INPUT);
    }
    if (_gdo2Pin > 0) {
        pinMode(_gdo2Pin, INPUT);
    }

    // Reset CC1101
    delay(10);
    reset();
    delay(10);

    // Check if CC1101 is present
    uint8_t partnum = readRegister(CC1101_PARTNUM);
    uint8_t version = readRegister(CC1101_VERSION);

    if (partnum != 0x00 || version != 0x14) {
        Serial.printf("[CC1101] Warning: Unexpected partnum=0x%02X, version=0x%02X\n", partnum, version);
        // Continue anyway, might work
    }

    // Configure default settings
    configureRegisters();
    setIdleMode();

    _initialized = true;
    Serial.println("[CC1101] Initialized successfully");
    return true;
}

void CC1101Driver::end() {
    if (!_initialized) return;

    setIdleMode();
    SPI.end();
    _initialized = false;
}

void CC1101Driver::reset() {
    digitalWrite(_csPin, LOW);
    SPI.transfer(CC1101_SRES);
    digitalWrite(_csPin, HIGH);
    delay(1);
}

void CC1101Driver::configureRegisters() {
    // Default configuration for 433.92 MHz, 2-FSK, 38.4 kbps
    writeRegister(CC1101_IOCFG2, 0x0B);  // GDO2 as serial data output
    writeRegister(CC1101_IOCFG1, 0x2E);  // High impedance
    writeRegister(CC1101_IOCFG0, 0x06);  // Asserts when sync word has been sent/received
    writeRegister(CC1101_FIFOTHR, 0x47); // TX: 9, RX: 33
    writeRegister(CC1101_SYNC1, 0xD3);
    writeRegister(CC1101_SYNC0, 0x91);
    writeRegister(CC1101_PKTLEN, 0xFF);   // Variable packet length
    writeRegister(CC1101_PKTCTRL1, 0x04); // Append status bytes
    writeRegister(CC1101_PKTCTRL0, 0x05); // Variable length, CRC enabled
    writeRegister(CC1101_ADDR, 0x00);
    writeRegister(CC1101_CHANNR, 0x00);
    writeRegister(CC1101_FSCTRL1, 0x06);
    writeRegister(CC1101_FSCTRL0, 0x00);
    writeRegister(CC1101_MDMCFG4, 0x5B);  // 500 kHz channel filter BW
    writeRegister(CC1101_MDMCFG3, 0xF8);  // 38.4 kbps
    writeRegister(CC1101_MDMCFG2, 0x13);  // 2-FSK, Manchester encoding disabled
    writeRegister(CC1101_MDMCFG1, 0x22);  // FEC disabled
    writeRegister(CC1101_MDMCFG0, 0xF8);
    writeRegister(CC1101_DEVIATN, 0x47);  // 45.776367 kHz deviation
    writeRegister(CC1101_MCSM2, 0x07);
    writeRegister(CC1101_MCSM1, 0x3F);
    writeRegister(CC1101_MCSM0, 0x18);   // Auto calibrate
    writeRegister(CC1101_FOCCFG, 0x1D);
    writeRegister(CC1101_BSCFG, 0x1C);
    writeRegister(CC1101_AGCTRL2, 0xC7);
    writeRegister(CC1101_AGCTRL1, 0x00);
    writeRegister(CC1101_AGCTRL0, 0xB0);
    writeRegister(CC1101_FREND1, 0xB6);
    writeRegister(CC1101_FREND0, 0x10);
    writeRegister(CC1101_FSCAL3, 0xEA);
    writeRegister(CC1101_FSCAL2, 0x0A);
    writeRegister(CC1101_FSCAL1, 0x00);
    writeRegister(CC1101_FSCAL0, 0x11);
    writeRegister(CC1101_RCCTRL1, 0x41);
    writeRegister(CC1101_RCCTRL0, 0x00);

    // Calibrate
    strobe(CC1101_SCAL);
    delay(1);
}

bool CC1101Driver::setFrequency(uint32_t frequencyHz) {
    if (!_initialized) return false;

    uint32_t freqReg = frequencyToRegisters(frequencyHz);
    
    writeRegister(CC1101_FREQ2, (freqReg >> 16) & 0xFF);
    writeRegister(CC1101_FREQ1, (freqReg >> 8) & 0xFF);
    writeRegister(CC1101_FREQ0, freqReg & 0xFF);

    // Calibrate
    strobe(CC1101_SCAL);
    delay(1);

    Serial.printf("[CC1101] Frequency set to %lu Hz\n", frequencyHz);
    return true;
}

bool CC1101Driver::setPower(uint8_t power) {
    if (!_initialized) return false;
    if (power > 7) power = 7;

    // PA_TABLE values for different power levels
    uint8_t paTable[8] = {0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t paValues[8] = {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60};
    
    // Set power table
    writeBurst(0x3E, paTable, 8);
    writeRegister(CC1101_FREND0, paValues[power]);

    return true;
}

bool CC1101Driver::setDataRate(uint32_t baudRate) {
    if (!_initialized) return false;

    // Calculate MDMCFG3 and MDMCFG4 values
    // This is simplified - full calculation depends on XOSC frequency
    uint8_t mdmcfg4 = 0x5B;  // Default
    uint8_t mdmcfg3 = 0xF8;  // Default

    writeRegister(CC1101_MDMCFG4, mdmcfg4);
    writeRegister(CC1101_MDMCFG3, mdmcfg3);

    return true;
}

bool CC1101Driver::setModulation(uint8_t modType) {
    if (!_initialized) return false;
    if (modType > 4) return false;

    uint8_t mdmcfg2 = readRegister(CC1101_MDMCFG2);
    mdmcfg2 = (mdmcfg2 & 0x8F) | (modType << 4);
    writeRegister(CC1101_MDMCFG2, mdmcfg2);

    return true;
}

bool CC1101Driver::transmit(const uint8_t* data, size_t len) {
    if (!_initialized) return false;
    if (len == 0 || len > 255) return false;

    setIdleMode();
    strobe(CC1101_SFTX);  // Flush TX FIFO

    // Write data to FIFO
    digitalWrite(_csPin, LOW);
    SPI.transfer(CC1101_TXBYTES | 0x40);  // Burst write
    SPI.transfer(len);
    for (size_t i = 0; i < len; ++i) {
        SPI.transfer(data[i]);
    }
    digitalWrite(_csPin, HIGH);

    // Start transmission
    setTransmitMode();

    // Wait for transmission to complete
    uint32_t start = millis();
    while (isTransmitting() && (millis() - start < 1000)) {
        delay(1);
    }

    setIdleMode();
    return true;
}

size_t CC1101Driver::receive(uint8_t* buffer, size_t maxLen, uint32_t timeout) {
    if (!_initialized) return 0;

    setReceiveMode();

    uint32_t start = millis();
    while (!isReceiving() && (millis() - start < timeout)) {
        delay(1);
    }

    if (!isReceiving()) {
        setIdleMode();
        return 0;
    }

    // Wait for packet
    start = millis();
    while (isReceiving() && (millis() - start < timeout)) {
        delay(1);
    }

    // Read FIFO
    uint8_t rxBytes = readRegister(CC1101_RXBYTES);
    if (rxBytes == 0 || rxBytes & 0x80) {  // Underflow or overflow
        setIdleMode();
        strobe(CC1101_SFRX);
        return 0;
    }

    size_t len = rxBytes & 0x7F;
    if (len > maxLen) len = maxLen;

    // Read data
    digitalWrite(_csPin, LOW);
    SPI.transfer(CC1101_RXBYTES | 0x40);  // Burst read
    for (size_t i = 0; i < len; ++i) {
        buffer[i] = SPI.transfer(0x00);
    }
    digitalWrite(_csPin, HIGH);

    setIdleMode();
    strobe(CC1101_SFRX);  // Flush RX FIFO

    return len;
}

bool CC1101Driver::setReceiveMode() {
    strobe(CC1101_SRX);
    delay(1);
    return true;
}

bool CC1101Driver::setTransmitMode() {
    strobe(CC1101_STX);
    delay(1);
    return true;
}

bool CC1101Driver::setIdleMode() {
    strobe(CC1101_SIDLE);
    delay(1);
    return true;
}

int8_t CC1101Driver::getRSSI() {
    uint8_t rssi = readRegister(CC1101_RSSI);
    if (rssi >= 128) {
        return ((int8_t)(rssi - 256) / 2) - 74;
    } else {
        return (rssi / 2) - 74;
    }
}

uint8_t CC1101Driver::getStatus() {
    return readRegister(CC1101_MARCSTATE);
}

bool CC1101Driver::isIdle() {
    return (getStatus() == 0x01);
}

bool CC1101Driver::isTransmitting() {
    uint8_t state = getStatus();
    return (state >= 0x13 && state <= 0x17);
}

bool CC1101Driver::isReceiving() {
    uint8_t state = getStatus();
    return (state >= 0x0D && state <= 0x12);
}

int8_t CC1101Driver::scanFrequency(uint32_t frequencyHz) {
    if (!_initialized) return -128;

    setFrequency(frequencyHz);
    setReceiveMode();
    delay(10);  // Wait for RSSI to stabilize
    int8_t rssi = getRSSI();
    setIdleMode();
    return rssi;
}

void CC1101Driver::writeRegister(uint8_t address, uint8_t value) {
    digitalWrite(_csPin, LOW);
    SPI.transfer(address);
    SPI.transfer(value);
    digitalWrite(_csPin, HIGH);
}

uint8_t CC1101Driver::readRegister(uint8_t address) {
    digitalWrite(_csPin, LOW);
    SPI.transfer(address | 0x80);
    uint8_t value = SPI.transfer(0x00);
    digitalWrite(_csPin, HIGH);
    return value;
}

void CC1101Driver::writeBurst(uint8_t address, const uint8_t* data, size_t len) {
    digitalWrite(_csPin, LOW);
    SPI.transfer(address | 0x40);
    for (size_t i = 0; i < len; ++i) {
        SPI.transfer(data[i]);
    }
    digitalWrite(_csPin, HIGH);
}

void CC1101Driver::readBurst(uint8_t address, uint8_t* data, size_t len) {
    digitalWrite(_csPin, LOW);
    SPI.transfer(address | 0xC0);
    for (size_t i = 0; i < len; ++i) {
        data[i] = SPI.transfer(0x00);
    }
    digitalWrite(_csPin, HIGH);
}

void CC1101Driver::strobe(uint8_t command) {
    digitalWrite(_csPin, LOW);
    SPI.transfer(command);
    digitalWrite(_csPin, HIGH);
}

uint32_t CC1101Driver::frequencyToRegisters(uint32_t frequencyHz) {
    // CC1101 uses 26 MHz crystal (default)
    // Frequency register = (freq * 2^16) / 26000000
    uint64_t freqReg = ((uint64_t)frequencyHz << 16) / 26000000;
    return (uint32_t)freqReg;
}

} // namespace Modules
} // namespace NightStrike

