#pragma once

#include "errors.h"
#include <stdint.h>
#include <string>

namespace NightStrike {
namespace Core {

/**
 * @brief Hardware detection and auto-configuration
 * 
 * Automatically detects connected hardware modules:
 * - Display (ST7789v2, ILI9341, etc.)
 * - IMU (MPU6886, MPU6050, etc.)
 * - RTC (BM8563, DS3231, etc.)
 * - IR transmitter
 * - Microphone
 * - Buzzer
 * - LED
 */
class HardwareDetection {
public:
    enum class DisplayType {
        NONE,
        ST7789V2,      // M5StickC PLUS2
        ILI9341,
        ST7735,
        UNKNOWN
    };

    enum class IMUType {
        NONE,
        MPU6886,       // M5StickC PLUS2
        MPU6050,
        MPU9250,
        UNKNOWN
    };

    enum class RTCType {
        NONE,
        BM8563,        // M5StickC PLUS2
        DS3231,
        PCF8563,
        UNKNOWN
    };

    struct HardwareInfo {
        DisplayType display = DisplayType::NONE;
        IMUType imu = IMUType::NONE;
        RTCType rtc = RTCType::NONE;
        bool hasIR = false;
        bool hasMic = false;
        bool hasBuzzer = false;
        bool hasLED = false;
        bool hasSDCard = false;
        std::string boardName = "Unknown";
    };

    static HardwareDetection& getInstance();
    
    // Detection
    Error detectAll();
    HardwareInfo getInfo() const { return _info; }
    
    // Individual detection
    DisplayType detectDisplay();
    IMUType detectIMU();
    RTCType detectRTC();
    bool detectIR();
    bool detectMic();
    bool detectBuzzer();
    bool detectLED();
    bool detectSDCard();
    
    // Board identification
    std::string identifyBoard();

private:
    HardwareDetection() = default;
    ~HardwareDetection() = default;
    HardwareDetection(const HardwareDetection&) = delete;
    HardwareDetection& operator=(const HardwareDetection&) = delete;

    HardwareInfo _info;
    bool _detected = false;

    // Detection helpers
    bool probeI2C(uint8_t address);
    bool probeSPI();
    DisplayType probeDisplayST7789();
    DisplayType probeDisplayILI9341();
    IMUType probeIMUMPU6886();
    IMUType probeIMUMPU6050();
    RTCType probeRTCBM8563();
    RTCType probeRTCDS3231();
};

} // namespace Core
} // namespace NightStrike

