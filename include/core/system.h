#pragma once

#include "errors.h"

namespace NightStrike {
namespace Core {

/**
 * @brief System initialization and management
 *
 * Handles boot sequence, power management, and system state
 */
class System {
public:
    struct SystemInfo {
        const char* firmwareVersion;
        const char* gitCommit;
        uint32_t freeHeap;
        uint32_t totalHeap;
        uint32_t freePSRAM;
        uint32_t totalPSRAM;
    };

    static System& getInstance();

    // Initialization
    Error initialize();
    Error shutdown();

    // System information
    SystemInfo getSystemInfo() const;

    // Power management
    Error enterDeepSleep(uint32_t seconds);
    Error restart();

    // State management
    bool isInitialized() const { return _initialized; }

private:
    System() = default;
    ~System() = default;
    System(const System&) = delete;
    System& operator=(const System&) = delete;

    bool _initialized = false;
};

} // namespace Core
} // namespace NightStrike

