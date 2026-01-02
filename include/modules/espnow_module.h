#pragma once

#include "core/module_interface.h"
#include "core/errors.h"
#include <vector>
#include <string>
#include <functional>

namespace NightStrike {
namespace Modules {

/**
 * @brief ESPNOW module for peer-to-peer communication
 *
 * Features:
 * - Send/Receive files
 * - Send/Receive commands
 * - Peer discovery
 */
class ESPNOWModule : public Core::IModule {
public:
    struct Peer {
        uint8_t mac[6];
        std::string name;
        int8_t rssi;
    };

    ESPNOWModule();
    ~ESPNOWModule() override = default;

    // IModule interface
    const char* getName() const override { return "ESPNOW"; }
    Core::Error initialize() override;
    Core::Error shutdown() override;
    bool isInitialized() const override { return _initialized; }
    bool isSupported() const override { return true; }

    // Peer management
    Core::Error addPeer(const uint8_t* mac);
    Core::Error removePeer(const uint8_t* mac);
    Core::Error getPeers(std::vector<Peer>& peers);

    // File operations
    Core::Error sendFile(const uint8_t* mac, const std::string& filePath);
    Core::Error receiveFile(const std::string& savePath, std::function<void(size_t received, size_t total)> progress = nullptr);

    // Command operations
    Core::Error sendCommand(const uint8_t* mac, const std::string& command);
    Core::Error setCommandCallback(std::function<void(const uint8_t* mac, const std::string& command)> callback);

    // Discovery
    Core::Error startDiscovery(std::function<void(const Peer&)> onPeerFound);
    Core::Error stopDiscovery();

private:
    bool _initialized = false;
    std::vector<Peer> _peers;
    bool _discoveryRunning = false;
    std::function<void(const Peer&)> _discoveryCallback;
    std::function<void(const uint8_t*, const std::string&)> _commandCallback;

    static void onReceiveCallback(const uint8_t* mac, const uint8_t* data, int len);
    static ESPNOWModule* _instance;
};

} // namespace Modules
} // namespace NightStrike

