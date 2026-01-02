#include "modules/ir_module.h"
#include <Arduino.h>
#include <driver/rmt.h>
#include <driver/gpio.h>

namespace NightStrike {
namespace Modules {

IRModule::IRModule() {
}

Core::Error IRModule::initialize() {
    if (_initialized) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }

    // Initialize RMT for IR transmission
    rmt_config_t rmt_tx_config = RMT_DEFAULT_CONFIG_TX((gpio_num_t)_txPin, RMT_CHANNEL_0);
    rmt_tx_config.tx_config.carrier_en = true;
    rmt_tx_config.tx_config.carrier_freq_hz = _frequency;
    rmt_tx_config.tx_config.carrier_duty_percent = 50;
    rmt_config(&rmt_tx_config);
    rmt_driver_install(RMT_CHANNEL_0, 0, 0);

    // Initialize RMT for IR reception
    rmt_config_t rmt_rx_config = RMT_DEFAULT_CONFIG_RX((gpio_num_t)_rxPin, RMT_CHANNEL_1);
    rmt_rx_config.rx_config.filter_en = true;
    rmt_rx_config.rx_config.filter_ticks_thresh = 100;
    rmt_config(&rmt_rx_config);
    rmt_driver_install(RMT_CHANNEL_1, 1024, 0);

    Serial.println("[IR] Module initialized");
    _initialized = true;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error IRModule::shutdown() {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    stopJammer();
    rmt_driver_uninstall(RMT_CHANNEL_0);
    rmt_driver_uninstall(RMT_CHANNEL_1);

    _initialized = false;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

bool IRModule::isSupported() const {
    // IR is supported if we have GPIO pins available
    return true;
}

Core::Error IRModule::sendCode(const IRCode& code) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (!code.rawTimings.empty()) {
        return sendRaw(code.rawTimings, _frequency);
    }

    // Send based on protocol
    if (code.protocol == "NEC") {
        return sendNEC(code.address, code.command);
    } else         if (code.protocol == "RC5") {
            return sendRC5(code.address, code.command);
        } else if (code.protocol == "RC5X") {
            return sendRC5X(code.address, code.command);
        } else if (code.protocol == "RC6") {
            return sendRC6(code.address, code.command);
        } else if (code.protocol == "SIRC") {
            return sendSIRC(code.address, code.command);
        } else if (code.protocol == "SIRC15") {
            return sendSIRC15(code.address, code.command);
        } else if (code.protocol == "SIRC20") {
            return sendSIRC20(code.address, code.command);
        } else if (code.protocol == "Samsung32") {
            return sendSamsung32(code.address, code.command);
        } else if (code.protocol == "Sony") {
            return sendSony(code.address, code.command, 12);
        } else if (code.protocol == "NECext") {
            return sendNECext(code.address, code.command);
        }

    return Core::Error(Core::ErrorCode::INVALID_PARAMETER, "Unsupported protocol");
}

Core::Error IRModule::sendRaw(const std::vector<uint16_t>& timings, uint32_t frequency) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    // Convert timings to RMT items
    size_t num_items = timings.size();
    rmt_item32_t* items = new rmt_item32_t[num_items];

    for (size_t i = 0; i < num_items; ++i) {
        items[i].level0 = (i % 2 == 0) ? 1 : 0;  // Alternate high/low
        items[i].duration0 = timings[i] * 80 / 1000;  // Convert to RMT ticks (80MHz / 1000)
        items[i].level1 = 0;
        items[i].duration1 = 0;
    }

    // Send via RMT
    rmt_write_items(RMT_CHANNEL_0, items, num_items, true);
    delete[] items;

    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error IRModule::receiveCode(IRCode& code, uint32_t timeout) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    rmt_item32_t* items = new rmt_item32_t[1024];
    size_t num_items = 0;

    rmt_rx_start(RMT_CHANNEL_1, true);
    delay(timeout);
    rmt_rx_stop(RMT_CHANNEL_1);

    RingbufHandle_t rb = nullptr;
    rmt_get_ringbuf_handle(RMT_CHANNEL_1, &rb);
    if (rb == nullptr) {
        delete[] items;
        return Core::Error(Core::ErrorCode::OPERATION_FAILED, "No IR signal received");
    }

