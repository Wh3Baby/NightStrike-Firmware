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

    // Convert RMT items to timing vector
    std::vector<uint16_t> timings;
    for (size_t i = 0; i < num_items && i < 1024; ++i) {
        if (items[i].duration0 > 0) {
            timings.push_back(items[i].duration0 * 1000 / 80);  // Convert RMT ticks to microseconds
        }
        if (items[i].duration1 > 0) {
            timings.push_back(items[i].duration1 * 1000 / 80);
        }
    }
    
    code.rawTimings = timings;
    
    // Try to decode using different protocols
    uint32_t address = 0, command = 0;
    if (decodeNEC(timings, address, command)) {
        code.protocol = "NEC";
        code.address = address;
        code.command = command;
    } else if (decodeRC5(timings, address, command)) {
        code.protocol = "RC5";
        code.address = address;
        code.command = command;
    } else if (decodeRC6(timings, address, command)) {
        code.protocol = "RC6";
        code.address = address;
        code.command = command;
    } else if (decodeSIRC(timings, address, command, 12)) {
        code.protocol = "SIRC12";
        code.address = address;
        code.command = command;
    } else if (decodeSIRC(timings, address, command, 15)) {
        code.protocol = "SIRC15";
        code.address = address;
        code.command = command;
    } else if (decodeSIRC(timings, address, command, 20)) {
        code.protocol = "SIRC20";
        code.address = address;
        code.command = command;
    } else {
        code.protocol = "RAW";
        code.address = 0;
        code.command = 0;
    }

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

    // Reconfigure RMT for continuous jamming
    rmt_driver_uninstall(RMT_CHANNEL_0);
    rmt_config_t rmt_tx_config = RMT_DEFAULT_CONFIG_TX((gpio_num_t)_txPin, RMT_CHANNEL_0);
    rmt_tx_config.tx_config.carrier_en = true;
    rmt_tx_config.tx_config.carrier_freq_hz = frequency;
    rmt_tx_config.tx_config.carrier_duty_percent = 50;
    rmt_config(&rmt_tx_config);
    rmt_driver_install(RMT_CHANNEL_0, 0, 0);

    // Create continuous jamming signal (alternating high/low)
    // This creates a continuous carrier wave
    rmt_item32_t jammer_items[2];
    jammer_items[0].level0 = 1;
    jammer_items[0].duration0 = 13;  // ~50% duty cycle at 38kHz (80MHz / 38kHz / 2 ≈ 1053 ticks, use 13 for faster)
    jammer_items[0].level1 = 0;
    jammer_items[0].duration1 = 13;
    jammer_items[1].level0 = 1;
    jammer_items[1].duration0 = 0;  // End marker
    jammer_items[1].duration1 = 0;

    // Start continuous transmission in a loop
    xTaskCreatePinnedToCore(
        [](void* param) {
            IRModule* module = static_cast<IRModule*>(param);
            rmt_item32_t items[2];
            items[0].level0 = 1;
            items[0].duration0 = 13;
            items[0].level1 = 0;
            items[0].duration1 = 13;
            items[1].level0 = 1;
            items[1].duration0 = 0;
            items[1].duration1 = 0;
            
            while (module->_jamming) {
                rmt_write_items(RMT_CHANNEL_0, items, 2, true);
                vTaskDelay(1 / portTICK_PERIOD_MS);
            }
            vTaskDelete(NULL);
        },
        "IRJammer",
        2048,
        this,
        1,
        &_jammerTaskHandle,
        1
    );

    Serial.printf("[IR] Jammer started at %d Hz\n", frequency);
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error IRModule::stopJammer() {
    if (!_jamming) {
        return Core::Error(Core::ErrorCode::SUCCESS);
    }

    _jamming = false;
    
    // Wait for task to finish
    if (_jammerTaskHandle) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        if (_jammerTaskHandle) {
            vTaskDelete(_jammerTaskHandle);
            _jammerTaskHandle = nullptr;
        }
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

// Decoding helper functions
bool IRModule::decodeNEC(const std::vector<uint16_t>& timings, uint32_t& address, uint32_t& command) {
    // NEC: 9ms header, 4.5ms pause, then 32 bits (address + command)
    if (timings.size() < 70) return false;
    
    // Check for header (9000us high, 4500us low)
    if (timings[0] < 8000 || timings[0] > 10000 || timings[1] < 4000 || timings[1] > 5000) {
        return false;
    }
    
    uint32_t data = 0;
    size_t bitIndex = 0;
    
    // Decode 32 bits (16 address + 16 command)
    for (size_t i = 2; i < timings.size() && bitIndex < 32; i += 2) {
        if (i + 1 >= timings.size()) break;
        
        uint16_t mark = timings[i];
        uint16_t space = timings[i + 1];
        
        if (mark > 500 && mark < 700) {  // 560us mark
            if (space > 500 && space < 700) {  // 560us space = 0
                // bit is 0
            } else if (space > 1600 && space < 1800) {  // 1690us space = 1
                data |= (1UL << bitIndex);
            } else {
                return false;  // Invalid timing
            }
            bitIndex++;
        } else {
            break;  // End of data
        }
    }
    
    if (bitIndex == 32) {
        address = data & 0xFFFF;
        command = (data >> 16) & 0xFFFF;
        return true;
    }
    
    return false;
}

bool IRModule::decodeRC5(const std::vector<uint16_t>& timings, uint32_t& address, uint32_t& command) {
    // RC5: Manchester encoding, 889us bit time
    if (timings.size() < 20) return false;
    
    uint32_t data = 0;
    size_t bitIndex = 0;
    
    // Skip first two bits (start bits)
    size_t startIdx = 2;
    
    for (size_t i = startIdx; i < timings.size() && bitIndex < 14; i++) {
        uint16_t timing = timings[i];
        
        if (timing > 400 && timing < 600) {  // ~500us = 0
            // bit is 0
        } else if (timing > 1200 && timing < 1400) {  // ~1300us = 1
            data |= (1UL << bitIndex);
        } else {
            break;
        }
        bitIndex++;
    }
    
    if (bitIndex >= 12) {
        address = (data >> 6) & 0x1F;  // 5 bits
        command = data & 0x3F;  // 6 bits
        return true;
    }
    
    return false;
}

bool IRModule::decodeRC6(const std::vector<uint16_t>& timings, uint32_t& address, uint32_t& command) {
    // RC6: Similar to RC5 but with different timing
    if (timings.size() < 20) return false;
    
    // RC6 has a leader: 2666us mark, 889us space
    if (timings[0] < 2400 || timings[0] > 2900 || timings[1] < 800 || timings[1] > 1000) {
        return false;
    }
    
    uint32_t data = 0;
    size_t bitIndex = 0;
    
    // Decode from bit 2
    for (size_t i = 2; i < timings.size() && bitIndex < 20; i++) {
        uint16_t timing = timings[i];
        
        if (timing > 400 && timing < 600) {  // ~444us = 0
            // bit is 0
        } else if (timing > 1200 && timing < 1400) {  // ~1333us = 1
            data |= (1UL << bitIndex);
        } else {
            break;
        }
        bitIndex++;
    }
    
    if (bitIndex >= 16) {
        address = (data >> 8) & 0xFF;
        command = data & 0xFF;
        return true;
    }
    
    return false;
}

bool IRModule::decodeSIRC(const std::vector<uint16_t>& timings, uint32_t& address, uint32_t& command, uint8_t bits) {
    // SIRC: 2400us header, then data
    if (timings.size() < 10) return false;
    
    // Check for header (2400us mark, 600us space)
    if (timings[0] < 2200 || timings[0] > 2600 || timings[1] < 500 || timings[1] > 700) {
        return false;
    }
    
    uint32_t data = 0;
    size_t bitIndex = 0;
    size_t expectedBits = (bits == 12) ? 12 : (bits == 15) ? 15 : 20;
    
    // Decode bits (LSB first)
    for (size_t i = 2; i < timings.size() && bitIndex < expectedBits; i += 2) {
        if (i + 1 >= timings.size()) break;
        
        uint16_t mark = timings[i];
        uint16_t space = timings[i + 1];
        
        if (mark > 500 && mark < 700) {  // ~600us mark
            if (space > 500 && space < 700) {  // ~600us space = 0
                // bit is 0
            } else if (space > 1100 && space < 1300) {  // ~1200us space = 1
                data |= (1UL << bitIndex);
            } else {
                return false;
            }
            bitIndex++;
        } else {
            break;
        }
    }
    
    if (bitIndex == expectedBits) {
        if (bits == 12) {
            address = (data >> 7) & 0x1F;  // 5 bits
            command = data & 0x7F;  // 7 bits
        } else if (bits == 15) {
            address = (data >> 7) & 0xFF;  // 8 bits
            command = data & 0x7F;  // 7 bits
        } else {  // 20 bits
            address = (data >> 8) & 0xFF;  // 8 bits
            command = data & 0xFF;  // 8 bits
        }
        return true;
    }
    
    return false;
}

} // namespace Modules
} // namespace NightStrike

