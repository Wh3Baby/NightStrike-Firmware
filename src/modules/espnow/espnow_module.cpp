#include "modules/espnow_module.h"
#include "core/storage.h"
#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>
#include <algorithm>
#include <LittleFS.h>
#include <FS.h>

namespace NightStrike {
namespace Modules {

ESPNOWModule* ESPNOWModule::_instance = nullptr;

ESPNOWModule::ESPNOWModule() {
    _instance = this;
}

Core::Error ESPNOWModule::initialize() {
    if (_initialized) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }

    if (esp_now_init() != ESP_OK) {
        return Core::Error(Core::ErrorCode::OPERATION_FAILED, "ESPNOW init failed");
    }

    esp_now_register_recv_cb(onReceiveCallback);

    Serial.println("[ESPNOW] Module initialized");
    _initialized = true;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error ESPNOWModule::shutdown() {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    stopDiscovery();
    esp_now_deinit();
    
    _initialized = false;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error ESPNOWModule::addPeer(const uint8_t* mac) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, mac, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        return Core::Error(Core::ErrorCode::OPERATION_FAILED, "Failed to add peer");
    }

    Peer peer;
    memcpy(peer.mac, mac, 6);
    peer.name = "";
    peer.rssi = 0;
    _peers.push_back(peer);

    Serial.printf("[ESPNOW] Peer added: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error ESPNOWModule::removePeer(const uint8_t* mac) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (esp_now_del_peer(mac) != ESP_OK) {
        return Core::Error(Core::ErrorCode::OPERATION_FAILED, "Failed to remove peer");
    }

