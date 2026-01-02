#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace NightStrike {
namespace Utils {

/**
 * @brief String utility functions
 */

// String manipulation
std::vector<std::string> splitString(const std::string& str, char delimiter);
std::string joinString(const std::vector<std::string>& strings, const std::string& delimiter);
std::string toUpper(const std::string& str);
std::string toLower(const std::string& str);
std::string trim(const std::string& str);
bool startsWith(const std::string& str, const std::string& prefix);
bool endsWith(const std::string& str, const std::string& suffix);

// Hex conversion
std::string toHexString(const uint8_t* data, size_t length);
std::vector<uint8_t> fromHexString(const std::string& hex);

// MAC address
std::string macToString(const uint8_t* mac);
bool stringToMAC(const std::string& str, uint8_t* mac);

// IP address
std::string ipToString(const uint8_t* ip);
bool stringToIP(const std::string& str, uint8_t* ip);

} // namespace Utils
} // namespace NightStrike

