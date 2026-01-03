#pragma once

#include "core/module_interface.h"
#include "core/errors.h"
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace NightStrike {
namespace Modules {

/**
 * @brief Advanced Black Hat tools module
 *
 * Additional offensive security tools:
 * - Network reconnaissance
 * - Credential harvesting
 * - Man-in-the-middle attacks
 * - Exploit frameworks
 */
class BlackHatToolsModule : public Core::IModule {
public:
    BlackHatToolsModule();
    ~BlackHatToolsModule() override = default;

    // IModule interface
    const char* getName() const override { return "BlackHat Tools"; }
    Core::Error initialize() override;
    Core::Error shutdown() override;
    bool isInitialized() const override { return _initialized; }
    bool isSupported() const override { return true; }

    // Network reconnaissance
    Core::Error scanHosts(const std::string& network, std::vector<std::string>& hosts);
    Core::Error portScan(const std::string& host, const std::vector<uint16_t>& ports,
                        std::vector<uint16_t>& openPorts);
    Core::Error serviceDetection(const std::string& host, uint16_t port, std::string& service);

    // Credential harvesting
    Core::Error startCredentialHarvester(const std::string& interface);
    Core::Error stopCredentialHarvester();
    Core::Error getHarvestedCredentials(std::vector<std::pair<std::string, std::string>>& creds);

    // MITM attacks
    Core::Error startARPspoofing(const std::string& target, const std::string& gateway);
    Core::Error stopARPspoofing();
    Core::Error startDNSspoofing(const std::map<std::string, std::string>& dnsMap);
    Core::Error stopDNSspoofing();

    // Packet manipulation
    Core::Error injectPacket(const std::vector<uint8_t>& packet);
    Core::Error capturePackets(std::function<void(const uint8_t*, size_t)> callback,
                             uint32_t count = 0);
    Core::Error stopPacketCapture();

    // Exploit framework
    Core::Error loadExploit(const std::string& name);
    Core::Error executeExploit(const std::string& target, const std::map<std::string, std::string>& params);
    Core::Error listExploits(std::vector<std::string>& exploits);

private:
    bool _initialized = false;
    bool _harvesting = false;
    bool _arpSpoofing = false;
    bool _dnsSpoofing = false;
    bool _capturing = false;
    std::string _arpTarget;
    std::string _arpGateway;
    std::map<std::string, std::string> _dnsMap;
    std::vector<std::pair<std::string, std::string>> _harvestedCreds;
    TaskHandle_t _arpTaskHandle = nullptr;
    TaskHandle_t _dnsTaskHandle = nullptr;
    std::function<void(const uint8_t*, size_t)> _captureCallback;
    uint32_t _captureCount = 0;
    uint32_t _capturedCount = 0;
};

} // namespace Modules
} // namespace NightStrike