    _peers.erase(
        std::remove_if(_peers.begin(), _peers.end(),
            [mac](const Peer& p) {
                return memcmp(p.mac, mac, 6) == 0;
            }),
        _peers.end()
    );

    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error ESPNOWModule::getPeers(std::vector<Peer>& peers) {
    peers = _peers;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error ESPNOWModule::sendFile(const uint8_t* mac, const std::string& filePath) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    // Read file from LittleFS
    File file = LittleFS.open(filePath.c_str(), "r");
    if (!file) {
        return Core::Error(Core::ErrorCode::FILE_READ_ERROR, "Failed to open file");
    }

    size_t fileSize = file.size();
    const size_t chunkSize = 200;  // ESPNOW max payload is 250 bytes
    uint8_t buffer[chunkSize];
    uint32_t sequence = 0;
    size_t totalSent = 0;

    Serial.printf("[ESPNOW] Sending file %s (%zu bytes) to peer\n", filePath.c_str(), fileSize);

    // Send file header: "FILE:" + filename + ":" + size
    std::string header = "FILE:" + filePath + ":" + std::to_string(fileSize);
    if (esp_now_send(mac, (const uint8_t*)header.c_str(), header.length()) != ESP_OK) {
        file.close();
        return Core::Error(Core::ErrorCode::OPERATION_FAILED, "Failed to send file header");
    }
    delay(50);

    // Send file data in chunks
    while (file.available()) {
        size_t bytesRead = file.readBytes((char*)buffer, chunkSize);
        
        // Prepend sequence number (4 bytes) to chunk
        uint8_t packet[chunkSize + 4];
        packet[0] = (sequence >> 24) & 0xFF;
        packet[1] = (sequence >> 16) & 0xFF;
        packet[2] = (sequence >> 8) & 0xFF;
        packet[3] = sequence & 0xFF;
        memcpy(packet + 4, buffer, bytesRead);

        if (esp_now_send(mac, packet, bytesRead + 4) != ESP_OK) {
            file.close();
            return Core::Error(Core::ErrorCode::OPERATION_FAILED, "Failed to send file chunk");
        }

        totalSent += bytesRead;
        sequence++;
        delay(20);  // Small delay between chunks
    }

    // Send end marker
    std::string endMarker = "FILE_END:" + std::to_string(sequence);
    esp_now_send(mac, (const uint8_t*)endMarker.c_str(), endMarker.length());

    file.close();
    Serial.printf("[ESPNOW] File sent: %zu bytes in %u chunks\n", totalSent, sequence);
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

// Static variables for file reception
static File receivingFile;
static size_t receivingFileSize = 0;
static size_t receivingFileReceived = 0;
static uint32_t receivingSequence = 0;
static std::string receivingFilePath;
static std::function<void(size_t, size_t)> receivingProgress;

Core::Error ESPNOWModule::receiveFile(const std::string& savePath, std::function<void(size_t, size_t)> progress) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    receivingFilePath = savePath;
    receivingProgress = progress;
    receivingFileSize = 0;
    receivingFileReceived = 0;
    receivingSequence = 0;

    // Create file for writing
    receivingFile = LittleFS.open(savePath.c_str(), "w");
    if (!receivingFile) {
        return Core::Error(Core::ErrorCode::FILE_WRITE_ERROR, "Failed to create file");
    }

    Serial.printf("[ESPNOW] Ready to receive file: %s\n", savePath.c_str());
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error ESPNOWModule::sendCommand(const uint8_t* mac, const std::string& command) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (esp_now_send(mac, (const uint8_t*)command.c_str(), command.length()) != ESP_OK) {
        return Core::Error(Core::ErrorCode::OPERATION_FAILED, "Failed to send command");
    }

    Serial.printf("[ESPNOW] Command sent: %s\n", command.c_str());
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error ESPNOWModule::setCommandCallback(std::function<void(const uint8_t*, const std::string&)> callback) {
    _commandCallback = callback;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error ESPNOWModule::startDiscovery(std::function<void(const Peer&)> onPeerFound) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    _discoveryCallback = onPeerFound;
    _discoveryRunning = true;
    
    Serial.println("[ESPNOW] Discovery started");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error ESPNOWModule::stopDiscovery() {
    _discoveryRunning = false;
    _discoveryCallback = nullptr;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

void ESPNOWModule::onReceiveCallback(const uint8_t* mac, const uint8_t* data, int len) {
    if (!_instance) return;

    std::string message((char*)data, len);
    
    // Check if it's a file header
    if (message.find("FILE:") == 0) {
        // Parse header: "FILE:filename:size"
        size_t nameStart = 5;
        size_t nameEnd = message.find(":", nameStart);
        size_t sizeStart = nameEnd + 1;
        
        if (nameEnd != std::string::npos && sizeStart < message.length()) {
            std::string filename = message.substr(nameStart, nameEnd - nameStart);
            size_t fileSize = std::stoul(message.substr(sizeStart));
            
            receivingFileSize = fileSize;
            receivingFileReceived = 0;
            receivingSequence = 0;
            
            Serial.printf("[ESPNOW] Receiving file: %s (%zu bytes)\n", filename.c_str(), fileSize);
        }
        return;
    }
    
    // Check if it's file data (starts with 4-byte sequence number)
    if (len > 4 && receivingFile) {
        uint32_t seq = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
        
        if (seq == receivingSequence) {
            receivingFile.write(data + 4, len - 4);
            receivingFileReceived += (len - 4);
            receivingSequence++;
            
            if (receivingProgress) {
                receivingProgress(receivingFileReceived, receivingFileSize);
            }
            
            if (receivingFileReceived >= receivingFileSize) {
                receivingFile.close();
                Serial.printf("[ESPNOW] File received: %zu bytes\n", receivingFileReceived);
                receivingFile = File();
            }
        }
        return;
    }
    
    // Check if it's file end marker
    if (message.find("FILE_END:") == 0) {
        if (receivingFile) {
            receivingFile.close();
            receivingFile = File();
        }
        Serial.println("[ESPNOW] File transfer complete");
        return;
    }
    
    // Check if it's a command
    if (_instance->_commandCallback) {
        _instance->_commandCallback(mac, message);
    }

    Serial.printf("[ESPNOW] Received from %02X:%02X:%02X:%02X:%02X:%02X: %s\n",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], message.c_str());
}

} // namespace Modules
} // namespace NightStrike

