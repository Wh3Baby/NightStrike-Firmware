#pragma once

#include <cstdint>

namespace NightStrike {
namespace Core {

enum class LogLevel : uint8_t {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    FATAL = 4
};

/**
 * @brief Logging system
 */
class Logger {
public:
    static Logger& getInstance();

    void setLevel(LogLevel level);
    LogLevel getLevel() const;

    void log(LogLevel level, const char* message);
    void debug(const char* format, ...);
    void info(const char* format, ...);
    void warn(const char* format, ...);
    void error(const char* format, ...);
    void fatal(const char* format, ...);

private:
    Logger() : _level(LogLevel::INFO) {}
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    LogLevel _level;
};

// Convenience macros
#define LOG_DEBUG(...) NightStrike::Core::Logger::getInstance().debug(__VA_ARGS__)
#define LOG_INFO(...) NightStrike::Core::Logger::getInstance().info(__VA_ARGS__)
#define LOG_WARN(...) NightStrike::Core::Logger::getInstance().warn(__VA_ARGS__)
#define LOG_ERROR(...) NightStrike::Core::Logger::getInstance().error(__VA_ARGS__)
#define LOG_FATAL(...) NightStrike::Core::Logger::getInstance().fatal(__VA_ARGS__)

} // namespace Core
} // namespace NightStrike

