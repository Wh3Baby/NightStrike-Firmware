#pragma once

#include "core/module_interface.h"
#include "core/errors.h"
#include <WiFi.h>
#include <vector>
#include <string>

namespace NightStrike {
namespace Modules {

/**
 * @brief WiFi module for offensive operations
 *
 * Features:
 * - Deauthentication attacks
 * - Evil Portal (captive portal)
 * - WiFi scanning and wardriving
 * - Packet sniffing
 * - AP attacks
 */
class WiFiModule : public Core::IModule {
public:
    struct AccessPoint {
        std::string ssid;
        std::string bssid;
        int8_t rssi;
        uint8_t channel;
        bool encrypted;
        uint8_t bssidBytes[6];
    };

    struct Client {
        std::string mac;
        int8_t rssi;
        uint8_t macBytes[6];
    };

    WiFiModule();
    ~WiFiModule() override = default;

    // IModule interface
    const char* getName() const override { return "WiFi"; }
    Core::Error initialize() override;
    Core::Error shutdown() override;
    bool isInitialized() const override { return _initialized; }
    bool isSupported() const override { return true; }

    // WiFi operations
    Core::Error scanNetworks(std::vector<AccessPoint>& aps);
    Core::Error connectToAP(const std::string& ssid, const std::string& password);
    Core::Error disconnect();
    Core::Error startAP(const std::string& ssid, const std::string& password = "");
    Core::Error stopAP();

    // Attack functions
    Core::Error deauthAttack(const AccessPoint& ap, uint32_t count = 0);
    Core::Error beaconSpam(const std::vector<std::string>& ssids);
    Core::Error startSniffer(std::function<void(const uint8_t*, size_t)> callback);
    Core::Error stopSniffer();

    // Evil Portal
    Core::Error startEvilPortal(const std::string& ssid, const std::string& portalHtml = "");
    Core::Error stopEvilPortal();

    // Karma Attack (auto Evil Portal on probe requests)
    Core::Error startKarmaAttack(const std::vector<std::string>& ssids);
    Core::Error stopKarmaAttack();

    // TCP Client/Listener
    Core::Error tcpConnect(const std::string& host, uint16_t port, std::function<void(const uint8_t*, size_t)> onData);
    Core::Error tcpDisconnect();
    Core::Error tcpSend(const std::vector<uint8_t>& data);
    Core::Error tcpStartListener(uint16_t port, std::function<void(const std::string&, uint16_t, const uint8_t*, size_t)> onClient);
    Core::Error tcpStopListener();

    // ARP Spoofing
    Core::Error startARPSpoofing(const std::string& targetIP, const std::string& gatewayIP);
    Core::Error stopARPSpoofing();

    // Scan Hosts with Port Scanning
    Core::Error scanHosts(const std::string& network, std::vector<std::string>& hosts);
    Core::Error portScan(const std::string& host, const std::vector<uint16_t>& ports, std::vector<uint16_t>& openPorts);

    // Responder (LLMNR/NBT-NS/mDNS)
    Core::Error startResponder(const std::string& netbiosName = "NIGHTSTRIKE");
    Core::Error stopResponder();
    Core::Error getCapturedHashes(std::vector<std::string>& hashes);

    // TelNet Client
    Core::Error telnetConnect(const std::string& host, uint16_t port = 23);
    Core::Error telnetDisconnect();
    Core::Error telnetSend(const std::string& data);
    Core::Error telnetReceive(std::string& data, size_t maxLen = 1024);
    bool isTelnetConnected() const;

    // SSH Client
    Core::Error sshConnect(const std::string& host, uint16_t port, const std::string& user, const std::string& password);
    Core::Error sshDisconnect();
    Core::Error sshSend(const std::string& command);
    Core::Error sshReceive(std::string& output, size_t maxLen = 4096);
    Core::Error sshExecute(const std::string& command, std::string& output);
    bool isSSHConnected() const;

    // Wireguard Tunneling
    Core::Error startWireguard(const std::string& config);
    Core::Error stopWireguard();
    Core::Error getWireguardStatus(std::string& status);
    bool isWireguardActive() const;

private:
    std::string getDefaultPortalHTML();

    // Status
    bool isConnected() const;
    bool isAPActive() const;
    std::string getIP() const;

private:
    bool _sniffing = false;
    std::function<void(const uint8_t*, size_t)> _snifferCallback;

    static void snifferCallback(void* buf, wifi_promiscuous_pkt_type_t type);
    void sendDeauthFrame(const uint8_t* bssid, uint8_t channel);
};

} // namespace Modules
} // namespace NightStrike

