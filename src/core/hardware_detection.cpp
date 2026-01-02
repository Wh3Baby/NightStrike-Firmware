#include "core/hardware_detection.h"
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Arduino.h>

namespace NightStrike {
namespace Core {

HardwareDetection& HardwareDetection::getInstance() {
    static HardwareDetection instance;
    return instance;
}

Error HardwareDetection::detectAll() {
    if (_detected) {
        return Error(ErrorCode::SUCCESS);
    }

    Serial.println("[HW] Starting hardware detection...");

    // Identify board first
    _info.boardName = identifyBoard();
    Serial.printf("[HW] Board: %s\n", _info.boardName.c_str());

    // Initialize I2C for detection
    Wire.begin();
#ifndef UNIT_TEST
    delay(10);
#endif

    // Detect display
    _info.display = detectDisplay();
    Serial.printf("[HW] Display: %s\n", 
        _info.display == DisplayType::ST7789V2 ? "ST7789v2" :
        _info.display == DisplayType::ILI9341 ? "ILI9341" :
        _info.display == DisplayType::ST7735 ? "ST7735" :
        _info.display == DisplayType::NONE ? "None" : "Unknown");

    // Detect IMU
    _info.imu = detectIMU();
    Serial.printf("[HW] IMU: %s\n",
        _info.imu == IMUType::MPU6886 ? "MPU6886" :
        _info.imu == IMUType::MPU6050 ? "MPU6050" :
        _info.imu == IMUType::MPU9250 ? "MPU9250" :
        _info.imu == IMUType::NONE ? "None" : "Unknown");

    // Detect RTC
    _info.rtc = detectRTC();
    Serial.printf("[HW] RTC: %s\n",
        _info.rtc == RTCType::BM8563 ? "BM8563" :
        _info.rtc == RTCType::DS3231 ? "DS3231" :
        _info.rtc == RTCType::PCF8563 ? "PCF8563" :
        _info.rtc == RTCType::NONE ? "None" : "Unknown");

    // Detect other modules
    _info.hasIR = detectIR();
    _info.hasMic = detectMic();
    _info.hasBuzzer = detectBuzzer();
    _info.hasLED = detectLED();
    _info.hasSDCard = detectSDCard();

    Serial.printf("[HW] IR: %s, Mic: %s, Buzzer: %s, LED: %s, SD: %s\n",
        _info.hasIR ? "Yes" : "No",
        _info.hasMic ? "Yes" : "No",
        _info.hasBuzzer ? "Yes" : "No",
        _info.hasLED ? "Yes" : "No",
        _info.hasSDCard ? "Yes" : "No");

    _detected = true;
    Serial.println("[HW] Hardware detection complete");
    return Error(ErrorCode::SUCCESS);
}

std::string HardwareDetection::identifyBoard() {
#ifdef M5STICKC_PLUS2
    return "M5StickC PLUS2";
#elif defined(M5STACK_CARDPUTER)
    return "M5Stack Cardputer";
#elif defined(M5STACK_CORE)
    return "M5Stack Core";
#elif defined(M5STACK_CORE2)
    return "M5Stack Core2";
#elif defined(M5STACK_CORES3)
    return "M5Stack CoreS3";
#elif defined(LILYGO_T_EMBED)
    return "Lilygo T-Embed";
#elif defined(LILYGO_T_DECK)
    return "Lilygo T-Deck";
#elif defined(LILYGO_T_DISPLAY_S3)
    return "Lilygo T-Display-S3";
#elif defined(ESP32_S3)
    return "ESP32-S3 DevKit";
#elif defined(ESP32_C5)
    return "ESP32-C5 DevKit";
#elif defined(CYD_2432S028)
    return "CYD-2432S028";
#elif defined(ARDUINO_M5STICKC_PLUS)
    return "M5StickC PLUS";
#elif defined(ARDUINO_M5STICKC)
    return "M5StickC";
#elif defined(ARDUINO_ESP32_DEV)
    return "ESP32 DevKit";
#elif defined(ARDUINO_ESP32S3_DEV)
    return "ESP32-S3 DevKit";
#else
    // Try to detect by hardware
    // M5StickC PLUS2 has specific I2C devices
    if (probeI2C(0x68) && probeI2C(0x51)) {  // MPU6886 + BM8563
        return "M5StickC PLUS2 (detected)";
    }
    // M5Stack Cardputer has TCA8418 keyboard controller
    if (probeI2C(0x34)) {  // TCA8418
        return "M5Stack Cardputer (detected)";
    }
    return "Generic ESP32";
#endif
}

HardwareDetection::DisplayType HardwareDetection::detectDisplay() {
#ifdef HAS_SCREEN
#ifdef M5STICKC_PLUS2
    // M5StickC PLUS2 uses ST7789v2
    return DisplayType::ST7789V2;
#else
    // Try to detect display type
    DisplayType type = probeDisplayST7789();
    if (type != DisplayType::NONE) return type;
    
    type = probeDisplayILI9341();
    if (type != DisplayType::NONE) return type;
    
    return DisplayType::UNKNOWN;
#endif
#else
    return DisplayType::NONE;
#endif
}

HardwareDetection::IMUType HardwareDetection::detectIMU() {
#ifdef HAS_IMU
#ifdef M5STICKC_PLUS2
    // M5StickC PLUS2 uses MPU6886
    return IMUType::MPU6886;
#else
    // Try to detect IMU
    IMUType type = probeIMUMPU6886();
    if (type != IMUType::NONE) return type;
    
    type = probeIMUMPU6050();
    if (type != IMUType::NONE) return type;
    
    return IMUType::UNKNOWN;
#endif
#else
    // Try I2C detection anyway
    if (probeI2C(0x68)) {  // Common IMU address
        return probeIMUMPU6886();
    }
    return IMUType::NONE;
#endif
}

HardwareDetection::RTCType HardwareDetection::detectRTC() {
#ifdef HAS_RTC
#ifdef M5STICKC_PLUS2
    // M5StickC PLUS2 uses BM8563
    return RTCType::BM8563;
#else
    // Try to detect RTC
    RTCType type = probeRTCBM8563();
    if (type != RTCType::NONE) return type;
    
    type = probeRTCDS3231();
    if (type != RTCType::NONE) return type;
    
    return RTCType::UNKNOWN;
#endif
#else
    // Try I2C detection anyway
    if (probeI2C(0x51)) {  // BM8563 address
        return probeRTCBM8563();
    }
    if (probeI2C(0x68)) {  // DS3231 address (same as IMU, need to distinguish)
        return probeRTCDS3231();
    }
    return RTCType::NONE;
#endif
}

bool HardwareDetection::detectIR() {
#ifdef HAS_IR
    return true;
#else
    // IR is usually on GPIO, check if pin responds
    // M5StickC PLUS2: IR on GPIO 9
#ifndef UNIT_TEST
    pinMode(9, INPUT_PULLUP);
    delay(1);
    bool detected = digitalRead(9) != 0;  // Basic check
    return detected;
#else
    return false;  // Mock for tests
#endif
#endif
}

bool HardwareDetection::detectMic() {
#ifdef HAS_MIC
    return true;
#else
    // Microphone detection is difficult without specific hardware
    // M5StickC PLUS2 has built-in mic
    return false;  // Would need specific detection
#endif
}

bool HardwareDetection::detectBuzzer() {
#ifdef HAS_BUZZER
    return true;
#else
    // Buzzer is usually on GPIO with PWM
    // M5StickC PLUS2: Buzzer on GPIO 2
#ifndef UNIT_TEST
    pinMode(2, OUTPUT);
    // Try to generate a tone (very short, inaudible)
    tone(2, 20000, 1);  // 20kHz, 1ms
    delay(2);
    noTone(2);
    return true;  // Assume present if we can control it
#else
    return false;  // Mock for tests
#endif
#endif
}

bool HardwareDetection::detectLED() {
#ifdef HAS_LED
    return true;
#else
    // LED is usually on GPIO
    // M5StickC PLUS2: LED on GPIO 10
#ifndef UNIT_TEST
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);
    delay(1);
    digitalWrite(10, LOW);
    return true;  // Assume present if we can control it
#else
    return false;  // Mock for tests
#endif
#endif
}

bool HardwareDetection::detectSDCard() {
    // Try to initialize SD card
    if (!SD.begin()) {
        return false;
    }
    
    uint8_t cardType = SD.cardType();
    return cardType != CARD_NONE;
}

bool HardwareDetection::probeI2C(uint8_t address) {
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    return error == 0;
}

bool HardwareDetection::probeSPI() {
    // Basic SPI probe - check if we can communicate
    SPI.begin();
#ifndef UNIT_TEST
    delay(10);
#endif
    return true;  // SPI is usually always available
}

HardwareDetection::DisplayType HardwareDetection::probeDisplayST7789() {
    // ST7789 detection - try to read ID register
    // This requires SPI communication
    // For now, if HAS_SCREEN is defined, assume ST7789
    return DisplayType::ST7789V2;
}

HardwareDetection::DisplayType HardwareDetection::probeDisplayILI9341() {
    // ILI9341 detection
    return DisplayType::NONE;  // Not implemented yet
}

HardwareDetection::IMUType HardwareDetection::probeIMUMPU6886() {
    // MPU6886 is at I2C address 0x68
    if (probeI2C(0x68)) {
        // Try to read WHO_AM_I register (0x75)
        Wire.beginTransmission(0x68);
        Wire.write(0x75);
        if (Wire.endTransmission() == 0) {
            Wire.requestFrom(0x68, 1);
            if (Wire.available()) {
                uint8_t whoami = Wire.read();
                // MPU6886 WHO_AM_I = 0x19
                if (whoami == 0x19) {
                    return IMUType::MPU6886;
                }
            }
        }
    }
    return IMUType::NONE;
}

HardwareDetection::IMUType HardwareDetection::probeIMUMPU6050() {
    // MPU6050 is at I2C address 0x68
    if (probeI2C(0x68)) {
        Wire.beginTransmission(0x68);
        Wire.write(0x75);
        if (Wire.endTransmission() == 0) {
            Wire.requestFrom(0x68, 1);
            if (Wire.available()) {
                uint8_t whoami = Wire.read();
                // MPU6050 WHO_AM_I = 0x68
                if (whoami == 0x68) {
                    return IMUType::MPU6050;
                }
            }
        }
    }
    return IMUType::NONE;
}

HardwareDetection::RTCType HardwareDetection::probeRTCBM8563() {
    // BM8563 is at I2C address 0x51
    if (probeI2C(0x51)) {
        // Try to read control register
        Wire.beginTransmission(0x51);
        Wire.write(0x00);
        if (Wire.endTransmission() == 0) {
            Wire.requestFrom(0x51, 1);
            if (Wire.available()) {
                // BM8563 detected
                return RTCType::BM8563;
            }
        }
    }
    return RTCType::NONE;
}

HardwareDetection::RTCType HardwareDetection::probeRTCDS3231() {
    // DS3231 is at I2C address 0x68 (same as IMU!)
    // Need to distinguish by reading specific register
    if (probeI2C(0x68)) {
        Wire.beginTransmission(0x68);
        Wire.write(0x0F);  // Control/Status register
        if (Wire.endTransmission() == 0) {
            Wire.requestFrom(0x68, 1);
            if (Wire.available()) {
                // DS3231 has specific register layout
                // This is a simplified check
                return RTCType::DS3231;
            }
        }
    }
    return RTCType::NONE;
}

} // namespace Core
} // namespace NightStrike

