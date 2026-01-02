#include "modules/wifi_module.h"
#include <esp_wifi.h>
#include <Arduino.h>

namespace NightStrike {
namespace Modules {

Core::Error WiFiModule::beaconSpam(const std::vector<std::string>& ssids) {
    if (!_initialized || ssids.empty()) {
        return Core::Error(Core::ErrorCode::INVALID_PARAMETER);
    }

    // Set to AP mode
    WiFi.mode(WIFI_AP);

    // Create beacon frames for each SSID
    for (const auto& ssid : ssids) {
        // Beacon frame structure
        uint8_t beaconFrame[128];
        size_t frameLen = 0;

        // Frame Control
        beaconFrame[frameLen++] = 0x80;  // Type: Management, Subtype: Beacon
        beaconFrame[frameLen++] = 0x00;

        // Duration
        beaconFrame[frameLen++] = 0x00;
        beaconFrame[frameLen++] = 0x00;

        // Destination (broadcast)
        for (int i = 0; i < 6; ++i) {
            beaconFrame[frameLen++] = 0xFF;
        }

        // Source (random MAC)
        uint8_t sourceMAC[6];
        esp_read_mac(sourceMAC, ESP_MAC_WIFI_SOFTAP);
        for (int i = 0; i < 6; ++i) {
            beaconFrame[frameLen++] = sourceMAC[i];
        }

        // BSSID (same as source)
        for (int i = 0; i < 6; ++i) {
            beaconFrame[frameLen++] = sourceMAC[i];
        }

        // Sequence
        beaconFrame[frameLen++] = 0x00;
        beaconFrame[frameLen++] = 0x00;

        // Timestamp
        for (int i = 0; i < 8; ++i) {
            beaconFrame[frameLen++] = 0x00;
        }

        // Beacon interval
        beaconFrame[frameLen++] = 0x64;  // 100ms
        beaconFrame[frameLen++] = 0x00;

        // Capability info
        beaconFrame[frameLen++] = 0x01;
        beaconFrame[frameLen++] = 0x04;

        // SSID element
        uint8_t ssidLen = ssid.length() > 32 ? 32 : ssid.length();
        beaconFrame[frameLen++] = 0x00;  // Element ID (SSID)
        beaconFrame[frameLen++] = ssidLen;
        for (size_t i = 0; i < ssidLen; ++i) {
            beaconFrame[frameLen++] = ssid[i];
        }

        // Supported rates
        beaconFrame[frameLen++] = 0x01;  // Element ID
        beaconFrame[frameLen++] = 0x08;  // Length
        beaconFrame[frameLen++] = 0x82;  // 1 Mbps
        beaconFrame[frameLen++] = 0x84;  // 2 Mbps
        beaconFrame[frameLen++] = 0x8B;  // 5.5 Mbps
        beaconFrame[frameLen++] = 0x96;  // 11 Mbps
        beaconFrame[frameLen++] = 0x0C;  // 6 Mbps
        beaconFrame[frameLen++] = 0x12;  // 9 Mbps
        beaconFrame[frameLen++] = 0x18;  // 12 Mbps
        beaconFrame[frameLen++] = 0x24;  // 18 Mbps

        // Send beacon frame
        esp_wifi_80211_tx(WIFI_IF_AP, beaconFrame, frameLen, false);

        delay(10);
    }

    Serial.printf("[WiFi] Beacon spam: %zu SSIDs\n", ssids.size());
    return Core::Error(Core::ErrorCode::SUCCESS);
}

} // namespace Modules
} // namespace NightStrike

