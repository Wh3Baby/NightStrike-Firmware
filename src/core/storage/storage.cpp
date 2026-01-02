#include "core/storage.h"
#include <LittleFS.h>
#include <SD.h>
#include <SPI.h>

namespace NightStrike {
namespace Core {

Storage& Storage::getInstance() {
    static Storage instance;
    return instance;
}

Error Storage::initialize() {
    if (_initialized) {
        return Error(ErrorCode::ALREADY_INITIALIZED);
    }

    // Initialize LittleFS (if filesystem partition exists)
#ifdef BOARD_HAS_FILESYSTEM
    if (!LittleFS.begin(true)) {
        Serial.println("[Storage] LittleFS format failed, trying format...");
        LittleFS.format();
        if (!LittleFS.begin(true)) {
            return Error(ErrorCode::STORAGE_NOT_MOUNTED, "LittleFS init failed");
        }
    }
#else
    // No filesystem partition - use RAM storage or SD card only
    Serial.println("[Storage] No LittleFS partition, using SD card only");
#endif

    _littlefsMounted = true;
    Serial.println("[Storage] LittleFS mounted");

    // Try to mount SD card
    _sdcardMounted = setupSDCard();
    if (_sdcardMounted) {
        Serial.println("[Storage] SD card mounted");
    } else {
        Serial.println("[Storage] SD card not available");
    }

    _initialized = true;
    return Error(ErrorCode::SUCCESS);
}

Error Storage::shutdown() {
    if (!_initialized) {
        return Error(ErrorCode::NOT_INITIALIZED);
    }

    if (_sdcardMounted) {
        SD.end();
        _sdcardMounted = false;
    }

    LittleFS.end();
    _littlefsMounted = false;
    _initialized = false;

    return Error(ErrorCode::SUCCESS);
}

bool Storage::setupSDCard() {
#ifdef M5STICKC_PLUS2
    // M5StickC PLUS2: SD card on SPI (default pins)
    // CS pin may vary, try common pins
    if (!SD.begin(4)) {  // Try GPIO 4 first
        if (!SD.begin(5)) {  // Try GPIO 5
            return false;
        }
    }
#else
    // Generic ESP32 - try default SPI pins
    if (!SD.begin()) {
        return false;
    }
#endif

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        return false;
    }

    return true;
}

fs::FS* Storage::getStorage(bool preferSD) {
    if (preferSD && _sdcardMounted) {
        return &SD;
    }

    if (_littlefsMounted) {
        return &LittleFS;
    }

    return nullptr;
}

Error Storage::readFile(const std::string& path, std::vector<uint8_t>& data, bool preferSD) {
    fs::FS* fs = getStorage(preferSD);
    if (!fs) {
        return Error(ErrorCode::STORAGE_NOT_MOUNTED);
    }

    File file = fs->open(path.c_str(), "r");
    if (!file) {
        return Error(ErrorCode::FILE_NOT_FOUND);
    }

    data.resize(file.size());
    size_t read = file.read(data.data(), data.size());
    file.close();

    if (read != data.size()) {
        return Error(ErrorCode::FILE_READ_ERROR);
    }

    return Error(ErrorCode::SUCCESS);
}

Error Storage::writeFile(const std::string& path, const std::vector<uint8_t>& data, bool preferSD) {
    fs::FS* fs = getStorage(preferSD);
    if (!fs) {
        return Error(ErrorCode::STORAGE_NOT_MOUNTED);
    }

    File file = fs->open(path.c_str(), "w");
    if (!file) {
        return Error(ErrorCode::FILE_WRITE_ERROR);
    }

    size_t written = file.write(data.data(), data.size());
    file.close();

    if (written != data.size()) {
        return Error(ErrorCode::FILE_WRITE_ERROR);
    }

    return Error(ErrorCode::SUCCESS);
}

Error Storage::deleteFile(const std::string& path, bool preferSD) {
    fs::FS* fs = getStorage(preferSD);
    if (!fs) {
        return Error(ErrorCode::STORAGE_NOT_MOUNTED);
    }

    if (!fs->exists(path.c_str())) {
        return Error(ErrorCode::FILE_NOT_FOUND);
    }

    if (!fs->remove(path.c_str())) {
        return Error(ErrorCode::FILE_DELETE_ERROR);
    }

    return Error(ErrorCode::SUCCESS);
}

Error Storage::listFiles(const std::string& path, std::vector<std::string>& files, bool preferSD) {
    fs::FS* fs = getStorage(preferSD);
    if (!fs) {
        return Error(ErrorCode::STORAGE_NOT_MOUNTED);
    }

    files.clear();
    File root = fs->open(path.c_str());
    if (!root || !root.isDirectory()) {
        return Error(ErrorCode::FILE_NOT_FOUND);
    }

    File file = root.openNextFile();
    while (file) {
        files.push_back(file.name());
        file = root.openNextFile();
    }

    root.close();
    return Error(ErrorCode::SUCCESS);
}

bool Storage::fileExists(const std::string& path, bool preferSD) {
    fs::FS* fs = getStorage(preferSD);
    if (!fs) {
        return false;
    }

    return fs->exists(path.c_str());
}

uint64_t Storage::getFreeSpace(bool preferSD) {
    fs::FS* fs = getStorage(preferSD);
    if (!fs) {
        return 0;
    }

    if (preferSD && _sdcardMounted) {
        return (SD.totalBytes() - SD.usedBytes());
    }

    // LittleFS doesn't have a direct free space API
    // Would need to calculate manually
    return 0;
}

} // namespace Core
} // namespace NightStrike

