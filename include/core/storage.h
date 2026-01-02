#pragma once

#include "errors.h"
#include <vector>
#include <string>

// Forward declaration for FS
namespace fs {
    class FS;
}

namespace NightStrike {
namespace Core {

/**
 * @brief Storage management system
 * Supports LittleFS and SD card
 */
class Storage {
public:
    static Storage& getInstance();

    // Initialization
    Error initialize();
    Error shutdown();

    // Storage status
    bool isLittleFSMounted() const { return _littlefsMounted; }
    bool isSDCardMounted() const { return _sdcardMounted; }
    bool isInitialized() const { return _initialized; }

    // File operations
    Error readFile(const std::string& path, std::vector<uint8_t>& data, bool preferSD = false);
    Error writeFile(const std::string& path, const std::vector<uint8_t>& data, bool preferSD = false);
    Error deleteFile(const std::string& path, bool preferSD = false);
    Error listFiles(const std::string& path, std::vector<std::string>& files, bool preferSD = false);
    bool fileExists(const std::string& path, bool preferSD = false);

    // Storage info
    uint64_t getFreeSpace(bool preferSD = false);

private:
    Storage() = default;
    ~Storage() = default;
    Storage(const Storage&) = delete;
    Storage& operator=(const Storage&) = delete;

    bool _initialized = false;
    bool _littlefsMounted = false;
    bool _sdcardMounted = false;

    bool setupSDCard();
    fs::FS* getStorage(bool preferSD);
};

} // namespace Core
} // namespace NightStrike

