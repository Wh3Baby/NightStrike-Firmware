#include "modules/rf_module.h"
#include "modules/rf/protocols.h"
#include "modules/rf/rf_driver_interface.h"
#include "core/storage.h"
#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <map>
#include <memory>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Условная компиляция RF драйверов для экономии памяти
#ifdef ENABLE_RF_CC1101
#include "modules/rf/cc1101_driver.h"
#endif
#ifdef ENABLE_RF_NRF24L01
#include "modules/rf/nrf24l01_driver.h"
#endif

namespace NightStrike {
namespace Modules {

// RF Jammer task function
void rfJammerTask(void* param) {
    RFModule* module = static_cast<RFModule*>(param);
    IRFDriver* driver = static_cast<IRFDriver*>(module->_rfDriver);
    
    // Generate random noise data for jamming
    uint8_t noise[32];
    for (int i = 0; i < 32; ++i) {
        noise[i] = random(0, 256);
    }
    
    while (module->_jamming) {
        // Transmit noise continuously
        driver->transmit(noise, sizeof(noise));
        
        if (module->_intermittent) {
            vTaskDelay(random(10, 100) / portTICK_PERIOD_MS);  // Random delay for intermittent jamming
        } else {
            vTaskDelay(1 / portTICK_PERIOD_MS);  // Continuous jamming
        }
    }
    
    vTaskDelete(NULL);
}

RFModule::RFModule() {
    _rfDriver = nullptr;
    _rfModuleType = RFModuleType::NONE;
}

RFModule::~RFModule() {
    if (_rfDriver) {
        delete static_cast<IRFDriver*>(_rfDriver);
        _rfDriver = nullptr;
    }
}

Core::Error RFModule::initialize() {
    if (_initialized) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }

    // Initialize RF module if configured
    if (_rfModuleEnabled && _rfCSPin > 0 && _rfModuleType != RFModuleType::NONE) {
        if (!_rfDriver) {
            // Create appropriate driver based on type (only if enabled via build flags)
            switch (_rfModuleType) {
#ifdef ENABLE_RF_CC1101
                case RFModuleType::CC1101:
                    _rfDriver = new CC1101Driver(_rfCSPin, _rfPin1, _rfPin2);
                    break;
#endif
#ifdef ENABLE_RF_NRF24L01
                case RFModuleType::NRF24L01:
                    _rfDriver = new NRF24L01Driver(_rfPin1, _rfCSPin);  // CE, CSN
                    break;
#endif
                default:
                    Serial.println("[RF] Module type not compiled in (check build flags)");
                    break;
            }
        }
        
        if (_rfDriver) {
            IRFDriver* driver = static_cast<IRFDriver*>(_rfDriver);
            if (driver->begin()) {
                Serial.printf("[RF] %s initialized successfully\n", driver->getModuleName());
                // Set default frequency
                setFrequency(_currentFreq);
            } else {
                Serial.printf("[RF] %s initialization failed\n", driver->getModuleName());
            }
        }
    } else {
        Serial.println("[RF] Module initialized (no RF hardware configured)");
    }

