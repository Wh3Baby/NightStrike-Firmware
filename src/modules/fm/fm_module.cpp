#include "modules/fm_module.h"
#include "core/errors.h"
#include <Arduino.h>
#include <functional>
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace NightStrike {
namespace Modules {

FMModule::FMModule() {
}

Core::Error FMModule::initialize() {
    if (_initialized) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }

    // Check if Si4713 is available
    if (!detectSi4713()) {
        Serial.println("[FM] Si4713 not detected - FM module not available");
        _initialized = false;
        return Core::Error(Core::ErrorCode::OPERATION_FAILED, "Si4713 not found");
    }

    Serial.println("[FM] Module initialized");
    _initialized = true;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error FMModule::shutdown() {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    stopBroadcast();
    stopSpectrumAnalyzer();

    _initialized = false;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

bool FMModule::isSupported() const {
    // FM requires Si4713 chip
    return detectSi4713();
}

Core::Error FMModule::begin() {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (!detectSi4713()) {
        return Core::Error(Core::ErrorCode::OPERATION_FAILED, "Si4713 not detected");
    }

    // Initialize I2C for Si4713 (address 0x63)
    Wire.begin();
    
    // Check if Si4713 responds
    Wire.beginTransmission(0x63);
    if (Wire.endTransmission() != 0) {
        return Core::Error(Core::ErrorCode::OPERATION_FAILED, "Si4713 I2C communication failed");
    }

    Serial.println("[FM] Si4713 initialized (full implementation requires Adafruit_Si4713 library)");
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error FMModule::setFrequency(uint16_t frequency) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (frequency < 7600 || frequency > 10800) {
        return Core::Error(Core::ErrorCode::INVALID_PARAMETER, "Frequency out of range (76.0-108.0 MHz)");
    }

    _currentFrequency = frequency;
    
    // Set frequency on Si4713
    // Frequency in 10kHz units, convert to kHz for Si4713
    uint16_t freqKhz = frequency * 10;  // Convert to kHz
    
    // Si4713 requires frequency in kHz (87500-108000)
    if (freqKhz < 87500 || freqKhz > 108000) {
        return Core::Error(Core::ErrorCode::INVALID_PARAMETER, "Frequency out of Si4713 range");
    }
    
    // TODO: Use Adafruit_Si4713 library: radio.tuneFM(frequency)
    Serial.printf("[FM] Frequency set to %d.%02d MHz (%d kHz)\n", 
                   frequency / 100, frequency % 100, freqKhz);
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error FMModule::startBroadcast(BroadcastType type) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (_broadcasting) {
        stopBroadcast();
    }

    // Implement broadcast with Si4713
    if (!begin().isSuccess()) {
        return Core::Error(Core::ErrorCode::OPERATION_FAILED, "FM initialization failed");
    }

    // Set TX power (88-115 dBuV, default 115)
    // TODO: Use Adafruit_Si4713: radio.setTXpower(115);
    
    switch (type) {
        case BroadcastType::STANDARD:
            // Standard FM band (87.5-108.0 MHz)
            if (_currentFrequency < 8750 || _currentFrequency > 10800) {
                _currentFrequency = 10230;  // Default to 102.30 MHz
            }
            Serial.printf("[FM] Starting standard broadcast on %d.%02d MHz\n", 
                         _currentFrequency / 100, _currentFrequency % 100);
            // TODO: radio.tuneFM(_currentFrequency);
            // TODO: radio.beginRDS();
            // TODO: radio.setRDSstation("NightStrike");
            break;
            
        case BroadcastType::RESERVED:
            // Reserved band (76.0-87.5 MHz)
            if (_currentFrequency < 7600 || _currentFrequency >= 8750) {
                _currentFrequency = 8000;  // Default to 80.00 MHz
            }
            Serial.printf("[FM] Starting reserved band broadcast on %d.%02d MHz\n", 
                         _currentFrequency / 100, _currentFrequency % 100);
            // TODO: radio.tuneFM(_currentFrequency);
            break;
            
        case BroadcastType::STOP:
            return stopBroadcast();
    }

    _broadcasting = true;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error FMModule::stopBroadcast() {
    if (!_broadcasting) {
        return Core::Error(Core::ErrorCode::SUCCESS);
    }

    // Stop broadcast on Si4713
    // TODO: Use Adafruit_Si4713: radio.setProperty(SI4713_PROP_TX_LINE_INPUT_LEVEL, 0);
    // TODO: radio.setProperty(SI4713_PROP_TX_PREEMPHASIS, 0);
    
    Serial.println("[FM] Broadcast stopped");
    _broadcasting = false;
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error FMModule::scanFrequency(uint16_t startFreq, uint16_t endFreq, uint16_t& bestFreq) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (startFreq < 7600 || endFreq > 10800 || startFreq >= endFreq) {
        return Core::Error(Core::ErrorCode::INVALID_PARAMETER, "Invalid frequency range");
    }

    // Implement frequency scan with Si4713
    Serial.printf("[FM] Scanning %d.%02d - %d.%02d MHz\n", 
                  startFreq / 100, startFreq % 100,
                  endFreq / 100, endFreq % 100);
    
    uint16_t minNoise = 0xFFFF;
    bestFreq = startFreq;
    
    // Scan frequencies in 10kHz steps
    for (uint16_t f = startFreq; f <= endFreq; f += 10) {
        // TODO: Use Adafruit_Si4713: radio.readTuneMeasure(f);
        // TODO: radio.readTuneStatus();
        // TODO: uint16_t noise = radio.currNoiseLevel;
        
        // Simulate noise measurement (replace with actual Si4713 call)
        uint16_t noise = random(1000, 2000);  // Placeholder
        
        if (noise < minNoise) {
            minNoise = noise;
            bestFreq = f;
        }
        
        delay(50);  // Small delay for measurement
    }
    
    Serial.printf("[FM] Best frequency: %d.%02d MHz (noise: %d)\n", 
                  bestFreq / 100, bestFreq % 100, minNoise);
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error FMModule::startSpectrumAnalyzer(std::function<void(uint16_t, int16_t)> callback) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (_spectrumRunning) {
        stopSpectrumAnalyzer();
    }

    _spectrumCallback = callback;
    _spectrumRunning = true;
    
    Serial.println("[FM] Spectrum analyzer started");
    
    // Start spectrum analysis task
    xTaskCreate([](void* param) {
        FMModule* fm = static_cast<FMModule*>(param);
        uint16_t startFreq = 7600;
        uint16_t endFreq = 10800;
        
        while (fm->_spectrumRunning) {
            for (uint16_t f = startFreq; f <= endFreq && fm->_spectrumRunning; f += 10) {
                // TODO: Use Adafruit_Si4713: radio.readTuneMeasure(f);
                // TODO: radio.readTuneStatus();
                // TODO: int16_t rssi = radio.currRSSI;
                
                // Simulate RSSI measurement
                int16_t rssi = random(-100, -50);  // Placeholder
                
                if (fm->_spectrumCallback) {
                    fm->_spectrumCallback(f, rssi);
                }
                
                delay(10);
            }
            delay(100);
        }
        vTaskDelete(NULL);
    }, "FM_Spectrum", 4096, this, 1, NULL);
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error FMModule::stopSpectrumAnalyzer() {
    if (!_spectrumRunning) {
        return Core::Error(Core::ErrorCode::SUCCESS);
    }

    _spectrumRunning = false;
    _spectrumCallback = nullptr;
    
    Serial.println("[FM] Spectrum analyzer stopped");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

bool FMModule::detectSi4713() const {
    // Detect Si4713 via I2C at address 0x63
    Wire.beginTransmission(0x63);
    uint8_t error = Wire.endTransmission();
    
    if (error == 0) {
        Serial.println("[FM] Si4713 detected at I2C 0x63");
        return true;
    }
    
    // Try alternative address 0x11 (if CS is low)
    Wire.beginTransmission(0x11);
    error = Wire.endTransmission();
    
    if (error == 0) {
        Serial.println("[FM] Si4713 detected at I2C 0x11");
        return true;
    }
    
    return false;
}

} // namespace Modules
} // namespace NightStrike

