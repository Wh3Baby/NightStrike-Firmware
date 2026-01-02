#pragma once

#include <cstdint>

namespace NightStrike {
namespace Core {

/**
 * @brief Error codes for NightStrike firmware
 *
 * Comprehensive error code system for better error handling
 */
enum class ErrorCode : uint16_t {
    // Success
    SUCCESS = 0x0000,

    // General errors (0x0001-0x00FF)
    UNKNOWN_ERROR = 0x0001,
    INVALID_PARAMETER = 0x0002,
    OUT_OF_MEMORY = 0x0003,
    NOT_INITIALIZED = 0x0004,
    ALREADY_INITIALIZED = 0x0005,
    OPERATION_FAILED = 0x0006,
    TIMEOUT = 0x0007,
    NOT_SUPPORTED = 0x0008,

    // Storage errors (0x0100-0x01FF)
    STORAGE_NOT_MOUNTED = 0x0100,
    STORAGE_FULL = 0x0101,
    FILE_NOT_FOUND = 0x0102,
    FILE_READ_ERROR = 0x0103,
    FILE_WRITE_ERROR = 0x0104,
    FILE_DELETE_ERROR = 0x0105,

    // Network errors (0x0200-0x02FF)
    NETWORK_NOT_CONNECTED = 0x0200,
    NETWORK_CONNECTION_FAILED = 0x0201,
    NETWORK_TIMEOUT = 0x0202,
    NETWORK_INVALID_CREDENTIALS = 0x0203,

    // Display errors (0x0300-0x03FF)
    DISPLAY_NOT_INITIALIZED = 0x0300,
    DISPLAY_INIT_FAILED = 0x0301,

    // Module errors (0x0400-0x04FF)
    MODULE_NOT_LOADED = 0x0400,
    MODULE_INIT_FAILED = 0x0401,
    MODULE_NOT_SUPPORTED = 0x0402,

    // Configuration errors (0x0500-0x05FF)
    CONFIG_INVALID = 0x0500,
    CONFIG_NOT_FOUND = 0x0501,
    CONFIG_SAVE_FAILED = 0x0502,
    CONFIG_LOAD_FAILED = 0x0503,

    // Security errors (0x0600-0x06FF)
    SECURITY_INVALID_CREDENTIALS = 0x0600,
    SECURITY_UNAUTHORIZED = 0x0601,
    SECURITY_PASSWORD_TOO_WEAK = 0x0602,
};

/**
 * @brief Error result type
 */
struct Error {
    ErrorCode code;
    const char* message;

    Error(ErrorCode c = ErrorCode::SUCCESS, const char* msg = nullptr)
        : code(c), message(msg) {}

    bool isSuccess() const { return code == ErrorCode::SUCCESS; }
    bool isError() const { return !isSuccess(); }

    operator bool() const { return isSuccess(); }
};

/**
 * @brief Get error message string
 */
const char* getErrorMessage(ErrorCode code);

} // namespace Core
} // namespace NightStrike

