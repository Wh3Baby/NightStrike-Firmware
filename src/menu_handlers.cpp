/**
 * @file menu_handlers.cpp
 * @brief Menu handlers for all modules
 */

#include "core/menu.h"
#include "core/display.h"
#include "core/errors.h"
#include "modules/wifi_module.h"
#include "modules/ble_module.h"
#include "modules/rf_module.h"
#include "modules/rfid_module.h"
#include "modules/blackhat_tools.h"
#include "modules/physical_hack_module.h"
#include "modules/ir_module.h"
#include "modules/badusb_module.h"
#include "modules/gps_module.h"
#include "modules/fm_module.h"
#include "modules/espnow_module.h"
#include "modules/nrf24_module.h"
#include "modules/ethernet_module.h"
#include "modules/interpreter_module.h"
#include "modules/others_module.h"
#include "core/config.h"
#include <Arduino.h>

using namespace NightStrike::Core;
using namespace NightStrike::Modules;

// Global module instances (set in main.cpp)
extern WiFiModule* g_wifiModule;
extern BLEModule* g_bleModule;
extern RFModule* g_rfModule;
extern RFIDModule* g_rfidModule;
extern BlackHatToolsModule* g_blackhatTools;
extern PhysicalHackModule* g_physicalHackModule;
extern IRModule* g_irModule;
extern BadUSBModule* g_badusbModule;
extern GPSModule* g_gpsModule;
extern FMModule* g_fmModule;
extern ESPNOWModule* g_espnowModule;
extern NRF24Module* g_nrf24Module;
extern EthernetModule* g_ethernetModule;
extern InterpreterModule* g_interpreterModule;
extern OthersModule* g_othersModule;

// Forward declarations
void setupMainMenu();
void showPhysicalHackMenu();
void showWiFiMenu();
void showWiFiNetworkList();
void showWiFiNetworkActions(size_t networkIndex);
void showBLEMenu();
void showBLEDeviceList();
void showBLEDeviceActions(size_t deviceIndex);
void showBlackHatMenu();
void showBlackHatHostList();
void showBlackHatHostActions(size_t hostIndex);
void showPhysicalHackExploitList();
void showPhysicalHackExploitActions(size_t exploitIndex);
void showIRMenu();
void showBadUSBMenu();
void showGPSMenu();
void showFMMenu();
void showESPNOWMenu();
void showNRF24Menu();
void showEthernetMenu();
void showInterpreterMenu();
void showOthersMenu();

// Helper to show message on display
void showMessage(const char* msg, uint32_t duration = 2000) {
    auto& display = Display::getInstance();
    display.clear();
    display.setTextColor(Display::Color::Green(), Display::Color::Black());
    display.setTextSize(1);
    display.drawTextCentered(Display::Point(display.getSize().width / 2,
                                            display.getSize().height / 2),
                             msg);
    delay(duration);
    // Don't auto-show menu - caller should call menu.show() or setupMainMenu()
}

// Global storage for scanned networks/devices
static std::vector<WiFiModule::AccessPoint> g_scannedAPs;
static std::vector<BLEModule::BLEDeviceInfo> g_scannedBLEDevices;
static std::vector<std::string> g_scannedHosts;
static std::vector<PhysicalHackModule::ExploitPayload> g_availableExploits;

// Show WiFi network list menu
void showWiFiNetworkList() {
    auto& menu = Menu::getInstance();
    menu.clear();

    if (g_scannedAPs.empty()) {
        showMessage("No networks found");
        delay(2000);
        showWiFiMenu();
        return;
    }

    // Add each network to menu
    for (size_t i = 0; i < g_scannedAPs.size() && i < 15; ++i) {
        const auto& ap = g_scannedAPs[i];
        char label[64];
        // Format: "SSID (RSSI: -XX Ch:XX)"
        snprintf(label, sizeof(label), "%s (%ddBm Ch%d)", 
                 ap.ssid.empty() ? "(hidden)" : ap.ssid.c_str(), 
                 ap.rssi, ap.channel);
        
        menu.addItem(Menu::MenuItem(label, [i]() {
            // Show network actions menu
            showWiFiNetworkActions(i);
        }));
    }

    menu.addItem(Menu::MenuItem("Back", []() {
        showWiFiMenu();
    }));

    menu.show();
}

// Show actions for selected WiFi network
void showWiFiNetworkActions(size_t networkIndex) {
    if (networkIndex >= g_scannedAPs.size()) {
        showWiFiMenu();
        return;
    }

    const auto& ap = g_scannedAPs[networkIndex];
    auto& menu = Menu::getInstance();
    menu.clear();

    char title[64];
    snprintf(title, sizeof(title), "Network: %s", ap.ssid.empty() ? "(hidden)" : ap.ssid.c_str());
    
    menu.addItem(Menu::MenuItem("Info", [networkIndex]() {
        const auto& ap = g_scannedAPs[networkIndex];
        char info[128];
        snprintf(info, sizeof(info), "SSID: %s\nRSSI: %d dBm\nCh: %d\nEnc: %s",
                 ap.ssid.empty() ? "(hidden)" : ap.ssid.c_str(),
                 ap.rssi, ap.channel, ap.encrypted ? "Yes" : "No");
        showMessage(info, 3000);
        delay(3000);
        showWiFiNetworkActions(networkIndex);
    }));

    menu.addItem(Menu::MenuItem("Deauth Attack", [networkIndex]() {
        if (!g_wifiModule || !g_wifiModule->isInitialized()) {
            showMessage("WiFi not initialized");
            showWiFiNetworkActions(networkIndex);
            return;
        }

        const auto& ap = g_scannedAPs[networkIndex];
        showMessage("Starting Deauth...", 1000);
        
        // Use AccessPoint directly for deauthAttack
        auto err = g_wifiModule->deauthAttack(ap, 10);
        if (err.isError()) {
            showMessage("Deauth failed");
        } else {
            showMessage("Deauth active");
        }
        delay(2000);
        showWiFiNetworkActions(networkIndex);
    }));

    menu.addItem(Menu::MenuItem("Clone AP", [networkIndex]() {
        if (!g_wifiModule || !g_wifiModule->isInitialized()) {
            showMessage("WiFi not initialized");
            showWiFiNetworkActions(networkIndex);
            return;
        }

        const auto& ap = g_scannedAPs[networkIndex];
        showMessage("Cloning AP...", 1000);
        
        auto err = g_wifiModule->startAP(ap.ssid, "");
        if (err.isError()) {
            showMessage("Clone failed");
        } else {
            showMessage("AP cloned");
        }
        delay(2000);
        showWiFiNetworkActions(networkIndex);
    }));

    menu.addItem(Menu::MenuItem("Back", [networkIndex]() {
        showWiFiNetworkList();
    }));

    menu.show();
}

