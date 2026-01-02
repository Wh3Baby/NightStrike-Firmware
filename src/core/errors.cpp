#include "core/errors.h"

namespace NightStrike {
namespace Core {

const char* getErrorMessage(ErrorCode code) {
    switch (code) {
        case ErrorCode::SUCCESS:
            return "Success";
        case ErrorCode::UNKNOWN_ERROR:
            return "Unknown error";
        case ErrorCode::INVALID_PARAMETER:
            return "Invalid parameter";
        case ErrorCode::OUT_OF_MEMORY:
            return "Out of memory";
        case ErrorCode::NOT_INITIALIZED:
            return "Not initialized";
        case ErrorCode::ALREADY_INITIALIZED:
            return "Already initialized";
        case ErrorCode::OPERATION_FAILED:
            return "Operation failed";
        case ErrorCode::TIMEOUT:
            return "Operation timeout";
        case ErrorCode::NOT_SUPPORTED:
            return "Not supported";
        case ErrorCode::STORAGE_NOT_MOUNTED:
            return "Storage not mounted";
        case ErrorCode::STORAGE_FULL:
            return "Storage full";
        case ErrorCode::FILE_NOT_FOUND:
            return "File not found";
        case ErrorCode::FILE_READ_ERROR:
            return "File read error";
        case ErrorCode::FILE_WRITE_ERROR:
            return "File write error";
        case ErrorCode::NETWORK_NOT_CONNECTED:
            return "Network not connected";
        case ErrorCode::NETWORK_CONNECTION_FAILED:
            return "Network connection failed";
        case ErrorCode::DISPLAY_NOT_INITIALIZED:
            return "Display not initialized";
        case ErrorCode::CONFIG_INVALID:
            return "Configuration invalid";
        case ErrorCode::CONFIG_NOT_FOUND:
            return "Configuration not found";
        case ErrorCode::SECURITY_INVALID_CREDENTIALS:
            return "Invalid credentials";
        case ErrorCode::SECURITY_PASSWORD_TOO_WEAK:
            return "Password too weak";
        default:
            return "Unknown error code";
    }
}

} // namespace Core
} // namespace NightStrike

