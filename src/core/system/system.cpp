#include "core/system.h"

#ifdef UNIT_TEST
#include "mocks/arduino_mock.h"
#else
#include <Arduino.h>
#include <esp_system.h>
#endif

namespace NightStrike {
namespace Core {

System& System::getInstance() {
    static System instance;
    return instance;
}

Error System::initialize() {
    if (_initialized) {
        return Error(ErrorCode::ALREADY_INITIALIZED, "System already initialized");
    }

    // Initialize serial
    Serial.begin(115200);
    Serial.setRxBufferSize(4096);

    // Log system info
    Serial.printf("[System] NightStrike Firmware v%s\n", NIGHTSTRIKE_VERSION ? NIGHTSTRIKE_VERSION : "dev");
    Serial.printf("[System] Git commit: %s\n", GIT_COMMIT_HASH ? GIT_COMMIT_HASH : "unknown");
    Serial.printf("[System] Free heap: %u bytes\n", ESP.getFreeHeap());
    Serial.printf("[System] Total heap: %u bytes\n", ESP.getHeapSize());

    if (psramFound()) {
        Serial.printf("[System] PSRAM found: %u bytes free\n", ESP.getFreePsram());
    }

    _initialized = true;
    return Error(ErrorCode::SUCCESS);
}

Error System::shutdown() {
    if (!_initialized) {
        return Error(ErrorCode::NOT_INITIALIZED, "System not initialized");
    }

    _initialized = false;
    return Error(ErrorCode::SUCCESS);
}

System::SystemInfo System::getSystemInfo() const {
    SystemInfo info;
    info.firmwareVersion = NIGHTSTRIKE_VERSION ? NIGHTSTRIKE_VERSION : "dev";
    info.gitCommit = GIT_COMMIT_HASH ? GIT_COMMIT_HASH : "unknown";
    info.freeHeap = ESP.getFreeHeap();
    info.totalHeap = ESP.getHeapSize();
    info.freePSRAM = psramFound() ? ESP.getFreePsram() : 0;
    info.totalPSRAM = psramFound() ? ESP.getPsramSize() : 0;
    return info;
}

Error System::enterDeepSleep(uint32_t seconds) {
#ifndef UNIT_TEST
    esp_sleep_enable_timer_wakeup(seconds * 1000000ULL);
    esp_deep_sleep_start();
#endif
    return Error(ErrorCode::SUCCESS);  // Never reached in real hardware
}

Error System::restart() {
#ifndef UNIT_TEST
    esp_restart();
#endif
    return Error(ErrorCode::SUCCESS);  // Never reached in real hardware
}

} // namespace Core
} // namespace NightStrike