// BLE Device List Menu
void showBLEDeviceList() {
    auto& menu = Menu::getInstance();
    menu.clear();

    if (g_scannedBLEDevices.empty()) {
        showMessage("No devices found");
        delay(2000);
        showBLEMenu();
        return;
    }

    // Add each device to menu
    for (size_t i = 0; i < g_scannedBLEDevices.size() && i < 15; ++i) {
        const auto& dev = g_scannedBLEDevices[i];
        char label[64];
        // Format: "Name (RSSI: -XX)"
        snprintf(label, sizeof(label), "%s (%ddBm)", 
                 dev.name.empty() ? dev.address.c_str() : dev.name.c_str(), 
                 dev.rssi);
        
        menu.addItem(Menu::MenuItem(label, [i]() {
            showBLEDeviceActions(i);
        }));
    }

    menu.addItem(Menu::MenuItem("Back", []() {
        showBLEMenu();
    }));

    menu.show();
}

// Show actions for selected BLE device
void showBLEDeviceActions(size_t deviceIndex) {
    if (deviceIndex >= g_scannedBLEDevices.size()) {
        showBLEMenu();
        return;
    }

    const auto& dev = g_scannedBLEDevices[deviceIndex];
    auto& menu = Menu::getInstance();
    menu.clear();

    menu.addItem(Menu::MenuItem("Info", [deviceIndex]() {
        const auto& dev = g_scannedBLEDevices[deviceIndex];
        char info[128];
        snprintf(info, sizeof(info), "Name: %s\nAddr: %s\nRSSI: %d dBm\nConn: %s",
                 dev.name.empty() ? "(unknown)" : dev.name.c_str(),
                 dev.address.c_str(),
                 dev.rssi,
                 dev.connectable ? "Yes" : "No");
        showMessage(info, 3000);
        delay(3000);
        showBLEDeviceActions(deviceIndex);
    }));

    menu.addItem(Menu::MenuItem("Keyboard", [deviceIndex]() {
        if (!g_bleModule || !g_bleModule->isInitialized()) {
            showMessage("BLE not initialized");
            showBLEDeviceActions(deviceIndex);
            return;
        }

        const auto& dev = g_scannedBLEDevices[deviceIndex];
        showMessage("Connecting...", 1000);
        
        auto err = g_bleModule->startKeyboard(dev.name.empty() ? dev.address : dev.name);
        if (err.isError()) {
            showMessage("Keyboard failed");
        } else {
            showMessage("Keyboard active");
        }
        delay(2000);
        showBLEDeviceActions(deviceIndex);
    }));

    menu.addItem(Menu::MenuItem("Back", [deviceIndex]() {
        showBLEDeviceList();
    }));

    menu.show();
}

