/**
 * NightStrike Firmware - Main Entry Point
 *
 * Advanced ESP32 Firmware for Offensive Security Operations
 */

#include "core/system.h"
#include "core/config.h"
#include "core/display.h"
#include "core/input.h"
#include "core/menu.h"
#include "core/web_ui.h"
#include "core/storage.h"
#include "core/network.h"
#include "core/hardware_detection.h"
#include "core/power_management.h"
#include "core/errors.h"
#include "modules/wifi_module.h"
#include "modules/ble_module.h"
#include "modules/rf_module.h"
#include "modules/rfid_module.h"
#include "modules/blackhat_tools.h"
#include "modules/ir_module.h"
#include "modules/badusb_module.h"
#include "modules/nrf24_module.h"
#include "modules/gps_module.h"
#include "modules/others_module.h"
#include "modules/ethernet_module.h"
#include "modules/interpreter_module.h"
#include "modules/fm_module.h"
#include "modules/espnow_module.h"
#include "modules/physical_hack_module.h"
// Forward declaration
void setupMainMenu();
#include <Arduino.h>

using namespace NightStrike::Core;
using namespace NightStrike::Modules;

// Global module instances
WiFiModule* g_wifiModule = nullptr;
BLEModule* g_bleModule = nullptr;
RFModule* g_rfModule = nullptr;
RFIDModule* g_rfidModule = nullptr;
BlackHatToolsModule* g_blackhatTools = nullptr;
IRModule* g_irModule = nullptr;
BadUSBModule* g_badusbModule = nullptr;
NRF24Module* g_nrf24Module = nullptr;
GPSModule* g_gpsModule = nullptr;
OthersModule* g_othersModule = nullptr;
EthernetModule* g_ethernetModule = nullptr;
InterpreterModule* g_interpreterModule = nullptr;
FMModule* g_fmModule = nullptr;
ESPNOWModule* g_espnowModule = nullptr;
PhysicalHackModule* g_physicalHackModule = nullptr;

