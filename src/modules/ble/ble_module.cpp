#include "modules/ble_module.h"

// Include NimBLE headers after our namespace to avoid conflicts
#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>
#include <NimBLEScan.h>
#include <NimBLECharacteristic.h>
#include <NimBLEAdvertising.h>

namespace NightStrike {
namespace Modules {

BLEModule::BLEModule() {
}

Core::Error BLEModule::initialize() {
    if (_initialized) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }

    if (!_initialized) {
        ::NimBLEDevice::init("");
    }

    Serial.println("[BLE] Module initialized");
    _initialized = true;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error BLEModule::shutdown() {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    stopScan();
    stopKeyboard();
    ::NimBLEDevice::deinit(true);

    _initialized = false;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error BLEModule::scanDevices(std::vector<BLEDeviceInfo>& devices, uint32_t duration) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (_scanning) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }

    devices.clear();

    ::NimBLEScan* pBLEScan = ::NimBLEDevice::getScan();
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);

    // Start scan - returns bool, not pointer
    bool scanStarted = pBLEScan->start(duration / 1000, false);
    if (!scanStarted) {
        return Core::Error(Core::ErrorCode::OPERATION_FAILED);
    }

    // Get results after scan - getResults() returns temporary, need to copy or use directly
    ::NimBLEScanResults results = pBLEScan->getResults();

    for (int i = 0; i < results.getCount(); ++i) {
        const ::NimBLEAdvertisedDevice* device = results.getDevice(i);
        if (!device) continue;

        BLEDeviceInfo dev;
        dev.address = device->getAddress().toString().c_str();
        dev.name = device->getName().c_str();
        dev.rssi = device->getRSSI();
        dev.connectable = device->isConnectable();

        devices.push_back(dev);
    }