// WiFi Menu
void showWiFiMenu() {
    auto& menu = Menu::getInstance();
    menu.clear();

    // Add Initialize as first item
    menu.addItem(Menu::MenuItem("Initialize", []() {
        if (g_wifiModule && g_wifiModule->isInitialized()) {
            showMessage("WiFi already initialized");
            showWiFiMenu();
            return;
        }

        if (!g_wifiModule) {
            g_wifiModule = new WiFiModule();
        }

        auto err = g_wifiModule->initialize();
        if (err.isError()) {
            char msg[64];
            snprintf(msg, sizeof(msg), "Init failed: %s", getErrorMessage(err.code));
            showMessage(msg);
        } else {
            showMessage("WiFi initialized");
        }
        delay(2000);
        showWiFiMenu();
    }));

    menu.addItem(Menu::MenuItem("Scan Networks", []() {
        Serial.println("[WiFi] Scanning networks...");
        showMessage("Scanning...", 2000);
        
        if (!g_wifiModule || !g_wifiModule->isInitialized()) {
            showMessage("WiFi not initialized");
            showWiFiMenu();
            return;
        }

        g_scannedAPs.clear();
        auto err = g_wifiModule->scanNetworks(g_scannedAPs);
        
        if (err.isError()) {
            Serial.printf("[WiFi] Scan failed: %s\n", getErrorMessage(err.code));
            showMessage("Scan failed");
            delay(2000);
            showWiFiMenu();
            return;
        }

        Serial.printf("[WiFi] Found %zu networks\n", g_scannedAPs.size());
        for (size_t i = 0; i < g_scannedAPs.size() && i < 10; ++i) {
            Serial.printf("  %zu. %s (RSSI: %d, Ch: %d)\n", 
                i+1, g_scannedAPs[i].ssid.c_str(), g_scannedAPs[i].rssi, g_scannedAPs[i].channel);
        }
        
        // Show network list menu
        if (g_scannedAPs.empty()) {
            showMessage("No networks found");
            delay(2000);
            showWiFiMenu();
        } else {
            showWiFiNetworkList();
        }
    }));

    menu.addItem(Menu::MenuItem("Start AP", []() {
        if (!g_wifiModule || !g_wifiModule->isInitialized()) {
            showMessage("WiFi not initialized");
            showWiFiMenu();
            return;
        }

        auto err = g_wifiModule->startAP("NightStrike-AP", "");
        if (err.isError()) {
            showMessage("AP start failed");
        } else {
            showMessage("AP started: NightStrike-AP");
        }
        showWiFiMenu();
    }));

    menu.addItem(Menu::MenuItem("Evil Portal", []() {
        if (!g_wifiModule || !g_wifiModule->isInitialized()) {
            showMessage("WiFi not initialized");
            showWiFiMenu();
            return;
        }

        auto err = g_wifiModule->startEvilPortal("FreeWiFi");
        if (err.isError()) {
            showMessage("Evil Portal failed");
        } else {
            showMessage("Evil Portal: FreeWiFi");
        }
        showWiFiMenu();
    }));

    menu.addItem(Menu::MenuItem("Beacon Spam", []() {
        if (!g_wifiModule || !g_wifiModule->isInitialized()) {
            showMessage("WiFi not initialized");
            showWiFiMenu();
            return;
        }

        std::vector<std::string> ssids = {
            "FreeWiFi", "Starbucks_WiFi", "Airport_Free", "Hotel_Guest",
            "Public_WiFi", "Open_Network", "Guest_Access", "Free_Internet"
        };
        
        auto err = g_wifiModule->beaconSpam(ssids);
        if (err.isError()) {
            showMessage("Beacon Spam failed");
        } else {
            showMessage("Beacon Spam active");
        }
        showWiFiMenu();
    }));

    menu.addItem(Menu::MenuItem("Packet Sniffer", []() {
        if (!g_wifiModule || !g_wifiModule->isInitialized()) {
            showMessage("WiFi not initialized");
            showWiFiMenu();
            return;
        }

        auto err = g_wifiModule->startSniffer([](const uint8_t* data, size_t len) {
            Serial.printf("[WiFi] Packet: %zu bytes\n", len);
        });
        
        if (err.isError()) {
            showMessage("Sniffer failed");
        } else {
            showMessage("Sniffer started");
        }
        showWiFiMenu();
    }));

    menu.addItem(Menu::MenuItem("Back", []() {
        setupMainMenu();
    }));

    menu.show();
}

// BLE Menu
void showBLEMenu() {
    auto& menu = Menu::getInstance();
    menu.clear();

    menu.addItem(Menu::MenuItem("Scan Devices", []() {
        Serial.println("[BLE] Scanning devices...");
        showMessage("Scanning...", 2000);
        
        if (!g_bleModule || !g_bleModule->isInitialized()) {
            showMessage("BLE not initialized");
            showBLEMenu();
            return;
        }

        g_scannedBLEDevices.clear();
        auto err = g_bleModule->scanDevices(g_scannedBLEDevices, 5000);
        
        if (err.isError()) {
            Serial.printf("[BLE] Scan failed: %s\n", getErrorMessage(err.code));
            showMessage("Scan failed");
            delay(2000);
            showBLEMenu();
            return;
        }

        Serial.printf("[BLE] Found %zu devices\n", g_scannedBLEDevices.size());
        for (size_t i = 0; i < g_scannedBLEDevices.size() && i < 10; ++i) {
            Serial.printf("  %zu. %s (RSSI: %d)\n", 
                i+1, g_scannedBLEDevices[i].name.c_str(), g_scannedBLEDevices[i].rssi);
        }
        
        // Show device list menu
        if (g_scannedBLEDevices.empty()) {
            showMessage("No devices found");
            delay(2000);
            showBLEMenu();
        } else {
            showBLEDeviceList();
        }
    }));

    menu.addItem(Menu::MenuItem("iOS Spam", []() {
        if (!g_bleModule || !g_bleModule->isInitialized()) {
            showMessage("BLE not initialized");
            return;
        }

        auto err = g_bleModule->spamIOS("iPhone");
        if (err.isError()) {
            showMessage("iOS Spam failed");
        } else {
            showMessage("iOS Spam active");
        }
    }));

    menu.addItem(Menu::MenuItem("Android Spam", []() {
        if (!g_bleModule || !g_bleModule->isInitialized()) {
            showMessage("BLE not initialized");
            return;
        }

        auto err = g_bleModule->spamAndroid("Android");
        if (err.isError()) {
            showMessage("Android Spam failed");
        } else {
            showMessage("Android Spam active");
        }
    }));

    menu.addItem(Menu::MenuItem("Windows Spam", []() {
        if (!g_bleModule || !g_bleModule->isInitialized()) {
            showMessage("BLE not initialized");
            return;
        }

        auto err = g_bleModule->spamWindows("Windows");
        if (err.isError()) {
            showMessage("Windows Spam failed");
        } else {
            showMessage("Windows Spam active");
        }
    }));

    menu.addItem(Menu::MenuItem("Samsung Spam", []() {
        if (!g_bleModule || !g_bleModule->isInitialized()) {
            showMessage("BLE not initialized");
            return;
        }

        auto err = g_bleModule->spamSamsung("Samsung");
        if (err.isError()) {
            showMessage("Samsung Spam failed");
        } else {
            showMessage("Samsung Spam active");
        }
    }));

    menu.addItem(Menu::MenuItem("Back", []() {
        setupMainMenu();
    }));

    menu.show();
}

