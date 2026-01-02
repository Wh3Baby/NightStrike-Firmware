#pragma once

#include "errors.h"
#include <cstdint>

namespace NightStrike {
namespace Core {

/**
 * @brief Power management system
 */
class PowerManagement {
public:
    static PowerManagement& getInstance();

    // Initialization
    Error initialize();

    // Sleep modes
    Error enterLightSleep(uint32_t duration_ms);
    Error enterDeepSleep(uint32_t duration_seconds);

    // CPU frequency
    Error setCPUFrequency(uint32_t freq_mhz);
    uint32_t getCPUFrequency() const;

    // Power saving
    Error setWiFiPowerSave(bool enable);

    // Battery
    int getBatteryLevel() const;  // Returns 0-100 or -1 if not available
    bool isCharging() const;

    bool isInitialized() const { return _initialized; }

private:
    PowerManagement() = default;
    ~PowerManagement() = default;
    PowerManagement(const PowerManagement&) = delete;
    PowerManagement& operator=(const PowerManagement&) = delete;

    bool _initialized = false;
};

} // namespace Core
} // namespace NightStrike

