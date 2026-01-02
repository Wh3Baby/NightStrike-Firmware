#include "core/power_management.h"
#include <esp_sleep.h>
#include <esp_pm.h>
#include <esp_wifi.h>
#include <Arduino.h>

namespace NightStrike {
namespace Core {

PowerManagement& PowerManagement::getInstance() {
    static PowerManagement instance;
    return instance;
}

Error PowerManagement::initialize() {
    if (_initialized) {
        return Error(ErrorCode::ALREADY_INITIALIZED);
    }

    // Configure power management
    esp_pm_config_esp32_t pm_config = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 80,
        .light_sleep_enable = true
    };

    esp_pm_configure(&pm_config);

    _initialized = true;
    Serial.println("[Power] Power management initialized");
    return Error(ErrorCode::SUCCESS);
}

Error PowerManagement::enterLightSleep(uint32_t duration_ms) {
    if (!_initialized) {
        return Error(ErrorCode::NOT_INITIALIZED);
    }

    esp_sleep_enable_timer_wakeup(duration_ms * 1000);
    esp_light_sleep_start();

    return Error(ErrorCode::SUCCESS);
}

Error PowerManagement::enterDeepSleep(uint32_t duration_seconds) {
    esp_sleep_enable_timer_wakeup(duration_seconds * 1000000ULL);
    esp_deep_sleep_start();

    return Error(ErrorCode::SUCCESS);  // Never reached
}

Error PowerManagement::setCPUFrequency(uint32_t freq_mhz) {
    if (freq_mhz != 80 && freq_mhz != 160 && freq_mhz != 240) {
        return Error(ErrorCode::INVALID_PARAMETER);
    }

    setCpuFrequencyMhz(freq_mhz);
    return Error(ErrorCode::SUCCESS);
}

uint32_t PowerManagement::getCPUFrequency() const {
    return getCpuFrequencyMhz();
}

Error PowerManagement::setWiFiPowerSave(bool enable) {
#ifdef M5STICKC_PLUS2
    // M5StickC PLUS2: Set WiFi power save mode
    esp_wifi_set_ps(enable ? WIFI_PS_MIN_MODEM : WIFI_PS_NONE);
    return Error(ErrorCode::SUCCESS);
#else
    // Generic ESP32
    esp_wifi_set_ps(enable ? WIFI_PS_MIN_MODEM : WIFI_PS_NONE);
    return Error(ErrorCode::SUCCESS);
#endif
}

int PowerManagement::getBatteryLevel() const {
#ifdef M5STICKC_PLUS2
    // M5StickC PLUS2: Read battery voltage from ADC (GPIO 35)
    // Battery voltage range: 3.0V - 4.2V
    // ADC reading: 0-4095 for 0-3.3V
    // Battery voltage = (ADC / 4095) * 3.3 * 2 (voltage divider)
    pinMode(35, INPUT);
    int adcValue = analogRead(35);
    float voltage = (adcValue / 4095.0) * 3.3 * 2.0;
    
    // Convert voltage to percentage (3.0V = 0%, 4.2V = 100%)
    int percentage = ((voltage - 3.0) / 1.2) * 100;
    if (percentage < 0) percentage = 0;
    if (percentage > 100) percentage = 100;
    
    return percentage;
#else
    return -1;  // Not available
#endif
}

bool PowerManagement::isCharging() const {
#ifdef M5STICKC_PLUS2
    // M5StickC PLUS2: Check charging status (GPIO 36)
    pinMode(36, INPUT);
    return digitalRead(36) == HIGH;  // HIGH = charging
#else
    return false;  // Not available
#endif
}

} // namespace Core
} // namespace NightStrike