// RF Menu
void showRFMenu() {
    auto& menu = Menu::getInstance();
    menu.clear();

    menu.addItem(Menu::MenuItem("Initialize", []() {
        if (g_rfModule && g_rfModule->isInitialized()) {
            showMessage("RF already initialized");
            showRFMenu();
            return;
        }

        if (!g_rfModule) {
            g_rfModule = new RFModule();
        }

        auto err = g_rfModule->initialize();
        if (err.isError()) {
            char msg[64];
            snprintf(msg, sizeof(msg), "Init failed: %s", getErrorMessage(err.code));
            showMessage(msg);
        } else {
            showMessage("RF initialized");
        }
        delay(2000);
        showRFMenu();
    }));

    menu.addItem(Menu::MenuItem("Transmit Code", []() {
        showMessage("RF Transmit (needs hardware)");
    }));

    menu.addItem(Menu::MenuItem("Receive Code", []() {
        showMessage("RF Receive (needs hardware)");
    }));

    menu.addItem(Menu::MenuItem("Jammer", []() {
        if (!g_rfModule || !g_rfModule->isInitialized()) {
            showMessage("RF not initialized");
            showRFMenu();
            return;
        }

        auto err = g_rfModule->startJammer(false);  // false = full jammer
        if (err.isError()) {
            showMessage("Jammer failed");
        } else {
            showMessage("Jammer active");
        }
        delay(2000);
        showRFMenu();
    }));

    menu.addItem(Menu::MenuItem("Back", []() {
        setupMainMenu();
    }));

    menu.show();
}

// RFID Menu
void showRFIDMenu() {
    auto& menu = Menu::getInstance();
    menu.clear();

    menu.addItem(Menu::MenuItem("Read Tag", []() {
        showMessage("RFID Read (needs hardware)");
    }));

    menu.addItem(Menu::MenuItem("Write Tag", []() {
        showMessage("RFID Write (needs hardware)");
    }));

    menu.addItem(Menu::MenuItem("Emulate Tag", []() {
        showMessage("RFID Emulate (needs hardware)");
    }));

    menu.addItem(Menu::MenuItem("Back", []() {
        setupMainMenu();
    }));

    menu.show();
}

// BlackHat Tools Menu
void showBlackHatMenu() {
    auto& menu = Menu::getInstance();
    menu.clear();

    menu.addItem(Menu::MenuItem("Network Scan", []() {
        if (!g_blackhatTools || !g_blackhatTools->isInitialized()) {
            showMessage("BlackHat Tools not initialized");
            showBlackHatMenu();
            return;
        }

        Serial.println("[BlackHat] Starting network scan...");
        showMessage("Scanning...", 2000);

        g_scannedHosts.clear();
        auto err = g_blackhatTools->scanHosts("192.168.1.0/24", g_scannedHosts);
        
        if (err.isError()) {
            Serial.printf("[BlackHat] Scan failed: %s\n", getErrorMessage(err.code));
            showMessage("Scan failed");
            delay(2000);
            showBlackHatMenu();
            return;
        }

        Serial.printf("[BlackHat] Found %zu hosts\n", g_scannedHosts.size());
        for (size_t i = 0; i < g_scannedHosts.size() && i < 10; ++i) {
            Serial.printf("  %zu. %s\n", i+1, g_scannedHosts[i].c_str());
        }
        
        // Show host list menu
        if (g_scannedHosts.empty()) {
            showMessage("No hosts found");
            delay(2000);
            showBlackHatMenu();
        } else {
            showBlackHatHostList();
        }
    }));

    menu.addItem(Menu::MenuItem("Port Scan", []() {
        if (!g_blackhatTools || !g_blackhatTools->isInitialized()) {
            showMessage("BlackHat Tools not initialized");
            showBlackHatMenu();
            return;
        }

        Serial.println("[BlackHat] Port scan (use Serial for IP)");
        showMessage("Use Serial/WebUI");
        delay(2000);
        showBlackHatMenu();
    }));

    menu.addItem(Menu::MenuItem("Credential Harvester", []() {
        if (!g_blackhatTools || !g_blackhatTools->isInitialized()) {
            showMessage("BlackHat Tools not initialized");
            showBlackHatMenu();
            return;
        }

        auto err = g_blackhatTools->startCredentialHarvester("wlan0");
        if (err.isError()) {
            showMessage("Harvester failed");
        } else {
            showMessage("Harvester active");
        }
        delay(2000);
        showBlackHatMenu();
    }));

    menu.addItem(Menu::MenuItem("View Credentials", []() {
        if (!g_blackhatTools || !g_blackhatTools->isInitialized()) {
            showMessage("BlackHat Tools not initialized");
            showBlackHatMenu();
            return;
        }

        std::vector<std::pair<std::string, std::string>> creds;
        auto err = g_blackhatTools->getHarvestedCredentials(creds);
        
        if (err.isError()) {
            showMessage("Failed to get creds");
            delay(2000);
            showBlackHatMenu();
            return;
        }

        Serial.printf("[BlackHat] Found %zu credentials\n", creds.size());
        for (size_t i = 0; i < creds.size() && i < 10; ++i) {
            Serial.printf("  %zu. %s / %s\n", 
                i+1, creds[i].first.c_str(), creds[i].second.c_str());
        }
        
        char msg[64];
        snprintf(msg, sizeof(msg), "Found %zu creds", creds.size());
        showMessage(msg);
        delay(2000);
        showBlackHatMenu();
    }));

    menu.addItem(Menu::MenuItem("ARP Spoof", []() {
        showMessage("ARP Spoof (use Serial)");
    }));

    menu.addItem(Menu::MenuItem("Back", []() {
        setupMainMenu();
    }));

    menu.show();
}

