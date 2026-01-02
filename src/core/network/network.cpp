#include "core/network.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <Arduino.h>

namespace NightStrike {
namespace Core {

Network& Network::getInstance() {
    static Network instance;
    return instance;
}

Error Network::initialize() {
    if (_initialized) {
        return Error(ErrorCode::ALREADY_INITIALIZED);
    }

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Set max power
    esp_wifi_set_max_tx_power(84);

    // Set country code
    wifi_country_t country = {
        .cc = "US",
        .schan = 1,
        .nchan = 13,
        .max_tx_power = 84,
        .policy = WIFI_COUNTRY_POLICY_MANUAL
    };
    esp_wifi_set_country(&country);

    _initialized = true;
    Serial.println("[Network] Network stack initialized");
    return Error(ErrorCode::SUCCESS);
}

Error Network::shutdown() {
    if (!_initialized) {
        return Error(ErrorCode::NOT_INITIALIZED);
    }

    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);

    _initialized = false;
    return Error(ErrorCode::SUCCESS);
}

std::string Network::getMACAddress() const {
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);

    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return std::string(macStr);
}

Error Network::setMACAddress(const std::string& mac) {
    if (!_initialized) {
        return Error(ErrorCode::NOT_INITIALIZED);
    }

    uint8_t macBytes[6];
    if (sscanf(mac.c_str(), "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx",
               &macBytes[0], &macBytes[1], &macBytes[2],
               &macBytes[3], &macBytes[4], &macBytes[5]) != 6) {
        return Error(ErrorCode::INVALID_PARAMETER, "Invalid MAC format");
    }

    esp_wifi_set_mac(WIFI_IF_STA, macBytes);
    return Error(ErrorCode::SUCCESS);
}

} // namespace Core
} // namespace NightStrike

