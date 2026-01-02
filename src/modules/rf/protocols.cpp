#include "modules/rf/protocols.h"
#include <algorithm>

namespace NightStrike {
namespace Modules {

// Came Protocol Implementation
std::vector<int> CameProtocol::encode(const std::vector<uint8_t>& data) {
    std::vector<int> timings;
    
    // Pilot period
    auto pilot = getPilotPeriod();
    timings.insert(timings.end(), pilot.begin(), pilot.end());
    
    // Encode each byte
    for (uint8_t byte : data) {
        for (int i = 7; i >= 0; --i) {
            bool bit = (byte >> i) & 1;
            if (bit) {
                timings.push_back(-640);
                timings.push_back(320);
            } else {
                timings.push_back(-320);
                timings.push_back(640);
            }
        }
    }
    
    return timings;
}

std::vector<uint8_t> CameProtocol::decode(const std::vector<int>& timings) {
    std::vector<uint8_t> data;
    
    // Skip pilot period
    size_t start = 2;  // After pilot
    
    // Decode bits
    uint8_t byte = 0;
    int bitPos = 7;
    
    for (size_t i = start; i < timings.size() - 1; i += 2) {
        int low = timings[i];
        int high = timings[i + 1];
        
        // Determine bit based on timing
        bool bit = (abs(low) > abs(high));  // -640/320 = 1, -320/640 = 0
        
        if (bit) {
            byte |= (1 << bitPos);
        }
        
        bitPos--;
        if (bitPos < 0) {
            data.push_back(byte);
            byte = 0;
            bitPos = 7;
        }
    }
    
    return data;
}

// Linear Protocol Implementation
std::vector<int> LinearProtocol::encode(const std::vector<uint8_t>& data) {
    std::vector<int> timings;
    
    // Linear protocol: 0 = short/short, 1 = long/short
    for (uint8_t byte : data) {
        for (int i = 7; i >= 0; --i) {
            bool bit = (byte >> i) & 1;
            if (bit) {
                timings.push_back(-400);
                timings.push_back(200);
            } else {
                timings.push_back(-200);
                timings.push_back(200);
            }
        }
    }
    
    return timings;
}

std::vector<uint8_t> LinearProtocol::decode(const std::vector<int>& timings) {
    std::vector<uint8_t> data;
    
    uint8_t byte = 0;
    int bitPos = 7;
    
    for (size_t i = 0; i < timings.size() - 1; i += 2) {
        int low = timings[i];
        bool bit = (abs(low) > 300);  // Long pulse = 1
        
        if (bit) {
            byte |= (1 << bitPos);
        }
        
        bitPos--;
        if (bitPos < 0) {
            data.push_back(byte);
            byte = 0;
            bitPos = 7;
        }
    }
    
    return data;
}

// Holtek Protocol Implementation
std::vector<int> HoltekProtocol::encode(const std::vector<uint8_t>& data) {
    std::vector<int> timings;
    
    // Holtek protocol encoding
    for (uint8_t byte : data) {
        for (int i = 7; i >= 0; --i) {
            bool bit = (byte >> i) & 1;
            if (bit) {
                timings.push_back(-500);
                timings.push_back(250);
            } else {
                timings.push_back(-250);
                timings.push_back(500);
            }
        }
    }
    
    return timings;
}

std::vector<uint8_t> HoltekProtocol::decode(const std::vector<int>& timings) {
    std::vector<uint8_t> data;
    
    uint8_t byte = 0;
    int bitPos = 7;
    
    for (size_t i = 0; i < timings.size() - 1; i += 2) {
        int low = timings[i];
        int high = timings[i + 1];
        bool bit = (abs(low) > abs(high));
        
        if (bit) {
            byte |= (1 << bitPos);
        }
        
        bitPos--;
        if (bitPos < 0) {
            data.push_back(byte);
            byte = 0;
            bitPos = 7;
        }
    }
    
    return data;
}

// NiceFlo Protocol Implementation
std::vector<int> NiceFloProtocol::encode(const std::vector<uint8_t>& data) {
    std::vector<int> timings;
    
    // NiceFlo protocol encoding
    for (uint8_t byte : data) {
        for (int i = 7; i >= 0; --i) {
            bool bit = (byte >> i) & 1;
            timings.push_back(bit ? -600 : -300);
            timings.push_back(bit ? 300 : 600);
        }
    }
    
    return timings;
}

std::vector<uint8_t> NiceFloProtocol::decode(const std::vector<int>& timings) {
    std::vector<uint8_t> data;
    
    uint8_t byte = 0;
    int bitPos = 7;
    
    for (size_t i = 0; i < timings.size() - 1; i += 2) {
        bool bit = (abs(timings[i]) > 400);
        
        if (bit) {
            byte |= (1 << bitPos);
        }
        
        bitPos--;
        if (bitPos < 0) {
            data.push_back(byte);
            byte = 0;
            bitPos = 7;
        }
    }
    
    return data;
}

// Chamberlain Protocol Implementation
std::vector<int> ChamberlainProtocol::encode(const std::vector<uint8_t>& data) {
    std::vector<int> timings;
    
    // Chamberlain protocol encoding
    for (uint8_t byte : data) {
        for (int i = 7; i >= 0; --i) {
            bool bit = (byte >> i) & 1;
            timings.push_back(bit ? -700 : -350);
            timings.push_back(bit ? 350 : 700);
        }
    }
    
    return timings;
}

std::vector<uint8_t> ChamberlainProtocol::decode(const std::vector<int>& timings) {
    std::vector<uint8_t> data;
    
    uint8_t byte = 0;
    int bitPos = 7;
    
    for (size_t i = 0; i < timings.size() - 1; i += 2) {
        bool bit = (abs(timings[i]) > 500);
        
        if (bit) {
            byte |= (1 << bitPos);
        }
        
        bitPos--;
        if (bitPos < 0) {
            data.push_back(byte);
            byte = 0;
            bitPos = 7;
        }
    }
    
    return data;
}

// Liftmaster Protocol Implementation
std::vector<int> LiftmasterProtocol::encode(const std::vector<uint8_t>& data) {
    std::vector<int> timings;
    
    // Liftmaster protocol encoding
    for (uint8_t byte : data) {
        for (int i = 7; i >= 0; --i) {
            bool bit = (byte >> i) & 1;
            timings.push_back(bit ? -800 : -400);
            timings.push_back(bit ? 400 : 800);
        }
    }
    
    return timings;
}

std::vector<uint8_t> LiftmasterProtocol::decode(const std::vector<int>& timings) {
    std::vector<uint8_t> data;
    
    uint8_t byte = 0;
    int bitPos = 7;
    
    for (size_t i = 0; i < timings.size() - 1; i += 2) {
        bool bit = (abs(timings[i]) > 600);
        
        if (bit) {
            byte |= (1 << bitPos);
        }
        
        bitPos--;
        if (bitPos < 0) {
            data.push_back(byte);
            byte = 0;
            bitPos = 7;
        }
    }
    
    return data;
}

// Ansonic Protocol Implementation
std::vector<int> AnsonicProtocol::encode(const std::vector<uint8_t>& data) {
    std::vector<int> timings;
    
    // Ansonic protocol encoding
    for (uint8_t byte : data) {
        for (int i = 7; i >= 0; --i) {
            bool bit = (byte >> i) & 1;
            timings.push_back(bit ? -450 : -225);
            timings.push_back(bit ? 225 : 450);
        }
    }
    
    return timings;
}

std::vector<uint8_t> AnsonicProtocol::decode(const std::vector<int>& timings) {
    std::vector<uint8_t> data;
    
    uint8_t byte = 0;
    int bitPos = 7;
    
    for (size_t i = 0; i < timings.size() - 1; i += 2) {
        bool bit = (abs(timings[i]) > 300);
        
        if (bit) {
            byte |= (1 << bitPos);
        }
        
        bitPos--;
        if (bitPos < 0) {
            data.push_back(byte);
            byte = 0;
            bitPos = 7;
        }
    }
    
    return data;
}

} // namespace Modules
} // namespace NightStrike