// Physical Hack Menu
void showPhysicalHackMenu() {
    auto& menu = Menu::getInstance();
    menu.clear();

    menu.addItem(Menu::MenuItem("Initialize", []() {
        if (g_physicalHackModule && g_physicalHackModule->isInitialized()) {
            showMessage("Physical Hack already initialized");
            showPhysicalHackMenu();
            return;
        }

        if (!g_physicalHackModule) {
            g_physicalHackModule = new PhysicalHackModule();
        }

        auto err = g_physicalHackModule->initialize();
        if (err.isError()) {
            char msg[64];
            snprintf(msg, sizeof(msg), "Init failed: %s", getErrorMessage(err.code));
            showMessage(msg);
        } else {
            showMessage("Physical Hack initialized");
        }
        delay(2000);
        showPhysicalHackMenu();
    }));

    menu.addItem(Menu::MenuItem("Auto Exploit", []() {
        if (!g_physicalHackModule || !g_physicalHackModule->isInitialized()) {
            showMessage("Physical Hack not initialized");
            return;
        }

        Serial.println("[PhysicalHack] Starting auto exploit...");
        showMessage("Connecting...", 2000);

        auto err = g_physicalHackModule->executeAutoExploit();
        if (err.isError()) {
            Serial.printf("[PhysicalHack] Auto exploit failed: %s\n", getErrorMessage(err.code));
            showMessage("Exploit failed");
        } else {
            showMessage("Exploit executed!");
        }
        delay(2000);
        showPhysicalHackMenu();
    }));

    menu.addItem(Menu::MenuItem("Detect OS", []() {
        if (!g_physicalHackModule || !g_physicalHackModule->isInitialized()) {
            showMessage("Physical Hack not initialized");
            return;
        }

        Serial.println("[PhysicalHack] Detecting OS...");
        showMessage("Detecting...", 2000);

        PhysicalHackModule::OSInfo osInfo;
        auto err = g_physicalHackModule->detectOS(PhysicalHackModule::ConnectionType::AUTO, osInfo);
        
        if (err.isError() || osInfo.type == PhysicalHackModule::OSType::UNKNOWN) {
            Serial.println("[PhysicalHack] OS detection failed");
            showMessage("OS detection failed");
        } else {
            const char* osName = "Unknown";
            switch (osInfo.type) {
                case PhysicalHackModule::OSType::WINDOWS:
                    osName = "Windows";
                    break;
                case PhysicalHackModule::OSType::LINUX:
                    osName = "Linux";
                    break;
                case PhysicalHackModule::OSType::MACOS:
                    osName = "macOS";
                    break;
                case PhysicalHackModule::OSType::ANDROID:
                    osName = "Android";
                    break;
                case PhysicalHackModule::OSType::IOS:
                case PhysicalHackModule::OSType::IOS_JAILBROKEN:
                    osName = "iOS";
                    break;
                default:
                    break;
            }
            
            Serial.printf("[PhysicalHack] Detected: %s\n", osName);
            showMessage(osName);
        }
        delay(2000);
        showPhysicalHackMenu();
    }));

    menu.addItem(Menu::MenuItem("Connect USB", []() {
        if (!g_physicalHackModule || !g_physicalHackModule->isInitialized()) {
            showMessage("Physical Hack not initialized");
            showPhysicalHackMenu();
            return;
        }

        auto err = g_physicalHackModule->connectUSB(PhysicalHackModule::ConnectionType::AUTO);
        if (err.isError()) {
            showMessage("USB connect failed");
        } else {
            showMessage("USB connected");
        }
        delay(2000);
        showPhysicalHackMenu();
    }));

    menu.addItem(Menu::MenuItem("Connect BLE", []() {
        if (!g_physicalHackModule || !g_physicalHackModule->isInitialized()) {
            showMessage("Physical Hack not initialized");
            showPhysicalHackMenu();
            return;
        }

        auto err = g_physicalHackModule->connectBLE("");
        if (err.isError()) {
            showMessage("BLE connect failed");
        } else {
            showMessage("BLE connected");
        }
        delay(2000);
        showPhysicalHackMenu();
    }));

    menu.addItem(Menu::MenuItem("Back", []() {
        setupMainMenu();
    }));

    menu.show();
}

// BlackHat Host List Menu
void showBlackHatHostList() {
    auto& menu = Menu::getInstance();
    menu.clear();

    if (g_scannedHosts.empty()) {
        showMessage("No hosts found");
        delay(2000);
        showBlackHatMenu();
        return;
    }

    // Add each host to menu
    for (size_t i = 0; i < g_scannedHosts.size() && i < 15; ++i) {
        const std::string& host = g_scannedHosts[i];
        menu.addItem(Menu::MenuItem(host, [i]() {
            showBlackHatHostActions(i);
        }));
    }

    menu.addItem(Menu::MenuItem("Back", []() {
        showBlackHatMenu();
    }));

    menu.show();
}

// Show actions for selected host
void showBlackHatHostActions(size_t hostIndex) {
    if (hostIndex >= g_scannedHosts.size()) {
        showBlackHatMenu();
        return;
    }

    const std::string& host = g_scannedHosts[hostIndex];
    auto& menu = Menu::getInstance();
    menu.clear();

    menu.addItem(Menu::MenuItem("Port Scan", [hostIndex]() {
        if (!g_blackhatTools || !g_blackhatTools->isInitialized()) {
            showMessage("BlackHat Tools not initialized");
            showBlackHatHostActions(hostIndex);
            return;
        }

        const std::string& host = g_scannedHosts[hostIndex];
        showMessage("Port scanning...", 2000);
        
        // Port scan would be implemented here
        Serial.printf("[BlackHat] Port scanning %s\n", host.c_str());
        showMessage("Use Serial/WebUI");
        delay(2000);
        showBlackHatHostActions(hostIndex);
    }));

    menu.addItem(Menu::MenuItem("Info", [hostIndex]() {
        const std::string& host = g_scannedHosts[hostIndex];
        char info[128];
        snprintf(info, sizeof(info), "Host: %s\nStatus: Online", host.c_str());
        showMessage(info, 3000);
        delay(3000);
        showBlackHatHostActions(hostIndex);
    }));

    menu.addItem(Menu::MenuItem("Back", [hostIndex]() {
        showBlackHatHostList();
    }));

    menu.show();
}