    _initialized = true;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error RFModule::shutdown() {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    stopJammer();
    stopSpectrumAnalyzer();

    if (_rfDriver) {
        IRFDriver* driver = static_cast<IRFDriver*>(_rfDriver);
        driver->end();
        delete driver;
        _rfDriver = nullptr;
    }

    _initialized = false;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

bool RFModule::isSupported() const {
    // TODO: Check if RF hardware is present
    return true;  // Assume supported for now
}

Core::Error RFModule::setFrequency(Frequency freq) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    _currentFreq = freq;
    
    if (_rfDriver) {
        IRFDriver* driver = static_cast<IRFDriver*>(_rfDriver);
        if (driver->setFrequency(static_cast<uint32_t>(freq))) {
            Serial.printf("[RF] %s frequency set to %u Hz\n", driver->getModuleName(), static_cast<uint32_t>(freq));
        } else {
            return Core::Error(Core::ErrorCode::OPERATION_FAILED, "Failed to set RF frequency");
        }
    } else {
        Serial.printf("[RF] Frequency set to %u Hz (no hardware)\n", static_cast<uint32_t>(freq));
    }
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error RFModule::setTXPin(uint8_t pin) {
    _txPin = pin;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error RFModule::setRXPin(uint8_t pin) {
    _rxPin = pin;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error RFModule::transmit(const RFCode& code) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (_rfDriver) {
        IRFDriver* driver = static_cast<IRFDriver*>(_rfDriver);
        // Set frequency if different
        if (code.frequency != static_cast<uint32_t>(_currentFreq)) {
            driver->setFrequency(code.frequency);
        }
        
        // Transmit via RF module
        if (driver->transmit(code.data.data(), code.data.size())) {
            Serial.printf("[RF] %s transmitted: %s (%zu bytes)\n",
                         driver->getModuleName(), code.name.c_str(), code.data.size());
            return Core::Error(Core::ErrorCode::SUCCESS);
        } else {
            return Core::Error(Core::ErrorCode::OPERATION_FAILED, "RF transmit failed");
        }
    } else {
        Serial.printf("[RF] Transmitting code: %s (%zu bytes) (no hardware)\n",
                     code.name.c_str(), code.data.size());
        return Core::Error(Core::ErrorCode::SUCCESS);
    }
}

Core::Error RFModule::receive(RFCode& code, uint32_t timeout) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (_rfDriver) {
        IRFDriver* driver = static_cast<IRFDriver*>(_rfDriver);
        uint8_t buffer[255];
        size_t len = driver->receive(buffer, sizeof(buffer), timeout);
        
        if (len > 0) {
            code.data.assign(buffer, buffer + len);
            code.frequency = static_cast<uint32_t>(_currentFreq);
            code.name = "Received";
            Serial.printf("[RF] %s received: %zu bytes\n", driver->getModuleName(), len);
            return Core::Error(Core::ErrorCode::SUCCESS);
        } else {
            return Core::Error(Core::ErrorCode::OPERATION_FAILED, "No data received");
        }
    } else {
        Serial.println("[RF] Receiving code... (no hardware)");
        return Core::Error(Core::ErrorCode::SUCCESS);
    }
}

Core::Error RFModule::startJammer(bool intermittent) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (_jamming) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }

    if (!_rfDriver) {
        return Core::Error(Core::ErrorCode::NOT_SUPPORTED, "No RF hardware");
    }

    _jamming = true;
    _intermittent = intermittent;
    
    // Start jamming task
    xTaskCreatePinnedToCore(
        rfJammerTask,
        "RFJammer",
        4096,
        this,
        1,
        &_jammerTaskHandle,
        1
    );
    
    Serial.printf("[RF] Jammer started (intermittent: %s)\n",
                  intermittent ? "yes" : "no");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error RFModule::stopJammer() {
    if (!_jamming) {
        return Core::Error(Core::ErrorCode::SUCCESS);
    }

    _jamming = false;
    
    // Wait for task to finish
    if (_jammerTaskHandle) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        if (_jammerTaskHandle) {
            vTaskDelete(_jammerTaskHandle);
            _jammerTaskHandle = nullptr;
        }
    }
    
    Serial.println("[RF] Jammer stopped");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error RFModule::startSpectrumAnalyzer(std::function<void(uint32_t, int8_t)> callback) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (_spectrumActive) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }

    _spectrumActive = true;
    _spectrumCallback = callback;
    
    Serial.println("[RF] Spectrum analyzer started");
    Serial.println("[RF] Scanning frequencies...");
    
