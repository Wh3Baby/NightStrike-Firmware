#include "modules/ble_module.h"

// Include NimBLE headers after our namespace to avoid conflicts
#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>
#include <NimBLEScan.h>

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

    // TODO: Implement BLE HID keyboard
    Serial.println("[BLE] Keyboard injection not yet fully implemented");
    _keyboardActive = true;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error BLEModule::sendKeys(const std::string& text) {
    if (!_keyboardActive) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    // TODO: Send HID key codes
    Serial.printf("[BLE] Would send keys: %s\n", text.c_str());
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error BLEModule::stopKeyboard() {
    _keyboardActive = false;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

} // namespace Modules
} // namespace NightStrike