// Physical Hack Exploit List Menu
void showPhysicalHackExploitList() {
    auto& menu = Menu::getInstance();
    menu.clear();

    if (g_availableExploits.empty()) {
        showMessage("No exploits found");
        delay(2000);
        showPhysicalHackMenu();
        return;
    }

    // Add each exploit to menu
    for (size_t i = 0; i < g_availableExploits.size() && i < 15; ++i) {
        const auto& exploit = g_availableExploits[i];
        menu.addItem(Menu::MenuItem(exploit.name, [i]() {
            showPhysicalHackExploitActions(i);
        }));
    }

    menu.addItem(Menu::MenuItem("Back", []() {
        showPhysicalHackMenu();
    }));

    menu.show();
}

// Show actions for selected exploit
void showPhysicalHackExploitActions(size_t exploitIndex) {
    if (exploitIndex >= g_availableExploits.size()) {
        showPhysicalHackMenu();
        return;
    }

    const auto& exploit = g_availableExploits[exploitIndex];
    auto& menu = Menu::getInstance();
    menu.clear();

    menu.addItem(Menu::MenuItem("Info", [exploitIndex]() {
        const auto& exploit = g_availableExploits[exploitIndex];
        char info[128];
        snprintf(info, sizeof(info), "%s\n%s\nOS: %s",
                 exploit.name.c_str(),
                 exploit.description.c_str(),
                 "Target OS");
        showMessage(info, 3000);
        delay(3000);
        showPhysicalHackExploitActions(exploitIndex);
    }));

    menu.addItem(Menu::MenuItem("Execute", [exploitIndex]() {
        if (!g_physicalHackModule || !g_physicalHackModule->isInitialized()) {
            showMessage("Physical Hack not initialized");
            showPhysicalHackExploitActions(exploitIndex);
            return;
        }

        const auto& exploit = g_availableExploits[exploitIndex];
        showMessage("Executing...", 2000);
        
        PhysicalHackModule::OSInfo osInfo;
        osInfo.type = exploit.targetOS;
        
        auto err = g_physicalHackModule->executeExploit(exploit, osInfo);
        if (err.isError()) {
            showMessage("Execution failed");
        } else {
            showMessage("Exploit executed!");
        }
        delay(2000);
        showPhysicalHackExploitActions(exploitIndex);
    }));

    menu.addItem(Menu::MenuItem("Back", [exploitIndex]() {
        showPhysicalHackExploitList();
    }));

    menu.show();
}

// Config Menu
void showConfigMenu() {
    auto& menu = Menu::getInstance();
    menu.clear();

    menu.addItem(Menu::MenuItem("Set Password", []() {
        showMessage("Use Serial/WebUI");
    }));

    menu.addItem(Menu::MenuItem("Brightness", []() {
        Config config;
        config.load();
        uint8_t brightness = config.getBrightness();
        Serial.printf("[Config] Current brightness: %d\n", brightness);
        showMessage("Use Serial/WebUI");
    }));

    menu.addItem(Menu::MenuItem("Back", []() {
        setupMainMenu();
    }));

    menu.show();
}

// IR Menu
void showIRMenu() {
    auto& menu = Menu::getInstance();
    menu.clear();

    menu.addItem(Menu::MenuItem("Initialize", []() {
        if (g_irModule && g_irModule->isInitialized()) {
            showMessage("IR already initialized");
            showIRMenu();
            return;
        }
        if (!g_irModule) {
            g_irModule = new IRModule();
        }
        auto err = g_irModule->initialize();
        if (err.isError()) {
            showMessage("IR init failed");
        } else {
            showMessage("IR initialized");
        }
        delay(2000);
        showIRMenu();
    }));

    menu.addItem(Menu::MenuItem("Transmit Code", []() {
        showMessage("Use Serial/WebUI");
        delay(2000);
        showIRMenu();
    }));

    menu.addItem(Menu::MenuItem("Receive Code", []() {
        if (!g_irModule || !g_irModule->isInitialized()) {
            showMessage("IR not initialized");
            showIRMenu();
            return;
        }
        showMessage("Receiving... (5s)");
        IRModule::IRCode code;
        auto err = g_irModule->receiveCode(code, 5000);
        if (err.isError()) {
            showMessage("Receive failed");
        } else {
            showMessage("Code received!");
        }
        delay(2000);
        showIRMenu();
    }));

    menu.addItem(Menu::MenuItem("TV-B-Gone", []() {
        if (!g_irModule || !g_irModule->isInitialized()) {
            showMessage("IR not initialized");
            showIRMenu();
            return;
        }
        showMessage("TV-B-Gone running...");
        auto err = g_irModule->tvBGone();
        if (err.isError()) {
            showMessage("TV-B-Gone failed");
        }
        delay(2000);
        showIRMenu();
    }));

    menu.addItem(Menu::MenuItem("IR Jammer", []() {
        if (!g_irModule || !g_irModule->isInitialized()) {
            showMessage("IR not initialized");
            showIRMenu();
            return;
        }
        auto err = g_irModule->startJammer(38000);
        if (err.isError()) {
            showMessage("Jammer failed");
        } else {
            showMessage("Jammer started");
        }
        delay(2000);
        showIRMenu();
    }));

    menu.addItem(Menu::MenuItem("Stop Jammer", []() {
        if (!g_irModule || !g_irModule->isInitialized()) {
            showMessage("IR not initialized");
            showIRMenu();
            return;
        }
        g_irModule->stopJammer();
        showMessage("Jammer stopped");
        delay(2000);
        showIRMenu();
    }));

    menu.addItem(Menu::MenuItem("Back", []() {
        setupMainMenu();
    }));

    menu.show();
}