    // TODO: Decode received signal based on protocol
    code.protocol = "RAW";
    code.rawTimings.clear();

    delete[] items;
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error IRModule::recordCode(IRCode& code, uint32_t timeout) {
    return receiveCode(code, timeout);
}

Core::Error IRModule::tvBGone(bool region) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    // TV-B-Gone: Send multiple power-off codes to turn off any TV
    // Common TV power codes (NEC format)
    struct TVCode {
        uint32_t address;
        uint32_t command;
    };
    
    // US/Asia codes (NEC format)
    TVCode usCodes[] = {
        {0x00, 0x0C},  // Generic power
        {0x20, 0xDF},  // Samsung
        {0x40, 0xBF},  // LG
        {0x10, 0xEF},  // Sony
        {0x08, 0xF7},  // Panasonic
        {0x04, 0xFB},  // Sharp
        {0x02, 0xFD},  // Toshiba
        {0x01, 0xFE},  // Philips
    };
    
    // EU codes (RC5 format)
    TVCode euCodes[] = {
        {0x00, 0x0C},  // Generic power
        {0x11, 0x0C},  // Philips
        {0x10, 0x0C},  // Samsung
        {0x12, 0x0C},  // LG
    };
    
    Serial.println("[IR] TV-B-Gone: Sending power-off codes...");
    
    if (!region) {
        // US/Asia region - send NEC codes
        for (size_t i = 0; i < sizeof(usCodes) / sizeof(usCodes[0]); ++i) {
            sendNEC(usCodes[i].address, usCodes[i].command);
            delay(100);
        }
    } else {
        // EU region - send RC5 codes
        for (size_t i = 0; i < sizeof(euCodes) / sizeof(euCodes[0]); ++i) {
            sendRC5(euCodes[i].address, euCodes[i].command);
            delay(100);
        }
    }
    
    Serial.println("[IR] TV-B-Gone: All codes sent");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error IRModule::startJammer(uint32_t frequency) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    if (_jamming) {
        return Core::Error(Core::ErrorCode::ALREADY_INITIALIZED);
    }

    _frequency = frequency;
    _jamming = true;

    // Send continuous carrier signal
    // This is done by sending a continuous high signal
    Serial.printf("[IR] Jammer started at %d Hz\n", frequency);
    
    // TODO: Implement continuous jamming signal
    
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error IRModule::stopJammer() {
    if (!_jamming) {
        return Core::Error(Core::ErrorCode::SUCCESS);
    }

    _jamming = false;
    Serial.println("[IR] Jammer stopped");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error IRModule::sendNEC(uint32_t address, uint32_t command) {
    // NEC protocol: 9ms burst, 4.5ms space, then address (16 bits), command (16 bits)
    std::vector<uint16_t> timings;
    
    timings.push_back(9000);   // Start burst
    timings.push_back(4500);   // Start space
    
    // Send address (LSB first)
    for (int i = 0; i < 16; ++i) {
        if (address & (1 << i)) {
            timings.push_back(560);   // 1 burst
            timings.push_back(1690);  // 1 space
        } else {
            timings.push_back(560);   // 0 burst
            timings.push_back(560);    // 0 space
        }
    }
    
    // Send command (LSB first)
    for (int i = 0; i < 16; ++i) {
        if (command & (1 << i)) {
            timings.push_back(560);
            timings.push_back(1690);
        } else {
            timings.push_back(560);
            timings.push_back(560);
        }
    }
    
    timings.push_back(560);  // Final burst
    
    return sendRaw(timings, 38000);
}

Core::Error IRModule::sendRC5(uint32_t address, uint32_t command) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    // RC5 protocol: 36kHz carrier, 889us unit time
    // Format: Start bit (1), Toggle bit, Address (5 bits), Command (6 bits)
    std::vector<uint16_t> timings;
    
    // Start bit (always 1)
    timings.push_back(889);
    timings.push_back(889);
    
    // Toggle bit (alternates, use 0 for now)
    timings.push_back(889);
    timings.push_back(889);
    
    // Address (5 bits, MSB first)
    for (int i = 4; i >= 0; --i) {
        if (address & (1 << i)) {
            timings.push_back(889);
            timings.push_back(889);
        } else {
            timings.push_back(889);
            timings.push_back(889);
        }
    }
    
    // Command (6 bits, MSB first)
    for (int i = 5; i >= 0; --i) {
        if (command & (1 << i)) {
            timings.push_back(889);
            timings.push_back(889);
        } else {
            timings.push_back(889);
            timings.push_back(889);
        }
    }
    
    return sendRaw(timings, 36000);
}

Core::Error IRModule::sendRC6(uint32_t address, uint32_t command) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    // RC6 protocol: 36kHz carrier, 444us unit time
    // Format: Leader, Start bit, Toggle bit, Address (8 bits), Command (8 bits)
    std::vector<uint16_t> timings;
    
