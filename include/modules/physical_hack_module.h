#pragma once

#include "core/module_interface.h"
#include "core/errors.h"
#include <vector>
#include <string>
#include <map>
#include <functional>

namespace NightStrike {
namespace Modules {

/**
 * @brief Physical Hack Module - автоматическое определение ОС и подбор эксплойтов
 * 
 * Поддерживает:
 * - USB Type-C (HID, Mass Storage, Serial) - к компьютерам, Android, iPhone
 * - Bluetooth (BLE HID) - беспроводные атаки
 * - Автоматическое определение ОС
 * - Библиотека эксплойтов для каждой ОС
 */
class PhysicalHackModule : public Core::IModule {
public:
    enum class OSType {
        UNKNOWN,
        WINDOWS,
        WINDOWS_10,
        WINDOWS_11,
        LINUX,
        MACOS,
        ANDROID,
        ANDROID_ADB,      // Android с включенным ADB
        IOS,
        IOS_JAILBROKEN    // iOS с джейлбрейком
    };

    enum class ConnectionType {
        USB_HID,         // USB HID Keyboard
        USB_MASS_STORAGE,// USB Mass Storage
        USB_SERIAL,      // USB Serial/CDC
        BLE_HID,         // Bluetooth HID
        AUTO             // Автоматический выбор
    };

    struct OSInfo {
        OSType type = OSType::UNKNOWN;
        std::string version;
        std::string build;
        bool isAdmin = false;
        bool isRoot = false;
        std::string username;
        std::string hostname;
    };

    struct ExploitPayload {
        std::string name;
        std::string description;
        OSType targetOS;
        ConnectionType connectionType;
        std::string script;  // Ducky script или команды
        bool requiresAdmin = false;
        bool requiresRoot = false;
    };

    PhysicalHackModule();
    ~PhysicalHackModule() override = default;

    // IModule interface
    const char* getName() const override { return "Physical Hack"; }
    Core::Error initialize() override;
    Core::Error shutdown() override;
    bool isInitialized() const override { return _initialized; }
    bool isSupported() const override;

    // OS Detection
    Core::Error detectOS(ConnectionType connection, OSInfo& osInfo);
    Core::Error detectOSViaUSB(OSInfo& osInfo);
    Core::Error detectOSViaBLE(OSInfo& osInfo);
    
    // Exploit Management
    Core::Error loadExploitLibrary();
    Core::Error getExploitsForOS(OSType os, std::vector<ExploitPayload>& exploits);
    Core::Error getBestExploit(const OSInfo& osInfo, ExploitPayload& exploit);
    
    // Execution
    Core::Error executeExploit(const ExploitPayload& exploit, const OSInfo& osInfo);
    Core::Error executeAutoExploit(ConnectionType connection = ConnectionType::AUTO);
    
    // Connection Management
    Core::Error connectUSB(ConnectionType type = ConnectionType::AUTO);
    Core::Error connectBLE(const std::string& targetDevice = "");
    Core::Error disconnect();
    
    // Payload Customization
    Core::Error addCustomPayload(const ExploitPayload& payload);
    Core::Error removeCustomPayload(const std::string& name);
    
    // Status
    bool isConnected() const { return _connected; }
    ConnectionType getConnectionType() const { return _connectionType; }
    OSInfo getDetectedOS() const { return _detectedOS; }
    
    // Callbacks
    void setOSDetectedCallback(std::function<void(const OSInfo&)> callback) {
        _osDetectedCallback = callback;
    }
    void setExploitProgressCallback(std::function<void(uint32_t, uint32_t)> callback) {
        _exploitProgressCallback = callback;
    }

private:
    bool _initialized = false;
    bool _connected = false;
    ConnectionType _connectionType = ConnectionType::AUTO;
    OSInfo _detectedOS;
    
    std::map<OSType, std::vector<ExploitPayload>> _exploitLibrary;
    std::function<void(const OSInfo&)> _osDetectedCallback;
    std::function<void(uint32_t, uint32_t)> _exploitProgressCallback;
    
    // OS Detection helpers
    Core::Error detectWindows(OSInfo& osInfo);
    Core::Error detectLinux(OSInfo& osInfo);
    Core::Error detectMacOS(OSInfo& osInfo);
    Core::Error detectAndroid(OSInfo& osInfo);
    Core::Error detectIOS(OSInfo& osInfo);
    
    // USB helpers
    Core::Error initUSBHID();
    Core::Error initUSBMassStorage();
    Core::Error initUSBSerial();
    
    // BLE helpers
    Core::Error initBLEHID();
    
    // Exploit execution
    Core::Error executeWindowsExploit(const ExploitPayload& exploit);
    Core::Error executeLinuxExploit(const ExploitPayload& exploit);
    Core::Error executeMacOSExploit(const ExploitPayload& exploit);
    Core::Error executeAndroidExploit(const ExploitPayload& exploit);
    Core::Error executeIOSExploit(const ExploitPayload& exploit);
    
    // Payload generation
    std::string generateWindowsPayload(const ExploitPayload& exploit);
    std::string generateLinuxPayload(const ExploitPayload& exploit);
    std::string generateMacOSPayload(const ExploitPayload& exploit);
    std::string generateAndroidPayload(const ExploitPayload& exploit);
    std::string generateIOSPayload(const ExploitPayload& exploit);
    
    // Built-in exploit library initialization
    void initBuiltinExploits();
};

} // namespace Modules
} // namespace NightStrike

