#include "modules/wifi_module.h"
#include <WiFiClient.h>
#include <Arduino.h>
#include <lwip/sockets.h>
#include <string.h>

namespace NightStrike {
namespace Modules {

static WiFiClient telnetClient;
static bool telnetConnected = false;

Core::Error WiFiModule::telnetConnect(const std::string& host, uint16_t port) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (!isConnected()) {
        return Core::Error(Core::ErrorCode::OPERATION_FAILED, "WiFi not connected");
    }

    if (telnetConnected) {
        telnetDisconnect();
    }

    Serial.printf("[TelNet] Connecting to %s:%d\n", host.c_str(), port);
    
    if (telnetClient.connect(host.c_str(), port)) {
        telnetConnected = true;
        Serial.println("[TelNet] Connected");
        return Core::Error(Core::ErrorCode::SUCCESS);
    }

    return Core::Error(Core::ErrorCode::OPERATION_FAILED, "TelNet connection failed");
}

Core::Error WiFiModule::telnetDisconnect() {
    if (telnetConnected) {
        telnetClient.stop();
        telnetConnected = false;
        Serial.println("[TelNet] Disconnected");
    }
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error WiFiModule::telnetSend(const std::string& data) {
    if (!telnetConnected) {
        return Core::Error(Core::ErrorCode::OPERATION_FAILED, "TelNet not connected");
    }

    size_t written = telnetClient.write((const uint8_t*)data.c_str(), data.length());
    if (written != data.length()) {
        return Core::Error(Core::ErrorCode::OPERATION_FAILED, "TelNet send failed");
    }

    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error WiFiModule::telnetReceive(std::string& data, size_t maxLen) {
    if (!telnetConnected) {
        return Core::Error(Core::ErrorCode::OPERATION_FAILED, "TelNet not connected");
    }

    data.clear();
    
    if (telnetClient.available() > 0) {
        uint8_t buffer[256];
        size_t bytesRead = telnetClient.readBytes(buffer, (maxLen < sizeof(buffer)) ? maxLen : sizeof(buffer));
        
        // Filter TelNet negotiation commands (IAC - 0xFF)
        for (size_t i = 0; i < bytesRead; ++i) {
            if (buffer[i] == 0xFF && i + 1 < bytesRead) {
                // Skip IAC command sequence (IAC + command)
                i += 1;
                continue;
            }
            if (buffer[i] >= 32 || buffer[i] == '\n' || buffer[i] == '\r' || buffer[i] == '\t') {
                data += (char)buffer[i];
            }
        }
    }

    return Core::Error(Core::ErrorCode::SUCCESS);
}

bool WiFiModule::isTelnetConnected() const {
    return telnetConnected && telnetClient.connected();
}

} // namespace Modules
} // namespace NightStrike