    // Spectrum analysis would scan frequencies and call callback
    // For now, simulate scanning
    if (callback) {
        // Scan common Sub-GHz frequencies
        uint32_t frequencies[] = {
            433920000, 434000000, 434080000, 434160000, 434240000,
            868350000, 868400000, 868450000, 868500000,
            915000000, 915100000, 915200000
        };
        
        for (uint32_t freq : frequencies) {
            // Simulate RSSI reading (-100 to -30 dBm)
            int8_t rssi = -100 + (random(0, 70));
            callback(freq, rssi);
            delay(10);
        }
    }
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error RFModule::stopSpectrumAnalyzer() {
    if (!_spectrumActive) {
        return Core::Error(Core::ErrorCode::SUCCESS);
    }

    _spectrumActive = false;
    Serial.println("[RF] Spectrum analyzer stopped");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error RFModule::saveCode(const RFCode& code, const std::string& name) {
    if (!LittleFS.begin(true)) {
        return Core::Error(Core::ErrorCode::STORAGE_NOT_MOUNTED);
    }

    std::string filename = "/rf_codes/" + name + ".json";
    
    // Create directory if needed
    if (!LittleFS.exists("/rf_codes")) {
        // LittleFS doesn't support mkdir, files are created with full path
    }

    File file = LittleFS.open(filename.c_str(), "w");
    if (!file) {
        return Core::Error(Core::ErrorCode::FILE_WRITE_ERROR);
    }

    // Create JSON document
    StaticJsonDocument<2048> doc;
    doc["name"] = code.name;
    doc["frequency"] = code.frequency;
    doc["protocol"] = code.protocol;
    
    JsonArray dataArray = doc.createNestedArray("data");
    for (uint8_t byte : code.data) {
        dataArray.add(byte);
    }

    serializeJson(doc, file);
    file.close();

    Serial.printf("[RF] Code saved: %s\n", name.c_str());
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error RFModule::loadCode(const std::string& name, RFCode& code) {
    if (!LittleFS.begin(true)) {
        return Core::Error(Core::ErrorCode::STORAGE_NOT_MOUNTED);
    }

    std::string filename = "/rf_codes/" + name + ".json";
    File file = LittleFS.open(filename.c_str(), "r");
    if (!file) {
        return Core::Error(Core::ErrorCode::FILE_NOT_FOUND);
    }

    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        return Core::Error(Core::ErrorCode::FILE_READ_ERROR, error.c_str());
    }

    code.name = doc["name"].as<std::string>();
    code.frequency = doc["frequency"].as<uint32_t>();
    code.protocol = doc["protocol"].as<uint32_t>();
    
    code.data.clear();
    JsonArray dataArray = doc["data"];
    for (JsonVariant value : dataArray) {
        code.data.push_back(value.as<uint8_t>());
    }

    Serial.printf("[RF] Code loaded: %s (%zu bytes)\n", name.c_str(), code.data.size());
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error RFModule::listCodes(std::vector<std::string>& names) {
    names.clear();

    if (!LittleFS.begin(true)) {
        return Core::Error(Core::ErrorCode::STORAGE_NOT_MOUNTED);
    }

    File root = LittleFS.open("/rf_codes");
    if (!root) {
        // Directory doesn't exist, return empty list
        return Core::Error(Core::ErrorCode::SUCCESS);
    }

    File file = root.openNextFile();
    while (file) {
        std::string filename = file.name();
        if (filename.find(".json") != std::string::npos) {
            // Extract name without path and extension
            size_t slashPos = filename.find_last_of('/');
            size_t dotPos = filename.find_last_of('.');
            if (slashPos != std::string::npos && dotPos != std::string::npos) {
                std::string name = filename.substr(slashPos + 1, dotPos - slashPos - 1);
                names.push_back(name);
            }
        }
        file = root.openNextFile();
    }

    Serial.printf("[RF] Found %zu saved codes\n", names.size());
    return Core::Error(Core::ErrorCode::SUCCESS);
}

// Protocol registry
static std::map<std::string, RFProtocol*> g_protocols;
static RFProtocol* g_currentProtocol = nullptr;

static void initProtocols() {
    if (!g_protocols.empty()) return;
    
    g_protocols["Came"] = new CameProtocol();
    g_protocols["Linear"] = new LinearProtocol();
    g_protocols["Holtek"] = new HoltekProtocol();
    g_protocols["NiceFlo"] = new NiceFloProtocol();
    g_protocols["Chamberlain"] = new ChamberlainProtocol();
    g_protocols["Liftmaster"] = new LiftmasterProtocol();
    g_protocols["Ansonic"] = new AnsonicProtocol();
}

Core::Error RFModule::setProtocol(const std::string& protocolName) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    initProtocols();
    
    auto it = g_protocols.find(protocolName);
    if (it == g_protocols.end()) {
        return Core::Error(Core::ErrorCode::INVALID_PARAMETER, "Unknown protocol");
    }

    g_currentProtocol = it->second;
    Serial.printf("[RF] Protocol set to: %s\n", protocolName.c_str());
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error RFModule::listProtocols(std::vector<std::string>& protocols) {
    initProtocols();
    
    protocols.clear();
    for (const auto& pair : g_protocols) {
        protocols.push_back(pair.first);
    }
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error RFModule::transmitWithProtocol(const std::vector<uint8_t>& data, const std::string& protocol) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    initProtocols();
    
    auto it = g_protocols.find(protocol);
    if (it == g_protocols.end()) {
        return Core::Error(Core::ErrorCode::INVALID_PARAMETER, "Unknown protocol");
    }

    RFProtocol* proto = it->second;
    std::vector<int> timings = proto->encode(data);
    
    // TODO: Transmit timings via RF module
    Serial.printf("[RF] Transmitting %zu bytes with protocol %s (%zu timings)\n",
                 data.size(), protocol.c_str(), timings.size());
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error RFModule::receiveWithProtocol(std::vector<uint8_t>& data, const std::string& protocol, uint32_t timeout) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    initProtocols();
    
    auto it = g_protocols.find(protocol);
    if (it == g_protocols.end()) {
        return Core::Error(Core::ErrorCode::INVALID_PARAMETER, "Unknown protocol");
    }

    RFProtocol* proto = it->second;
    
    // TODO: Receive timings from RF module
    std::vector<int> timings;
    
    // Decode timings to data
    data = proto->decode(timings);
    
    Serial.printf("[RF] Received %zu bytes with protocol %s\n",
                 data.size(), protocol.c_str());
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error RFModule::setRFModule(RFModuleType type, uint8_t csPin, uint8_t pin1, uint8_t pin2) {
    _rfModuleType = type;
    _rfCSPin = csPin;
    _rfPin1 = pin1;
    _rfPin2 = pin2;
    
    if (_initialized && _rfModuleEnabled) {
        // Reinitialize with new module
        shutdown();
        return initialize();
    }
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error RFModule::enableRFModule(bool enable) {
    _rfModuleEnabled = enable;
    
    if (_initialized) {
        if (enable && _rfCSPin > 0 && _rfModuleType != RFModuleType::NONE) {
            if (!_rfDriver) {
                // Create appropriate driver (only if enabled via build flags)
                switch (_rfModuleType) {
#ifdef ENABLE_RF_CC1101
                    case RFModuleType::CC1101:
                        _rfDriver = new CC1101Driver(_rfCSPin, _rfPin1, _rfPin2);
                        break;
#endif
#ifdef ENABLE_RF_NRF24L01
                    case RFModuleType::NRF24L01:
                        _rfDriver = new NRF24L01Driver(_rfPin1, _rfCSPin);
                        break;
#endif
                    default:
                        Serial.println("[RF] Module type not compiled in");
                        break;
                }
            }
            
            if (_rfDriver) {
                IRFDriver* driver = static_cast<IRFDriver*>(_rfDriver);
                if (driver->begin()) {
                    setFrequency(_currentFreq);
                    Serial.printf("[RF] %s enabled\n", driver->getModuleName());
                }
            }
        } else if (!enable && _rfDriver) {
            IRFDriver* driver = static_cast<IRFDriver*>(_rfDriver);
            driver->end();
            delete driver;
            _rfDriver = nullptr;
            Serial.println("[RF] RF module disabled");
        }
    }
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error RFModule::detectRFModule(RFModuleType& detectedType) {
    detectedType = RFModuleType::NONE;
    
    if (_rfCSPin == 0) {
        return Core::Error(Core::ErrorCode::INVALID_PARAMETER, "CS pin not configured");
    }
    
    // Try to detect by reading chip ID/version registers
    // This is a simplified detection - real detection would read specific registers
    
    // Try CC1101 (only if compiled in)
#ifdef ENABLE_RF_CC1101
    CC1101Driver* cc1101 = new CC1101Driver(_rfCSPin, _rfPin1, _rfPin2);
    if (cc1101->begin()) {
        detectedType = RFModuleType::CC1101;
        cc1101->end();
        delete cc1101;
        Serial.println("[RF] Detected: CC1101");
        return Core::Error(Core::ErrorCode::SUCCESS);
    }
    delete cc1101;
#endif
    
    // Try NRF24L01 (only if compiled in)
#ifdef ENABLE_RF_NRF24L01
    NRF24L01Driver* nrf24 = new NRF24L01Driver(_rfPin1, _rfCSPin);
    if (nrf24->begin()) {
        detectedType = RFModuleType::NRF24L01;
        nrf24->end();
        delete nrf24;
        Serial.println("[RF] Detected: NRF24L01");
        return Core::Error(Core::ErrorCode::SUCCESS);
    }
    delete nrf24;
#endif
    
    
    Serial.println("[RF] No RF module detected");
    return Core::Error(Core::ErrorCode::OPERATION_FAILED, "No RF module detected");
}

Core::Error RFModule::setCC1101Pins(uint8_t csPin, uint8_t gdo0Pin, uint8_t gdo2Pin) {
    return setRFModule(RFModuleType::CC1101, csPin, gdo0Pin, gdo2Pin);
}

Core::Error RFModule::enableCC1101(bool enable) {
    return enableRFModule(enable);
}

} // namespace Modules
} // namespace NightStrike

