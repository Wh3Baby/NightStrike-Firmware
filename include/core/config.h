#pragma once

#include "errors.h"
#include <ArduinoJson.h>
#include <map>
#include <string>
#include <vector>

namespace NightStrike {
namespace Core {

/**
 * @brief Configuration manager with validation
 *
 * Improved over Bruce: type-safe, validated, secure defaults
 */
class Config {
public:
    struct WiFiCredential {
        std::string ssid;
        std::string password;
    };

    struct SecuritySettings {
        std::string adminPassword;  // Must be changed on first boot
        bool passwordChanged = false;
        bool requirePasswordChange = true;
    };

    struct DisplaySettings {
        uint8_t brightness = 100;
        uint8_t dimTimeout = 10;  // seconds
        uint8_t rotation = 1;
        bool inverted = false;
    };

    struct NetworkSettings {
        std::string apSSID = "NightStrike";
        std::string apPassword = "";  // Must be set by user
        std::map<std::string, WiFiCredential> savedNetworks;
    };

    Config();
    ~Config() = default;

    // Load/Save configuration
    Error load();
    Error save();

    // Validation
    Error validate() const;

    // Getters/Setters with validation
    Error setAdminPassword(const std::string& password);
    const std::string& getAdminPassword() const { return security.adminPassword; }

    Error setBrightness(uint8_t brightness);
    uint8_t getBrightness() const { return display.brightness; }

    // Security: Check if password was changed
    bool isPasswordChanged() const { return security.passwordChanged; }
    bool requiresPasswordChange() const { return security.requirePasswordChange && !security.passwordChanged; }

    // Configuration sections
    SecuritySettings security;
    DisplaySettings display;
    NetworkSettings network;

private:
    static constexpr const char* CONFIG_FILE = "/nightstrike.conf";

    JsonDocument toJson() const;
    Error fromJson(JsonDocument& doc);
    Error validatePassword(const std::string& password) const;
};

} // namespace Core
} // namespace NightStrike