void setup() {
    // Initialize system
    auto& system = System::getInstance();
    Error err = system.initialize();
    if (err.isError()) {
        Serial.printf("[FATAL] System initialization failed: %s\n", getErrorMessage(err.code));
        while (1) delay(1000);  // Halt on critical error
    }

    Serial.println("[System] Initialization complete");

    // Detect hardware
    auto& hwDetect = HardwareDetection::getInstance();
    err = hwDetect.detectAll();
    if (err.isError()) {
        Serial.printf("[WARN] Hardware detection failed: %s\n", getErrorMessage(err.code));
    }

    // Initialize storage
    auto& storage = Storage::getInstance();
    err = storage.initialize();
    if (err.isError()) {
        Serial.printf("[WARN] Storage init failed: %s\n", getErrorMessage(err.code));
    }

    // Initialize network
    auto& network = Network::getInstance();
    err = network.initialize();
    if (err.isError()) {
        Serial.printf("[WARN] Network init failed: %s\n", getErrorMessage(err.code));
    }

    // Initialize display
    auto& display = Display::getInstance();
    err = display.initialize();
    if (err.isError()) {
        Serial.printf("[WARN] Display init failed: %s\n", getErrorMessage(err.code));
    } else {
        display.fillScreen(Display::Color::Black());
        display.setTextColor(Display::Color::Green(), Display::Color::Black());
        display.setTextSize(2);
        display.drawTextCentered(Display::Point(display.getSize().width / 2,
                                                display.getSize().height / 2),
                                 "NightStrike");
        delay(2000);
    }

    // Initialize power management
    auto& power = PowerManagement::getInstance();
    err = power.initialize();
    if (err.isError()) {
        Serial.printf("[WARN] Power management init failed: %s\n", getErrorMessage(err.code));
    }

    // Initialize input
    auto& input = Input::getInstance();
    err = input.initialize();
    if (err.isError()) {
        Serial.printf("[WARN] Input init failed: %s\n", getErrorMessage(err.code));
    }

    // Load configuration
    Config config;
    err = config.load();
    if (err.isError()) {
        Serial.printf("[WARN] Failed to load config: %s, using defaults\n", getErrorMessage(err.code));
    }

    // Check if password needs to be changed (security requirement)
    if (config.requiresPasswordChange()) {
        Serial.println("[SECURITY] Password change required on first boot!");
        display.clear();
        display.setTextColor(Display::Color::Red(), Display::Color::Black());
        display.setTextSize(1);
        display.drawTextCentered(Display::Point(display.getSize().width / 2,
                                                display.getSize().height / 2),
                                 "Set Admin Password!");
        delay(3000);
    }

    // Initialize modules
    g_wifiModule = new WiFiModule();
    err = g_wifiModule->initialize();
    if (err.isError()) {
        Serial.printf("[WARN] WiFi module init failed: %s\n", getErrorMessage(err.code));
    }

    g_bleModule = new BLEModule();
    err = g_bleModule->initialize();
    if (err.isError()) {
        Serial.printf("[WARN] BLE module init failed: %s\n", getErrorMessage(err.code));
    }

    g_rfModule = new RFModule();
    err = g_rfModule->initialize();
    if (err.isError()) {
        Serial.printf("[WARN] RF module init failed: %s\n", getErrorMessage(err.code));
    }

    g_rfidModule = new RFIDModule();
    err = g_rfidModule->initialize();
    if (err.isError()) {
        Serial.printf("[WARN] RFID module init failed: %s\n", getErrorMessage(err.code));
    }

    g_blackhatTools = new BlackHatToolsModule();
    err = g_blackhatTools->initialize();
    if (err.isError()) {
        Serial.printf("[WARN] BlackHat Tools init failed: %s\n", getErrorMessage(err.code));
    }

    g_irModule = new IRModule();
    err = g_irModule->initialize();
    if (err.isError()) {
        Serial.printf("[WARN] IR module init failed: %s\n", getErrorMessage(err.code));
    }

    g_badusbModule = new BadUSBModule();
    err = g_badusbModule->initialize();
    if (err.isError()) {
        Serial.printf("[WARN] BadUSB module init failed: %s\n", getErrorMessage(err.code));
    }

    g_nrf24Module = new NRF24Module();
    err = g_nrf24Module->initialize();
    if (err.isError()) {
        Serial.printf("[WARN] NRF24 module init failed: %s\n", getErrorMessage(err.code));
    }

    g_gpsModule = new GPSModule();
    err = g_gpsModule->initialize();
    if (err.isError()) {
        Serial.printf("[WARN] GPS module init failed: %s\n", getErrorMessage(err.code));
    }

    g_othersModule = new OthersModule();
    err = g_othersModule->initialize();
    if (err.isError()) {
        Serial.printf("[WARN] Others module init failed: %s\n", getErrorMessage(err.code));
    }

    g_ethernetModule = new EthernetModule();
    err = g_ethernetModule->initialize();
    if (err.isError()) {
        Serial.printf("[WARN] Ethernet module init failed: %s\n", getErrorMessage(err.code));
    }

    g_interpreterModule = new InterpreterModule();
    err = g_interpreterModule->initialize();
    if (err.isError()) {
        Serial.printf("[WARN] Interpreter module init failed: %s\n", getErrorMessage(err.code));
    }

    g_fmModule = new FMModule();
    err = g_fmModule->initialize();
    if (err.isError()) {
        Serial.printf("[WARN] FM module init failed: %s\n", getErrorMessage(err.code));
    }

    g_espnowModule = new ESPNOWModule();
    err = g_espnowModule->initialize();
    if (err.isError()) {
        Serial.printf("[WARN] ESPNOW module init failed: %s\n", getErrorMessage(err.code));
    }

    g_physicalHackModule = new PhysicalHackModule();
    err = g_physicalHackModule->initialize();
    if (err.isError()) {
        Serial.printf("[WARN] Physical Hack module init failed: %s\n", getErrorMessage(err.code));
    }

    // Initialize menu
    auto& menu = Menu::getInstance();
    err = menu.initialize();
    if (err.isError()) {
        Serial.printf("[WARN] Menu init failed: %s\n", getErrorMessage(err.code));
    }

    // Setup main menu with handlers
    setupMainMenu();

    // Initialize Web UI
    auto& webUI = WebUI::getInstance();
    err = webUI.initialize(80);
    if (err.isError()) {
        Serial.printf("[WARN] WebUI init failed: %s\n", getErrorMessage(err.code));
    } else {
        Serial.printf("[WebUI] Started at %s\n", webUI.getURL().c_str());
    }

    // Show menu
    menu.show();

    Serial.println("[System] Setup complete");
}

void loop() {
    // Update input
    auto& input = Input::getInstance();
    input.update();

    // Update menu
    auto& menu = Menu::getInstance();
    menu.update();

    delay(10);
}
