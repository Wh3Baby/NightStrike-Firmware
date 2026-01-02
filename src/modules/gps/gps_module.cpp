#include "modules/gps_module.h"
#include "modules/wifi_module.h"
#include "core/storage.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <sstream>
#include <iomanip>

namespace NightStrike {
namespace Modules {

// External WiFi module instance
extern WiFiModule* g_wifiModule;

GPSModule::GPSModule() {
}

Core::Error GPSModule::initialize() {
    if (_initialized) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }

    // Initialize GPS serial
    // Note: TinyGPS++ library would be used here
    // For now, create framework
    
    Serial.printf("[GPS] Module initialized (RX: %d, TX: %d, Baud: %lu)\n",
                 _rxPin, _txPin, _baud);
    Serial.println("[GPS] Note: TinyGPS++ library required for full functionality");
    
    _initialized = true;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error GPSModule::shutdown() {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    stopTracking();
    stopWardriving();
    
    _initialized = false;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

bool GPSModule::isSupported() const {
    // GPS requires serial connection
    // Check if pins are configured
    return _rxPin > 0 && _txPin > 0;
}

Core::Error GPSModule::getPosition(GPSPosition& position) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    // TODO: Parse GPS data from serial
    // TinyGPS++ gps;
    // while (Serial2.available() > 0) {
    //     gps.encode(Serial2.read());
    // }
    // 
    // if (gps.location.isValid()) {
    //     position.latitude = gps.location.lat();
    //     position.longitude = gps.location.lng();
    //     position.altitude = gps.altitude.meters();
    //     position.satellites = gps.satellites.value();
    //     position.valid = true;
    // } else {
    //     position.valid = false;
    // }

    // For now, return last known position
    position = _lastPosition;
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error GPSModule::startTracking() {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (_tracking) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }

    _tracking = true;
    _trackPoints.clear();
    
    Serial.println("[GPS] Tracking started");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error GPSModule::stopTracking() {
    if (!_tracking) {
        return Core::Error(Core::ErrorCode::SUCCESS);
    }

    _tracking = false;
    Serial.printf("[GPS] Tracking stopped (%zu points recorded)\n", _trackPoints.size());
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error GPSModule::saveTrack(const std::string& filename) {
    if (!LittleFS.begin(true)) {
        return Core::Error(Core::ErrorCode::STORAGE_NOT_MOUNTED);
    }

    File file = LittleFS.open(filename.c_str(), "w");
    if (!file) {
        return Core::Error(Core::ErrorCode::FILE_WRITE_ERROR);
    }

    // Save track in GPX format
    file.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
    file.println("<gpx version=\"1.1\">");
    file.println("<trk>");
    file.println("<name>NightStrike Track</name>");
    file.println("<trkseg>");
    
    for (const auto& point : _trackPoints) {
        if (point.valid) {
            file.printf("<trkpt lat=\"%.6f\" lon=\"%.6f\">", point.latitude, point.longitude);
            file.printf("<ele>%.2f</ele>", point.altitude);
            file.println("</trkpt>");
        }
    }
    
    file.println("</trkseg>");
    file.println("</trk>");
    file.println("</gpx>");
    file.close();

    Serial.printf("[GPS] Track saved to %s\n", filename.c_str());
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error GPSModule::startWardriving() {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (!g_wifiModule || !g_wifiModule->isInitialized()) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED, "WiFi module not initialized");
    }

    if (_wardriving) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }

    _wardriving = true;
    _capturedNetworks.clear();
    
    Serial.println("[GPS] Wardriving started");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error GPSModule::stopWardriving() {
    if (!_wardriving) {
        return Core::Error(Core::ErrorCode::SUCCESS);
    }

    _wardriving = false;
    Serial.printf("[GPS] Wardriving stopped (%zu networks captured)\n", _capturedNetworks.size());
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error GPSModule::getNetworks(std::vector<WiFiNetwork>& networks) {
    networks = _capturedNetworks;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error GPSModule::exportToWigle(const std::string& filename) {
    if (!LittleFS.begin(true)) {
        return Core::Error(Core::ErrorCode::STORAGE_NOT_MOUNTED);
    }

    File file = LittleFS.open(filename.c_str(), "w");
    if (!file) {
        return Core::Error(Core::ErrorCode::FILE_WRITE_ERROR);
    }

    // Wigle CSV format header
    file.println("WigleWifi-1.4,appRelease=NightStrike,model=ESP32,release=1.0.0,device=ESP32,display=NightStrike,board=ESP32,brand=NightStrike");
    file.println("MAC,SSID,AuthMode,FirstSeen,Channel,RSSI,CurrentLatitude,CurrentLongitude,AltitudeMeters,AccuracyMeters,Type");

    // Export networks
    for (const auto& net : _capturedNetworks) {
        file.printf("%s,%s,%s,%llu,%d,%d,%.6f,%.6f,%.2f,0.0,WIFI\n",
                   net.bssid.c_str(),
                   net.ssid.c_str(),
                   net.encrypted ? "WPA2" : "Open",
                   net.timestamp,
                   net.channel,
                   net.rssi,
                   net.latitude,
                   net.longitude,
                   0.0);  // Altitude
    }

    file.close();
    Serial.printf("[GPS] Exported %zu networks to Wigle format: %s\n",
                 _capturedNetworks.size(), filename.c_str());
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error GPSModule::setSerialPort(uint8_t rxPin, uint8_t txPin, uint32_t baud) {
    _rxPin = rxPin;
    _txPin = txPin;
    _baud = baud;
    
    if (_initialized) {
        shutdown();
        return initialize();
    }
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error GPSModule::parseGPSData() {
    // TODO: Implement GPS parsing with TinyGPS++
    // This would read from serial and parse NMEA sentences
    
    // For now, simulate
    _lastPosition.valid = false;
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

void GPSModule::scanAndStoreNetworks() {
    if (!g_wifiModule || !_wardriving) {
        return;
    }

    // Get current position
    GPSPosition position;
    getPosition(position);
    
    if (!position.valid) {
        return;  // No GPS fix
    }

    // Scan WiFi networks
    std::vector<WiFiModule::AccessPoint> aps;
    if (g_wifiModule->scanNetworks(aps).isError()) {
        return;
    }

    // Store networks with GPS coordinates
    for (const auto& ap : aps) {
        WiFiNetwork net;
        net.ssid = ap.ssid;
        net.bssid = ap.bssid;
        net.rssi = ap.rssi;
        net.channel = ap.channel;
        net.encrypted = ap.encrypted;
        net.latitude = position.latitude;
        net.longitude = position.longitude;
        net.timestamp = millis();
        
        _capturedNetworks.push_back(net);
    }
}

} // namespace Modules
} // namespace NightStrike

