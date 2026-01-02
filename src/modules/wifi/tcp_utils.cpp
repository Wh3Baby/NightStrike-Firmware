#include "modules/wifi_module.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <Arduino.h>

namespace NightStrike {
namespace Modules {

// TCP Client state
static WiFiClient* g_tcpClient = nullptr;
static std::function<void(const uint8_t*, size_t)> g_tcpClientCallback = nullptr;

// TCP Server state
static WiFiServer* g_tcpServer = nullptr;
static std::function<void(const std::string&, uint16_t, const uint8_t*, size_t)> g_tcpServerCallback = nullptr;
static std::vector<WiFiClient> g_tcpServerClients;

Core::Error WiFiModule::tcpConnect(const std::string& host, uint16_t port,
                                   std::function<void(const uint8_t*, size_t)> onData) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        return Core::Error(Core::ErrorCode::NETWORK_NOT_CONNECTED);
    }
    
    if (g_tcpClient && g_tcpClient->connected()) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED, "TCP client already connected");
    }
    
    if (g_tcpClient) {
        delete g_tcpClient;
    }
    
    g_tcpClient = new WiFiClient();
    g_tcpClientCallback = onData;
    
    IPAddress targetIP;
    if (!targetIP.fromString(host.c_str())) {
        // Try to resolve hostname
        if (!WiFi.hostByName(host.c_str(), targetIP)) {
            delete g_tcpClient;
            g_tcpClient = nullptr;
            return Core::Error(Core::ErrorCode::NETWORK_CONNECTION_FAILED, "Failed to resolve hostname");
        }
    }
    
    g_tcpClient->setTimeout(5000);
    
    if (!g_tcpClient->connect(targetIP, port)) {
        delete g_tcpClient;
        g_tcpClient = nullptr;
        return Core::Error(Core::ErrorCode::NETWORK_CONNECTION_FAILED);
    }
    
    Serial.printf("[TCP] Connected to %s:%d\n", host.c_str(), port);
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error WiFiModule::tcpDisconnect() {
    if (g_tcpClient) {
        if (g_tcpClient->connected()) {
            g_tcpClient->stop();
        }
        delete g_tcpClient;
        g_tcpClient = nullptr;
    }
    
    g_tcpClientCallback = nullptr;
    Serial.println("[TCP] Client disconnected");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error WiFiModule::tcpSend(const std::vector<uint8_t>& data) {
    if (!g_tcpClient || !g_tcpClient->connected()) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED, "TCP client not connected");
    }
    
    size_t written = g_tcpClient->write(data.data(), data.size());
    if (written != data.size()) {
        return Core::Error(Core::ErrorCode::OPERATION_FAILED, "Failed to send all data");
    }
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error WiFiModule::tcpStartListener(uint16_t port,
                                        std::function<void(const std::string&, uint16_t, const uint8_t*, size_t)> onClient) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }
    
    if (g_tcpServer) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }
    
    g_tcpServer = new WiFiServer(port);
    g_tcpServerCallback = onClient;
    g_tcpServer->begin();
    
    Serial.printf("[TCP] Listener started on port %d\n", port);
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error WiFiModule::tcpStopListener() {
    if (g_tcpServer) {
        g_tcpServer->stop();
        delete g_tcpServer;
        g_tcpServer = nullptr;
    }
    
    g_tcpServerClients.clear();
    g_tcpServerCallback = nullptr;
    
    Serial.println("[TCP] Listener stopped");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

// Helper function to process TCP server (should be called in loop)
void processTCPServer() {
    if (!g_tcpServer) return;
    
    // Check for new clients
    if (g_tcpServer->hasClient()) {
        WiFiClient newClient = g_tcpServer->available();
        g_tcpServerClients.push_back(newClient);
        Serial.printf("[TCP] New client connected: %s:%d\n",
                     newClient.remoteIP().toString().c_str(), newClient.remotePort());
    }
    
    // Process existing clients
    for (auto it = g_tcpServerClients.begin(); it != g_tcpServerClients.end();) {
        WiFiClient& client = *it;
        
        if (client.connected()) {
            if (client.available() > 0) {
                uint8_t buffer[1024];
                size_t len = client.available();
                if (len > sizeof(buffer)) len = sizeof(buffer);
                
                size_t read = client.read(buffer, len);
                if (read > 0 && g_tcpServerCallback) {
                    std::string clientIP = client.remoteIP().toString().c_str();
                    uint16_t clientPort = client.remotePort();
                    g_tcpServerCallback(clientIP, clientPort, buffer, read);
                }
            }
            ++it;
        } else {
            // Client disconnected
            Serial.printf("[TCP] Client disconnected: %s:%d\n",
                         client.remoteIP().toString().c_str(), client.remotePort());
            client.stop();
            it = g_tcpServerClients.erase(it);
        }
    }
}

// Helper function to process TCP client (should be called in loop)
void processTCPClient() {
    if (!g_tcpClient || !g_tcpClient->connected()) return;
    
    if (g_tcpClient->available() > 0) {
        uint8_t buffer[1024];
        size_t len = g_tcpClient->available();
        if (len > sizeof(buffer)) len = sizeof(buffer);
        
        size_t read = g_tcpClient->read(buffer, len);
        if (read > 0 && g_tcpClientCallback) {
            g_tcpClientCallback(buffer, read);
        }
    }
}

} // namespace Modules
} // namespace NightStrike