    pBLEScan->clearResults();
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error BLEModule::stopScan() {
    if (!_scanning) {
        return Core::Error(Core::ErrorCode::SUCCESS);
    }

    NimBLEScan* pBLEScan = ::NimBLEDevice::getScan();
    pBLEScan->stop();
    _scanning = false;

    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error BLEModule::spamIOS(const std::string& name) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    // Create BLE server with iOS-compatible services
    NimBLEServer* pServer = ::NimBLEDevice::createServer();

    // AirPods service
    NimBLEService* pService = pServer->createService("0000FE95-0000-1000-8000-00805F9B34FB");
    pService->start();

    // Start advertising
    NimBLEAdvertising* pAdvertising = ::NimBLEDevice::getAdvertising();
    pAdvertising->setName(name.c_str());
    // Note: setScanResponse and setMinPreferred are deprecated in newer NimBLE versions
    // Using setScanResponseData instead if needed
    pAdvertising->start();

    Serial.printf("[BLE] iOS spam started: %s\n", name.c_str());
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error BLEModule::spamAndroid(const std::string& name) {
    // Similar to iOS but with Android-specific UUIDs
    return spamIOS(name);  // Simplified for now
}

Core::Error BLEModule::spamWindows(const std::string& name) {
    // Windows-specific BLE spam
    return spamIOS(name);  // Simplified for now
}

Core::Error BLEModule::spamSamsung(const std::string& name) {
    // Samsung-specific BLE spam
    return spamIOS(name);  // Simplified for now
}

Core::Error BLEModule::spamAll(const std::vector<std::string>& names) {
    for (const auto& name : names) {
        spamIOS(name);
        delay(100);
    }
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error BLEModule::startKeyboard(const std::string& deviceName) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (_keyboardActive) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }

    // HID Report Map (Keyboard)
    static const uint8_t hidReportMap[] = {
        0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
        0x09, 0x06,        // Usage (Keyboard)
        0xA1, 0x01,        // Collection (Application)
        0x85, 0x01,        //   Report ID (1)
        0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
        0x19, 0xE0,        //   Usage Minimum (0xE0)
        0x29, 0xE7,        //   Usage Maximum (0xE7)
        0x15, 0x00,        //   Logical Minimum (0)
        0x25, 0x01,        //   Logical Maximum (1)
        0x75, 0x01,        //   Report Size (1)
        0x95, 0x08,        //   Report Count (8)
        0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x95, 0x01,        //   Report Count (1)
        0x75, 0x08,        //   Report Size (8)
        0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x95, 0x05,        //   Report Count (5)
        0x75, 0x01,        //   Report Size (1)
        0x05, 0x08,        //   Usage Page (LEDs)
        0x19, 0x01,        //   Usage Minimum (0x01)
        0x29, 0x05,        //   Usage Maximum (0x05)
        0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x95, 0x01,        //   Report Count (1)
        0x75, 0x03,        //   Report Size (3)
        0x91, 0x01,        //   Output (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
        0x95, 0x06,        //   Report Count (6)
        0x75, 0x08,        //   Report Size (8)
        0x15, 0x00,        //   Logical Minimum (0)
        0x25, 0x65,        //   Logical Maximum (101)
        0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
        0x19, 0x00,        //   Usage Minimum (0x00)
        0x29, 0x65,        //   Usage Maximum (0x65)
        0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0xC0,              // End Collection
    };

    // Create BLE HID Server
    ::NimBLEServer* pServer = ::NimBLEDevice::createServer();
    _hidServer = pServer;

    // HID Service UUID: 0x1812
    ::NimBLEService* pService = pServer->createService("1812");
    _hidService = pService;

    // HID Report Map Characteristic
    ::NimBLECharacteristic* pReportMapChar = pService->createCharacteristic("2A4B", NIMBLE_PROPERTY::READ);
    pReportMapChar->setValue((uint8_t*)hidReportMap, sizeof(hidReportMap));
    _reportMapChar = pReportMapChar;

    // HID Information Characteristic
    ::NimBLECharacteristic* pInfoChar = pService->createCharacteristic("2A4A", NIMBLE_PROPERTY::READ);
    uint8_t info[] = {0x01, 0x01, 0x00, 0x03}; // Version 1.1, Country Code 0, Flags 0x03
    pInfoChar->setValue(info, sizeof(info));

    // HID Control Point Characteristic
    ::NimBLECharacteristic* pControlChar = pService->createCharacteristic("2A4C", NIMBLE_PROPERTY::WRITE_NR);
    _controlChar = pControlChar;

    // HID Input Report Characteristic (for keyboard)
    ::NimBLECharacteristic* pInputChar = pService->createCharacteristic("2A4D", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    _inputChar = pInputChar;

    // HID Output Report Characteristic (for LEDs)
    ::NimBLECharacteristic* pOutputChar = pService->createCharacteristic("2A4D", NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
    _outputChar = pOutputChar;

    pService->start();

    // Start advertising as HID keyboard
    ::NimBLEAdvertising* pAdvertising = ::NimBLEDevice::getAdvertising();
    pAdvertising->setAppearance(0x03C1); // Keyboard appearance
    pAdvertising->setName(deviceName.empty() ? "NightStrike Keyboard" : deviceName.c_str());
    pAdvertising->addServiceUUID("1812"); // HID Service
    pAdvertising->start();

    _keyboardActive = true;
    Serial.printf("[BLE] HID Keyboard started: %s\n", deviceName.empty() ? "NightStrike Keyboard" : deviceName.c_str());
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error BLEModule::sendKeys(const std::string& text) {
    if (!_keyboardActive || !_inputChar) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    ::NimBLECharacteristic* pInputChar = static_cast<::NimBLECharacteristic*>(_inputChar);
    
    // Send each character as HID key code
    for (char c : text) {
        uint8_t keyCode = 0;
        uint8_t modifiers = 0;
        
        // Convert character to HID key code
        if (c >= 'a' && c <= 'z') {
            keyCode = 0x04 + (c - 'a');
        } else if (c >= 'A' && c <= 'Z') {
            keyCode = 0x04 + (c - 'A');
            modifiers = 0x02; // Left Shift
        } else if (c >= '1' && c <= '9') {
            keyCode = 0x1E + (c - '1');
        } else if (c == '0') {
            keyCode = 0x27;
        } else {
            // Special characters
            switch (c) {
                case ' ': keyCode = 0x2C; break; // Space
                case '\n': case '\r': keyCode = 0x28; break; // Enter
                case '\t': keyCode = 0x2B; break; // Tab
                case '\b': keyCode = 0x2A; break; // Backspace
                case '!': keyCode = 0x1E; modifiers = 0x02; break;
                case '@': keyCode = 0x1F; modifiers = 0x02; break;
                case '#': keyCode = 0x20; modifiers = 0x02; break;
                case '$': keyCode = 0x21; modifiers = 0x02; break;
                case '%': keyCode = 0x22; modifiers = 0x02; break;
                case '^': keyCode = 0x23; modifiers = 0x02; break;
                case '&': keyCode = 0x24; modifiers = 0x02; break;
                case '*': keyCode = 0x25; modifiers = 0x02; break;
                case '(': keyCode = 0x26; modifiers = 0x02; break;
                case ')': keyCode = 0x27; modifiers = 0x02; break;
                default: continue; // Skip unknown characters
            }
        }
        
        // Send key press
        uint8_t report[] = {0x01, modifiers, 0x00, keyCode, 0x00, 0x00, 0x00, 0x00, 0x00};
        pInputChar->setValue(report, sizeof(report));
        pInputChar->notify();
        delay(50);
        
        // Send key release
        uint8_t release[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        pInputChar->setValue(release, sizeof(release));
        pInputChar->notify();
        delay(50);
    }
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error BLEModule::sendRawHID(uint8_t key, uint8_t modifiers) {
    if (!_keyboardActive || !_inputChar) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    ::NimBLECharacteristic* pInputChar = static_cast<::NimBLECharacteristic*>(_inputChar);
    
    // Send key press
    uint8_t report[] = {0x01, modifiers, 0x00, key, 0x00, 0x00, 0x00, 0x00, 0x00};
    pInputChar->setValue(report, sizeof(report));
    pInputChar->notify();
    delay(50);
    
    // Send key release
    uint8_t release[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    pInputChar->setValue(release, sizeof(release));
    pInputChar->notify();
    delay(50);
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error BLEModule::stopKeyboard() {
    if (!_keyboardActive) {
        return Core::Error(Core::ErrorCode::SUCCESS);
    }

    // Stop advertising
    ::NimBLEAdvertising* pAdvertising = ::NimBLEDevice::getAdvertising();
    pAdvertising->stop();

    // Clean up
    _hidServer = nullptr;
    _hidService = nullptr;
    _inputChar = nullptr;
    _outputChar = nullptr;
    _controlChar = nullptr;
    _reportMapChar = nullptr;

    _keyboardActive = false;
    Serial.println("[BLE] HID Keyboard stopped");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

} // namespace Modules
} // namespace NightStrike