    // Leader: 2666us high, 889us low
    timings.push_back(2666);
    timings.push_back(889);
    
    // Start bit (always 1)
    timings.push_back(444);
    timings.push_back(444);
    
    // Toggle bit
    timings.push_back(444);
    timings.push_back(444);
    
    // Address (8 bits, MSB first)
    for (int i = 7; i >= 0; --i) {
        if (address & (1 << i)) {
            timings.push_back(444);
            timings.push_back(1333);  // 3x unit for 1
        } else {
            timings.push_back(444);
            timings.push_back(444);   // 1x unit for 0
        }
    }
    
    // Command (8 bits, MSB first)
    for (int i = 7; i >= 0; --i) {
        if (command & (1 << i)) {
            timings.push_back(444);
            timings.push_back(1333);
        } else {
            timings.push_back(444);
            timings.push_back(444);
        }
    }
    
    return sendRaw(timings, 36000);
}

Core::Error IRModule::sendSIRC(uint32_t address, uint32_t command) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    // SIRC (Sony) protocol: 40kHz carrier, 600us unit time
    // Format: Start (2400us), Address (7 bits), Command (5 bits)
    std::vector<uint16_t> timings;
    
    // Start pulse: 2400us high, 600us low
    timings.push_back(2400);
    timings.push_back(600);
    
    // Address (7 bits, LSB first)
    for (int i = 0; i < 7; ++i) {
        if (address & (1 << i)) {
            timings.push_back(1200);  // 2x unit for 1
            timings.push_back(600);
        } else {
            timings.push_back(600);   // 1x unit for 0
            timings.push_back(600);
        }
    }
    
    // Command (5 bits, LSB first)
    for (int i = 0; i < 5; ++i) {
        if (command & (1 << i)) {
            timings.push_back(1200);
            timings.push_back(600);
        } else {
            timings.push_back(600);
            timings.push_back(600);
        }
    }
    
    return sendRaw(timings, 40000);
}

Core::Error IRModule::sendNECext(uint32_t address, uint32_t command) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    // Extended NEC: Same as NEC but with 16-bit address and 16-bit command (no inversion)
    std::vector<uint16_t> timings;
    
    timings.push_back(9000);   // Start burst
    timings.push_back(4500);   // Start space
    
    // Send address (16 bits, LSB first, no inversion)
    for (int i = 0; i < 16; ++i) {
        if (address & (1 << i)) {
            timings.push_back(560);
            timings.push_back(1690);
        } else {
            timings.push_back(560);
            timings.push_back(560);
        }
    }
    
    // Send command (16 bits, LSB first, no inversion)
    for (int i = 0; i < 16; ++i) {
        if (command & (1 << i)) {
            timings.push_back(560);
            timings.push_back(1690);
        } else {
            timings.push_back(560);
            timings.push_back(560);
        }
    }
    
    timings.push_back(560);  // Final burst
    
    return sendRaw(timings, 38000);
}

