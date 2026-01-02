#pragma once

#include "core/module_interface.h"
#include "core/errors.h"
#include <vector>
#include <string>

namespace NightStrike {
namespace Modules {

/**
 * @brief GPS/Wardriving module
 *
 * Features:
 * - GPS tracking
 * - Wardriving (WiFi scanning with GPS coordinates)
 * - Wigle export format
 * - Track recording
 */
class GPSModule : public Core::IModule {
public:
    struct GPSPosition {
        double latitude;
        double longitude;
        double altitude;
        uint8_t satellites;
        bool valid;
    };

    struct WiFiNetwork {
        std::string ssid;
        std::string bssid;
        int8_t rssi;
        uint8_t channel;
        bool encrypted;
        double latitude;
        double longitude;
        uint64_t timestamp;
    };

    GPSModule();
    ~GPSModule() override = default;

    // IModule interface
    const char* getName() const override { return "GPS"; }
    Core::Error initialize() override;
    Core::Error shutdown() override;
    bool isInitialized() const override { return _initialized; }
    bool isSupported() const override;

    // GPS operations
    Core::Error getPosition(GPSPosition& position);
    Core::Error startTracking();
    Core::Error stopTracking();
    Core::Error saveTrack(const std::string& filename);

    // Wardriving
    Core::Error startWardriving();
    Core::Error stopWardriving();
    Core::Error getNetworks(std::vector<WiFiNetwork>& networks);
    Core::Error exportToWigle(const std::string& filename);

    // Configuration
    Core::Error setSerialPort(uint8_t rxPin, uint8_t txPin, uint32_t baud = 9600);

private:
    bool _initialized = false;
    bool _tracking = false;
    bool _wardriving = false;
    
    uint8_t _rxPin = 16;  // Default GPIO16
    uint8_t _txPin = 17;  // Default GPIO17
    uint32_t _baud = 9600;
    
    GPSPosition _lastPosition;
    std::vector<WiFiNetwork> _capturedNetworks;
    std::vector<GPSPosition> _trackPoints;
    
    // Internal methods
    Core::Error parseGPSData();
    void scanAndStoreNetworks();
};

} // namespace Modules
} // namespace NightStrike

