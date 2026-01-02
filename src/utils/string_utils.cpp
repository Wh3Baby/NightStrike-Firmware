#include "utils/string_utils.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace NightStrike {
namespace Utils {

std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

std::string joinString(const std::vector<std::string>& strings, const std::string& delimiter) {
    if (strings.empty()) {
        return "";
    }

    std::string result = strings[0];
    for (size_t i = 1; i < strings.size(); ++i) {
        result += delimiter + strings[i];
    }

    return result;
}

std::string toHexString(const uint8_t* data, size_t length) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    for (size_t i = 0; i < length; ++i) {
        ss << std::setw(2) << static_cast<int>(data[i]);
        if (i < length - 1) {
            ss << ":";
        }
    }

    return ss.str();
}

std::vector<uint8_t> fromHexString(const std::string& hex) {
    std::vector<uint8_t> result;
    std::string cleanHex = hex;

    // Remove colons and spaces
    cleanHex.erase(std::remove(cleanHex.begin(), cleanHex.end(), ':'), cleanHex.end());
    cleanHex.erase(std::remove(cleanHex.begin(), cleanHex.end(), ' '), cleanHex.end());

    if (cleanHex.length() % 2 != 0) {
        return result;  // Invalid hex string
    }

    for (size_t i = 0; i < cleanHex.length(); i += 2) {
        std::string byteStr = cleanHex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoul(byteStr, nullptr, 16));
        result.push_back(byte);
    }

    return result;
}

std::string macToString(const uint8_t* mac) {
    char buffer[18];
    snprintf(buffer, sizeof(buffer), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return std::string(buffer);
}

bool stringToMAC(const std::string& str, uint8_t* mac) {
    return sscanf(str.c_str(), "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx",
                   &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == 6;
}

std::string ipToString(const uint8_t* ip) {
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return std::string(buffer);
}

bool stringToIP(const std::string& str, uint8_t* ip) {
    return sscanf(str.c_str(), "%hhu.%hhu.%hhu.%hhu",
                   &ip[0], &ip[1], &ip[2], &ip[3]) == 4;
}

std::string toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) {
        return "";
    }

    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

bool startsWith(const std::string& str, const std::string& prefix) {
    return str.length() >= prefix.length() &&
           str.compare(0, prefix.length(), prefix) == 0;
}

bool endsWith(const std::string& str, const std::string& suffix) {
    return str.length() >= suffix.length() &&
           str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

} // namespace Utils
} // namespace NightStrike

