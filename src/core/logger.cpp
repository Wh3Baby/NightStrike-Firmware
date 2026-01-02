#include "core/logger.h"
#include <Arduino.h>
#include <cstdarg>

namespace NightStrike {
namespace Core {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::setLevel(LogLevel level) {
    _level = level;
}

LogLevel Logger::getLevel() const {
    return _level;
}

void Logger::log(LogLevel level, const char* message) {
    if (level < _level) {
        return;
    }

    const char* levelStr = "";
    switch (level) {
        case LogLevel::DEBUG: levelStr = "DEBUG"; break;
        case LogLevel::INFO: levelStr = "INFO"; break;
        case LogLevel::WARN: levelStr = "WARN"; break;
        case LogLevel::ERROR: levelStr = "ERROR"; break;
        case LogLevel::FATAL: levelStr = "FATAL"; break;
    }

    Serial.printf("[%s] %s\n", levelStr, message);
}

void Logger::debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    log(LogLevel::DEBUG, buffer);
}

void Logger::info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    log(LogLevel::INFO, buffer);
}

void Logger::warn(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    log(LogLevel::WARN, buffer);
}

void Logger::error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    log(LogLevel::ERROR, buffer);
}

void Logger::fatal(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    log(LogLevel::FATAL, buffer);
}

} // namespace Core
} // namespace NightStrike