// BadUSB Menu
void showBadUSBMenu() {
    auto& menu = Menu::getInstance();
    menu.clear();

    menu.addItem(Menu::MenuItem("Initialize", []() {
        if (g_badusbModule && g_badusbModule->isInitialized()) {
            showMessage("BadUSB already initialized");
            showBadUSBMenu();
            return;
        }
        if (!g_badusbModule) {
            g_badusbModule = new BadUSBModule();
        }
        auto err = g_badusbModule->initialize();
        if (err.isError()) {
            showMessage("BadUSB init failed");
        } else {
            showMessage("BadUSB initialized");
        }
        delay(2000);
        showBadUSBMenu();
    }));

    menu.addItem(Menu::MenuItem("Execute Script", []() {
        showMessage("Use Serial/WebUI");
        delay(2000);
        showBadUSBMenu();
    }));

    menu.addItem(Menu::MenuItem("List Scripts", []() {
        showMessage("Use Serial/WebUI");
        delay(2000);
        showBadUSBMenu();
    }));

    menu.addItem(Menu::MenuItem("Back", []() {
        setupMainMenu();
    }));

    menu.show();
}

// GPS Menu
void showGPSMenu() {
    auto& menu = Menu::getInstance();
    menu.clear();

    menu.addItem(Menu::MenuItem("Initialize", []() {
        if (g_gpsModule && g_gpsModule->isInitialized()) {
            showMessage("GPS already initialized");
            showGPSMenu();
            return;
        }
        if (!g_gpsModule) {
            g_gpsModule = new GPSModule();
        }
        auto err = g_gpsModule->initialize();
        if (err.isError()) {
            showMessage("GPS init failed");
        } else {
            showMessage("GPS initialized");
        }
        delay(2000);
        showGPSMenu();
    }));

    menu.addItem(Menu::MenuItem("Start Tracking", []() {
        if (!g_gpsModule || !g_gpsModule->isInitialized()) {
            showMessage("GPS not initialized");
            showGPSMenu();
            return;
        }
        showMessage("Tracking started");
        delay(2000);
        showGPSMenu();
    }));

    menu.addItem(Menu::MenuItem("Wardriving", []() {
        if (!g_gpsModule || !g_gpsModule->isInitialized()) {
            showMessage("GPS not initialized");
            showGPSMenu();
            return;
        }
        showMessage("Wardriving started");
        delay(2000);
        showGPSMenu();
    }));

    menu.addItem(Menu::MenuItem("Back", []() {
        setupMainMenu();
    }));

    menu.show();
}

// FM Radio Menu
void showFMMenu() {
    auto& menu = Menu::getInstance();
    menu.clear();

    menu.addItem(Menu::MenuItem("Initialize", []() {
        if (g_fmModule && g_fmModule->isInitialized()) {
            showMessage("FM already initialized");
            showFMMenu();
            return;
        }
        if (!g_fmModule) {
            g_fmModule = new FMModule();
        }
        auto err = g_fmModule->initialize();
        if (err.isError()) {
            showMessage("FM init failed");
        } else {
            showMessage("FM initialized");
        }
        delay(2000);
        showFMMenu();
    }));

    menu.addItem(Menu::MenuItem("Broadcast", []() {
        if (!g_fmModule || !g_fmModule->isInitialized()) {
            showMessage("FM not initialized");
            showFMMenu();
            return;
        }
        showMessage("Use Serial/WebUI");
        delay(2000);
        showFMMenu();
    }));

    menu.addItem(Menu::MenuItem("Scan Frequencies", []() {
        if (!g_fmModule || !g_fmModule->isInitialized()) {
            showMessage("FM not initialized");
            showFMMenu();
            return;
        }
        showMessage("Scanning...");
        delay(2000);
        showFMMenu();
    }));

    menu.addItem(Menu::MenuItem("Back", []() {
        setupMainMenu();
    }));

    menu.show();
}

// ESPNOW Menu
void showESPNOWMenu() {
    auto& menu = Menu::getInstance();
    menu.clear();

    menu.addItem(Menu::MenuItem("Initialize", []() {
        if (g_espnowModule && g_espnowModule->isInitialized()) {
            showMessage("ESPNOW already initialized");
            showESPNOWMenu();
            return;
        }
        if (!g_espnowModule) {
            g_espnowModule = new ESPNOWModule();
        }
        auto err = g_espnowModule->initialize();
        if (err.isError()) {
            showMessage("ESPNOW init failed");
        } else {
            showMessage("ESPNOW initialized");
        }
        delay(2000);
        showESPNOWMenu();
    }));

    menu.addItem(Menu::MenuItem("Send File", []() {
        showMessage("Use Serial/WebUI");
        delay(2000);
        showESPNOWMenu();
    }));

    menu.addItem(Menu::MenuItem("Receive File", []() {
        showMessage("Use Serial/WebUI");
        delay(2000);
        showESPNOWMenu();
    }));

    menu.addItem(Menu::MenuItem("Back", []() {
        setupMainMenu();
    }));

    menu.show();
}

