#pragma once

#include "errors.h"
#include <string>

namespace NightStrike {
namespace Core {

/**
 * @brief Base interface for all modules
 *
 * Every module must implement this interface for consistency
 */
class IModule {
public:
    virtual ~IModule() = default;

    /**
     * @brief Get module name
     */
    virtual const char* getName() const = 0;

    /**
     * @brief Initialize the module
     */
    virtual Error initialize() = 0;

    /**
     * @brief Shutdown the module
     */
    virtual Error shutdown() = 0;

    /**
     * @brief Check if module is initialized
     */
    virtual bool isInitialized() const = 0;

    /**
     * @brief Check if module is supported on current hardware
     */
    virtual bool isSupported() const = 0;

protected:
    bool _initialized = false;
};

} // namespace Core
} // namespace NightStrike

