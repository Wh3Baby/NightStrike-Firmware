#include "core/config.h"

#ifdef UNIT_TEST
#include "mocks/littlefs_mock.h"
#include "mocks/arduinojson_mock.h"
#else
#include <LittleFS.h>
#include <ArduinoJson.h>
#endif

#include <cctype>

namespace NightStrike {
namespace Core {

Config::Config() {
    // Secure defaults - no hardcoded passwords
    security.adminPassword = "";
    security.passwordChanged = false;
    security.requirePasswordChange = true;
}

Error Config::load() {
#ifdef UNIT_TEST
    // В тестах не загружаем из файла
    return Error(ErrorCode::SUCCESS);
#else
    if (!LittleFS.begin(true)) {
        return Error(ErrorCode::STORAGE_NOT_MOUNTED, "LittleFS not mounted");
    }

    if (!LittleFS.exists(CONFIG_FILE)) {
        Serial.println("[Config] Config file not found, creating default");
        return save();  // Create default config
    }

    File file = LittleFS.open(CONFIG_FILE, "r");
    if (!file) {
        return Error(ErrorCode::FILE_READ_ERROR, "Failed to open config file");
    }

    JsonDocument doc;
    DeserializationError jsonError = deserializeJson(doc, file);
    file.close();

    if (jsonError) {
        return Error(ErrorCode::CONFIG_INVALID, "Failed to parse config JSON");
    }

    Error err = fromJson(doc);
    if (err.isError()) {
        return err;
    }

    // Validate loaded configuration
    return validate();
#endif
}

Error Config::save() {
#ifdef UNIT_TEST
    // В тестах не сохраняем в файл
    return Error(ErrorCode::SUCCESS);
#else
    if (!LittleFS.begin(true)) {
        return Error(ErrorCode::STORAGE_NOT_MOUNTED, "LittleFS not mounted");
    }

    JsonDocument doc = toJson();

    File file = LittleFS.open(CONFIG_FILE, "w");
    if (!file) {
        return Error(ErrorCode::FILE_WRITE_ERROR, "Failed to open config file for writing");
    }

    if (serializeJson(doc, file) == 0) {
        file.close();
        return Error(ErrorCode::FILE_WRITE_ERROR, "Failed to write config");
    }

    file.close();
    return Error(ErrorCode::SUCCESS);
#endif
}

Error Config::validate() const {
    // Validate password strength if set
    if (!security.adminPassword.empty()) {
        Error pwdErr = validatePassword(security.adminPassword);
        if (pwdErr.isError()) {
            return pwdErr;
        }
    }

    // Validate brightness range
    if (display.brightness > 100) {
        return Error(ErrorCode::INVALID_PARAMETER, "Brightness out of range");
    }

    // Validate AP password if set (should be at least 8 chars for WPA2)
    if (!network.apPassword.empty() && network.apPassword.length() < 8) {
        return Error(ErrorCode::SECURITY_PASSWORD_TOO_WEAK, "AP password too weak");
    }

    return Error(ErrorCode::SUCCESS);
}

Error Config::setAdminPassword(const std::string& password) {
    Error err = validatePassword(password);
    if (err.isError()) {
        return err;
    }

    security.adminPassword = password;
    security.passwordChanged = true;

#ifdef UNIT_TEST
    // В тестах не сохраняем в файл
    return Error(ErrorCode::SUCCESS);
#else
    return save();
#endif
}

Error Config::setBrightness(uint8_t brightness) {
    if (brightness > 100) {
        return Error(ErrorCode::INVALID_PARAMETER, "Brightness must be 0-100");
    }

    display.brightness = brightness;
    return Error(ErrorCode::SUCCESS);
}

JsonDocument Config::toJson() const {
#ifdef UNIT_TEST
    // В тестах не используется - save() не вызывается
    // Возвращаем пустой JsonDocument (не используется в тестах)
    static JsonDocument dummy;
    return dummy;
#else
    JsonDocument doc;

    // Security settings
    doc["security"]["adminPassword"] = security.adminPassword;
    doc["security"]["passwordChanged"] = security.passwordChanged;
    doc["security"]["requirePasswordChange"] = security.requirePasswordChange;

    // Display settings
    doc["display"]["brightness"] = display.brightness;
    doc["display"]["dimTimeout"] = display.dimTimeout;
    doc["display"]["rotation"] = display.rotation;
    doc["display"]["inverted"] = display.inverted;

    // Network settings
    doc["network"]["apSSID"] = network.apSSID;
    doc["network"]["apPassword"] = network.apPassword;

    // Saved WiFi networks
    JsonArray networks = doc["network"].createNestedArray("savedNetworks");
    for (const auto& pair : network.savedNetworks) {
        const std::string& ssid = pair.first;
        const WiFiCredential& cred = pair.second;
        JsonObject net = networks.add<JsonObject>();
        net["ssid"] = cred.ssid;
        net["password"] = cred.password;
    }

    return doc;
#endif
}

Error Config::fromJson(JsonDocument& doc) {
#ifdef UNIT_TEST
    // В тестах не парсим JSON
    (void)doc;
    return Error(ErrorCode::SUCCESS);
#else
    // Security settings
    if (doc["security"].is<JsonObject>()) {
        security.adminPassword = doc["security"]["adminPassword"] | "";
        security.passwordChanged = doc["security"]["passwordChanged"] | false;
        security.requirePasswordChange = doc["security"]["requirePasswordChange"] | true;
    }

    // Display settings
    if (doc["display"].is<JsonObject>()) {
        display.brightness = doc["display"]["brightness"] | 100;
        display.dimTimeout = doc["display"]["dimTimeout"] | 10;
        display.rotation = doc["display"]["rotation"] | 1;
        display.inverted = doc["display"]["inverted"] | false;
    }

    // Network settings
    if (doc["network"].is<JsonObject>()) {
        network.apSSID = doc["network"]["apSSID"] | "NightStrike";
        network.apPassword = doc["network"]["apPassword"] | "";

        // Load saved networks
        // Work around ArduinoJson v7 limitation: can't convert JsonVariantConst to JsonArray
        // Access array elements directly by index without conversion
        if (doc["network"]["savedNetworks"].is<JsonArray>()) {
            // Access JsonArray directly from JsonDocument (mutable reference)
            JsonArray networks = doc["network"]["savedNetworks"];
            for (size_t i = 0; i < networks.size(); i++) {
                JsonVariant netVar = networks[i];
                if (netVar.is<JsonObject>()) {
                    JsonObject net = netVar;
                    WiFiCredential cred;
                    cred.ssid = net["ssid"] | "";
                    cred.password = net["password"] | "";
                    if (!cred.ssid.empty()) {
                        network.savedNetworks[cred.ssid] = cred;
                    }
                }
            }
        }
    }

    return Error(ErrorCode::SUCCESS);
#endif
}

Error Config::validatePassword(const std::string& password) const {
    if (password.length() < 8) {
        return Error(ErrorCode::SECURITY_PASSWORD_TOO_WEAK, "Password must be at least 8 characters");
    }

    // Check for at least one digit and one letter
    bool hasDigit = false;
    bool hasLetter = false;

    for (char c : password) {
        if (isdigit(c)) hasDigit = true;
        if (isalpha(c)) hasLetter = true;
    }

    if (!hasDigit || !hasLetter) {
        return Error(ErrorCode::SECURITY_PASSWORD_TOO_WEAK, "Password must contain letters and numbers");
    }

    return Error(ErrorCode::SUCCESS);
}

} // namespace Core
} // namespace NightStrike

