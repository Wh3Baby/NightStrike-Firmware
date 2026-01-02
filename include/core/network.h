#pragma once

#include "errors.h"
#include <string>

namespace NightStrike {
namespace Core {

/**
 * @brief Network stack abstraction
 */
class Network {
public:
    static Network& getInstance();

    // Initialization
    Error initialize();
    Error shutdown();

    // MAC address
    std::string getMACAddress() const;
    Error setMACAddress(const std::string& mac);

    bool isInitialized() const { return _initialized; }

private:
    Network() = default;
    ~Network() = default;
    Network(const Network&) = delete;
    Network& operator=(const Network&) = delete;

    bool _initialized = false;
};

} // namespace Core
} // namespace NightStrike

