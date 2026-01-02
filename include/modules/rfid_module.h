#pragma once

#include "core/module_interface.h"
#include "core/errors.h"
#include <vector>
#include <string>

namespace NightStrike {
namespace Modules {

/**
 * @brief RFID/NFC module
 *
 * Features:
 * - Read/Write tags
 * - Emulate tags
 * - Mifare operations
 * - NFC attacks
 */
class RFIDModule : public Core::IModule {
public:
    struct TagData {
        std::vector<uint8_t> uid;
        std::vector<uint8_t> data;
        uint8_t type;
        std::string name;
    };

    RFIDModule();
    ~RFIDModule() override = default;

    // IModule interface
    const char* getName() const override { return "RFID"; }
    Core::Error initialize() override;
    Core::Error shutdown() override;
    bool isInitialized() const override { return _initialized; }
    bool isSupported() const override;

    // Tag operations
    Core::Error readTag(TagData& tag);
    Core::Error writeTag(const TagData& tag);
    Core::Error eraseTag();
    Core::Error emulateTag(const TagData& tag);
    Core::Error stopEmulation();

    // Mifare operations
    Core::Error readMifareBlock(uint8_t block, std::vector<uint8_t>& data);
    Core::Error writeMifareBlock(uint8_t block, const std::vector<uint8_t>& data);
    Core::Error authenticateMifare(const std::vector<uint8_t>& key);

    // Tag management
    Core::Error saveTag(const TagData& tag, const std::string& name);
    Core::Error loadTag(const std::string& name, TagData& tag);
    Core::Error listTags(std::vector<std::string>& names);

    // Amiibo support
    Core::Error readAmiibo(TagData& tag);
    Core::Error writeAmiibo(const TagData& tag, const std::string& dumpFile);
    Core::Error emulateAmiibo(const std::string& dumpFile);

    // Chameleon (multi-tag emulation)
    Core::Error startChameleon();
    Core::Error stopChameleon();
    Core::Error addChameleonSlot(const TagData& tag, uint8_t slot);
    Core::Error switchChameleonSlot(uint8_t slot);

    // EMV/Credit card reading
    Core::Error readEMV(std::vector<uint8_t>& cardData);
    Core::Error parseEMV(const std::vector<uint8_t>& cardData, std::string& pan, std::string& expiry);

private:
    bool _emulating = false;
    bool _chameleonActive = false;
    uint8_t _currentSlot = 0;
    std::vector<TagData> _chameleonSlots;
};

} // namespace Modules
} // namespace NightStrike