Core::Error IRModule::sendRC5X(uint32_t address, uint32_t command) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    // RC5X (Extended RC5): Same as RC5 but with 7-bit command instead of 6-bit
    std::vector<uint16_t> timings;
    
    timings.push_back(889);  // Start bit
    timings.push_back(889);
    timings.push_back(889);  // Toggle bit
    timings.push_back(889);
    
    // Address (5 bits, MSB first)
    for (int i = 4; i >= 0; --i) {
        timings.push_back(889);
        timings.push_back((address & (1 << i)) ? 889 : 889);
    }
    
    // Command (7 bits, MSB first)
    for (int i = 6; i >= 0; --i) {
        timings.push_back(889);
        timings.push_back((command & (1 << i)) ? 889 : 889);
    }
    
    return sendRaw(timings, 36000);
}

Core::Error IRModule::sendSIRC15(uint32_t address, uint32_t command) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    // SIRC 15-bit: Start (2400us), Address (7 bits), Command (8 bits)
    std::vector<uint16_t> timings;
    
    timings.push_back(2400);  // Start pulse
    timings.push_back(600);
    
    // Address (7 bits, LSB first)
    for (int i = 0; i < 7; ++i) {
        if (address & (1 << i)) {
            timings.push_back(1200);
            timings.push_back(600);
        } else {
            timings.push_back(600);
            timings.push_back(600);
        }
    }
    
    // Command (8 bits, LSB first)
    for (int i = 0; i < 8; ++i) {
        if (command & (1 << i)) {
            timings.push_back(1200);
            timings.push_back(600);
        } else {
            timings.push_back(600);
            timings.push_back(600);
        }
    }
    
    return sendRaw(timings, 40000);
}

Core::Error IRModule::sendSIRC20(uint32_t address, uint32_t command) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    // SIRC 20-bit: Start (2400us), Address (7 bits), Command (13 bits)
    std::vector<uint16_t> timings;
    
    timings.push_back(2400);
    timings.push_back(600);
    
    // Address (7 bits, LSB first)
    for (int i = 0; i < 7; ++i) {
        if (address & (1 << i)) {
            timings.push_back(1200);
            timings.push_back(600);
        } else {
            timings.push_back(600);
            timings.push_back(600);
        }
    }
    
    // Command (13 bits, LSB first)
    for (int i = 0; i < 13; ++i) {
        if (command & (1 << i)) {
            timings.push_back(1200);
            timings.push_back(600);
        } else {
            timings.push_back(600);
            timings.push_back(600);
        }
    }
    
    return sendRaw(timings, 40000);
}

Core::Error IRModule::sendSamsung32(uint32_t address, uint32_t command) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    // Samsung 32-bit: Similar to NEC but with different timing
    std::vector<uint16_t> timings;
    
    timings.push_back(4500);   // Start burst
    timings.push_back(4500);   // Start space
    
    // Send 32-bit data (address + command combined)
    uint32_t data = (address << 16) | command;
    for (int i = 0; i < 32; ++i) {
        if (data & (1 << i)) {
            timings.push_back(560);
            timings.push_back(1690);
        } else {
            timings.push_back(560);
            timings.push_back(560);
        }
    }
    
    timings.push_back(560);  // Final burst
    
    return sendRaw(timings, 38000);
}

Core::Error IRModule::sendSony(uint32_t address, uint32_t command, uint8_t bits) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    // Sony protocol: 40kHz, variable bit length (12/15/20 bits)
    std::vector<uint16_t> timings;
    
    timings.push_back(2400);  // Start pulse
    timings.push_back(600);
    
    // Send command (bits length, LSB first)
    for (int i = 0; i < bits; ++i) {
        if (command & (1 << i)) {
            timings.push_back(1200);
            timings.push_back(600);
        } else {
            timings.push_back(600);
            timings.push_back(600);
        }
    }
    
    return sendRaw(timings, 40000);
}

Core::Error IRModule::setTXPin(uint8_t pin) {
    _txPin = pin;
    if (_initialized) {
        // Reinitialize with new pin
        shutdown();
        return initialize();
    }
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error IRModule::setRXPin(uint8_t pin) {
    _rxPin = pin;
    if (_initialized) {
        shutdown();
        return initialize();
    }
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error IRModule::setFrequency(uint32_t frequency) {
    _frequency = frequency;
    if (_initialized) {
        shutdown();
        return initialize();
    }
    return Core::Error(Core::ErrorCode::SUCCESS);
}

} // namespace Modules
} // namespace NightStrike

