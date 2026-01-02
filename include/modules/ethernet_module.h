#pragma once

#include "core/module_interface.h"
#include "core/errors.h"
#include <vector>
#include <string>

namespace NightStrike {
namespace Modules {

/**
 * @brief Ethernet module for wired network attacks
 *
 * Features:
 * - ARP Spoofing/Poisoning
 * - DHCP Starvation
 * - MAC Flooding
 * - ARP Scanner
 */
class EthernetModule : public Core::IModule {
public:
    struct Host {
        std::string ip;
        std::string mac;
        std::string hostname;
    };

    EthernetModule();
    ~EthernetModule() override = default;

    // IModule interface
    const char* getName() const override { return "Ethernet"; }
    Core::Error initialize() override;
    Core::Error shutdown() override;
    bool isInitialized() const override { return _initialized; }
    bool isSupported() const override;

    // ARP operations
    Core::Error startARPSpoofing(const std::string& targetIP, const std::string& gatewayIP);
    Core::Error stopARPSpoofing();
    Core::Error scanARP(std::vector<Host>& hosts);

    // DHCP Starvation
    Core::Error startDHCPStarvation();
    Core::Error stopDHCPStarvation();

    // MAC Flooding
    Core::Error startMACFlooding(uint32_t count = 0);  // 0 = infinite
    Core::Error stopMACFlooding();

    // Status
    bool isARPSpoofing() const { return _arpSpoofing; }
    bool isDHCPStarving() const { return _dhcpStarving; }
    bool isMACFlooding() const { return _macFlooding; }

private:
    bool _initialized = false;
    bool _arpSpoofing = false;
    bool _dhcpStarving = false;
    bool _macFlooding = false;
    
    std::string _targetIP;
    std::string _gatewayIP;
    uint32_t _macFloodCount = 0;
    
    // Internal methods
    Core::Error sendARPPacket(const std::string& targetIP, const std::string& targetMAC,
                             const std::string& spoofedIP, const std::string& spoofedMAC);
    void randomizeMAC(uint8_t* mac);
};

} // namespace Modules
} // namespace NightStrike