// NRF24 Menu
void showNRF24Menu() {
    auto& menu = Menu::getInstance();
    menu.clear();

    menu.addItem(Menu::MenuItem("Initialize", []() {
        if (g_nrf24Module && g_nrf24Module->isInitialized()) {
            showMessage("NRF24 already initialized");
            showNRF24Menu();
            return;
        }
        if (!g_nrf24Module) {
            g_nrf24Module = new NRF24Module();
        }
        auto err = g_nrf24Module->initialize();
        if (err.isError()) {
            showMessage("NRF24 init failed");
        } else {
            showMessage("NRF24 initialized");
        }
        delay(2000);
        showNRF24Menu();
    }));

    menu.addItem(Menu::MenuItem("Jammer", []() {
        if (!g_nrf24Module || !g_nrf24Module->isInitialized()) {
            showMessage("NRF24 not initialized");
            showNRF24Menu();
            return;
        }
        showMessage("Use Serial/WebUI");
        delay(2000);
        showNRF24Menu();
    }));

    menu.addItem(Menu::MenuItem("Spectrum Analyzer", []() {
        if (!g_nrf24Module || !g_nrf24Module->isInitialized()) {
            showMessage("NRF24 not initialized");
            showNRF24Menu();
            return;
        }
        showMessage("Use Serial/WebUI");
        delay(2000);
        showNRF24Menu();
    }));

    menu.addItem(Menu::MenuItem("Back", []() {
        setupMainMenu();
    }));

    menu.show();
}

// Ethernet Menu
void showEthernetMenu() {
    auto& menu = Menu::getInstance();
    menu.clear();

    menu.addItem(Menu::MenuItem("Initialize", []() {
        if (g_ethernetModule && g_ethernetModule->isInitialized()) {
            showMessage("Ethernet already initialized");
            showEthernetMenu();
            return;
        }
        if (!g_ethernetModule) {
            g_ethernetModule = new EthernetModule();
        }
        auto err = g_ethernetModule->initialize();
        if (err.isError()) {
            showMessage("Ethernet init failed");
        } else {
            showMessage("Ethernet initialized");
        }
        delay(2000);
        showEthernetMenu();
    }));

    menu.addItem(Menu::MenuItem("ARP Spoof", []() {
        showMessage("Use Serial/WebUI");
        delay(2000);
        showEthernetMenu();
    }));

    menu.addItem(Menu::MenuItem("Back", []() {
        setupMainMenu();
    }));

    menu.show();
}

// Interpreter Menu
void showInterpreterMenu() {
    auto& menu = Menu::getInstance();
    menu.clear();

    menu.addItem(Menu::MenuItem("Initialize", []() {
        if (g_interpreterModule && g_interpreterModule->isInitialized()) {
            showMessage("Interpreter already initialized");
            showInterpreterMenu();
            return;
        }
        if (!g_interpreterModule) {
            g_interpreterModule = new InterpreterModule();
        }
        auto err = g_interpreterModule->initialize();
        if (err.isError()) {
            showMessage("Interpreter init failed");
        } else {
            showMessage("Interpreter initialized");
        }
        delay(2000);
        showInterpreterMenu();
    }));

    menu.addItem(Menu::MenuItem("Execute Script", []() {
        showMessage("Use Serial/WebUI");
        delay(2000);
        showInterpreterMenu();
    }));

    menu.addItem(Menu::MenuItem("Back", []() {
        setupMainMenu();
    }));

    menu.show();
}

// Others Menu
void showOthersMenu() {
    auto& menu = Menu::getInstance();
    menu.clear();

    menu.addItem(Menu::MenuItem("Initialize", []() {
        if (g_othersModule && g_othersModule->isInitialized()) {
            showMessage("Others already initialized");
            showOthersMenu();
            return;
        }
        if (!g_othersModule) {
            g_othersModule = new OthersModule();
        }
        auto err = g_othersModule->initialize();
        if (err.isError()) {
            showMessage("Others init failed");
        } else {
            showMessage("Others initialized");
        }
        delay(2000);
        showOthersMenu();
    }));

    menu.addItem(Menu::MenuItem("Reverse Shell", []() {
        if (!g_othersModule || !g_othersModule->isInitialized()) {
            showMessage("Others not initialized");
            showOthersMenu();
            return;
        }
        showMessage("Use Serial/WebUI");
        delay(2000);
        showOthersMenu();
    }));

    menu.addItem(Menu::MenuItem("Back", []() {
        setupMainMenu();
    }));

    menu.show();
}

// Setup main menu (called from main.cpp)
void setupMainMenu() {
    auto& menu = Menu::getInstance();
    menu.clear();

    menu.addItem(Menu::MenuItem("WiFi", showWiFiMenu));
    menu.addItem(Menu::MenuItem("BLE", showBLEMenu));
    menu.addItem(Menu::MenuItem("RF", showRFMenu));
    menu.addItem(Menu::MenuItem("RFID", showRFIDMenu));
    menu.addItem(Menu::MenuItem("Physical Hack", showPhysicalHackMenu));
    menu.addItem(Menu::MenuItem("BlackHat Tools", showBlackHatMenu));
    menu.addItem(Menu::MenuItem("IR", showIRMenu));
    menu.addItem(Menu::MenuItem("BadUSB", showBadUSBMenu));
    menu.addItem(Menu::MenuItem("NRF24", showNRF24Menu));
    menu.addItem(Menu::MenuItem("GPS", showGPSMenu));
    menu.addItem(Menu::MenuItem("FM Radio", showFMMenu));
    menu.addItem(Menu::MenuItem("ESPNOW", showESPNOWMenu));
    menu.addItem(Menu::MenuItem("Ethernet", showEthernetMenu));
    menu.addItem(Menu::MenuItem("Interpreter", showInterpreterMenu));
    menu.addItem(Menu::MenuItem("Others", showOthersMenu));
    menu.addItem(Menu::MenuItem("Config", showConfigMenu));
    
    // Show menu after setup
    menu.show();
}

