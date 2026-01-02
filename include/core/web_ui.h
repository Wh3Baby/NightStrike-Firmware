#pragma once

#include "errors.h"
#include <string>
#include <functional>
#include <map>

namespace NightStrike {
namespace Core {

/**
 * @brief Web UI for remote management
 *
 * Provides HTTP API and web interface for controlling NightStrike
 */
class WebUI {
public:
    struct Request {
        std::string method;
        std::string path;
        std::string body;
        std::map<std::string, std::string> headers;
        std::map<std::string, std::string> params;
    };

    struct Response {
        int statusCode = 200;
        std::string contentType = "application/json";
        std::string body;
        std::map<std::string, std::string> headers;
    };

    using RouteHandler = std::function<Response(const Request&)>;

    static WebUI& getInstance();

    // Initialization
    Error initialize(uint16_t port = 80);
    Error shutdown();

    // Route management
    Error addRoute(const std::string& method, const std::string& path, RouteHandler handler);

    // Status
    bool isActive() const { return _active; }
    std::string getURL() const;

private:
    WebUI() = default;
    ~WebUI() = default;
    WebUI(const WebUI&) = delete;
    WebUI& operator=(const WebUI&) = delete;

    bool _initialized = false;
    bool _active = false;
    uint16_t _port = 80;
};

} // namespace Core
} // namespace NightStrike

