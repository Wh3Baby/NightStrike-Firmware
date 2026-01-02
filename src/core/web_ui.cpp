#include "core/web_ui.h"
#include "core/system.h"
#include "core/storage.h"
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <SD.h>
#include <ArduinoJson.h>

namespace NightStrike {
namespace Core {

WebUI* g_webUIInstance = nullptr;
AsyncWebServer* g_webServer = nullptr;

WebUI& WebUI::getInstance() {
    if (!g_webUIInstance) {
        g_webUIInstance = new WebUI();
    }
    return *g_webUIInstance;
}

Error WebUI::initialize(uint16_t port) {
    if (_initialized) {
        return Error(ErrorCode::ALREADY_INITIALIZED);
    }

    _port = port;

    if (!g_webServer) {
        g_webServer = new AsyncWebServer(port);
    }

    // Default routes
    g_webServer->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/html", R"html(
<!DOCTYPE html>
<html>
<head>
    <title>NightStrike Control Panel</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; background: #000; color: #0f0; padding: 20px; }
        .container { max-width: 800px; margin: 0 auto; }
        h1 { color: #0f0; text-shadow: 0 0 10px #0f0; }
        .module { background: #111; border: 1px solid #0f0; padding: 15px; margin: 10px 0; }
        button { background: #0f0; color: #000; border: none; padding: 10px 20px; cursor: pointer; }
        button:hover { background: #0a0; }
        .status { color: #0f0; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌑 NightStrike Control Panel</h1>
        <div class="module">
            <h2>System Status</h2>
            <p class="status" id="status">Loading...</p>
        </div>
        <div class="module">
            <h2>WiFi Module</h2>
            <button onclick="scanWiFi()">Scan Networks</button>
            <button onclick="startAP()">Start AP</button>
            <div id="wifiResults"></div>
        </div>
        <div class="module">
            <h2>BLE Module</h2>
            <button onclick="scanBLE()">Scan BLE</button>
            <button onclick="spamBLE()">Start BLE Spam</button>
        </div>
    </div>
    <script>
        function updateStatus() {
            fetch('/api/status').then(r => r.json()).then(data => {
                document.getElementById('status').innerHTML =
                    'Free Heap: ' + data.freeHeap + ' bytes<br>' +
                    'Uptime: ' + data.uptime + ' ms';
            });
        }
        function scanWiFi() {
            fetch('/api/wifi/scan').then(r => r.json()).then(data => {
                document.getElementById('wifiResults').innerHTML =
                    'Found ' + data.count + ' networks';
            });
        }
        function startAP() {
            fetch('/api/wifi/ap/start', {method: 'POST'}).then(r => r.json());
        }
        function scanBLE() {
            fetch('/api/ble/scan').then(r => r.json());
        }
        function spamBLE() {
            fetch('/api/ble/spam', {method: 'POST'}).then(r => r.json());
        }
        setInterval(updateStatus, 1000);
        updateStatus();
    </script>
</body>
</html>
        )html");
    });

    // API routes
    g_webServer->on("/api/status", HTTP_GET, [](AsyncWebServerRequest* request) {
        using namespace NightStrike::Core;
        auto& system = System::getInstance();
        auto info = system.getSystemInfo();

        String json = "{";
        json += "\"freeHeap\":" + String(info.freeHeap) + ",";
        json += "\"totalHeap\":" + String(info.totalHeap) + ",";
        json += "\"uptime\":" + String(millis());
        json += "}";

        request->send(200, "application/json", json);
    });

    // Storage API - LittleFS Manager
    g_webServer->on("/api/storage/littlefs/list", HTTP_GET, [](AsyncWebServerRequest* request) {
        using namespace NightStrike::Core;
        auto& storage = Storage::getInstance();
        std::vector<std::string> files;
        std::string path = "/";
        
        if (request->hasParam("path")) {
            path = request->getParam("path")->value().c_str();
        }
        
        Error err = storage.listFiles(path, files, false);
        
        if (err.isError()) {
            request->send(500, "application/json", "{\"error\":\"Failed to list files\"}");
            return;
        }
        
        String json = "{\"path\":\"" + String(path.c_str()) + "\",\"files\":[";
        for (size_t i = 0; i < files.size(); ++i) {
            if (i > 0) json += ",";
            json += "\"" + String(files[i].c_str()) + "\"";
        }
        json += "]}";
        request->send(200, "application/json", json);
    });

    g_webServer->on("/api/storage/littlefs/info", HTTP_GET, [](AsyncWebServerRequest* request) {
        String json = "{";
        json += "\"mounted\":" + String(LittleFS.begin() ? "true" : "false");
        json += "}";
        request->send(200, "application/json", json);
    });

    // Storage API - SD Card Manager
    g_webServer->on("/api/storage/sdcard/list", HTTP_GET, [](AsyncWebServerRequest* request) {
        using namespace NightStrike::Core;
        auto& storage = Storage::getInstance();
        std::vector<std::string> files;
        std::string path = "/";
        
        if (request->hasParam("path")) {
            path = request->getParam("path")->value().c_str();
        }
        
        Error err = storage.listFiles(path, files, true);
        
        if (err.isError()) {
            request->send(500, "application/json", "{\"error\":\"Failed to list files\"}");
            return;
        }
        
        String json = "{\"path\":\"" + String(path.c_str()) + "\",\"files\":[";
        for (size_t i = 0; i < files.size(); ++i) {
            if (i > 0) json += ",";
            json += "\"" + String(files[i].c_str()) + "\"";
        }
        json += "]}";
        request->send(200, "application/json", json);
    });

    g_webServer->on("/api/storage/sdcard/info", HTTP_GET, [](AsyncWebServerRequest* request) {
        using namespace NightStrike::Core;
        auto& storage = Storage::getInstance();
        String json = "{";
        json += "\"mounted\":" + String(storage.isSDCardMounted() ? "true" : "false");
        if (storage.isSDCardMounted()) {
            uint64_t freeSpace = storage.getFreeSpace(true);
            json += ",\"freeSpace\":" + String(freeSpace);
        }
        json += "}";
        request->send(200, "application/json", json);
    });

    // File upload endpoint
    g_webServer->on("/api/storage/upload", HTTP_POST, [](AsyncWebServerRequest* request) {
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    }, [](AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
        using namespace NightStrike::Core;
        static File uploadFile;
        static std::string uploadPath;
        
        if (index == 0) {
            String storage = request->getParam("storage", true)->value();
            String path = request->getParam("path", true)->value();
            uploadPath = path.c_str();
            
            if (storage == "sdcard") {
                uploadFile = SD.open(uploadPath.c_str(), "w");
            } else {
                uploadFile = LittleFS.open(uploadPath.c_str(), "w");
            }
        }
        
        if (uploadFile) {
            uploadFile.write(data, len);
        }
        
        if (final) {
            if (uploadFile) {
                uploadFile.close();
            }
        }
    });

    // File download endpoint
    g_webServer->on("/api/storage/download", HTTP_GET, [](AsyncWebServerRequest* request) {
        if (!request->hasParam("path") || !request->hasParam("storage")) {
            request->send(400, "application/json", "{\"error\":\"Missing parameters\"}");
            return;
        }
        
        String path = request->getParam("path")->value();
        String storage = request->getParam("storage")->value();
        
        File file;
        if (storage == "sdcard") {
            file = SD.open(path.c_str(), "r");
        } else {
            file = LittleFS.open(path.c_str(), "r");
        }
        
        if (!file) {
            request->send(404, "application/json", "{\"error\":\"File not found\"}");
            return;
        }
        
        String contentType = "application/octet-stream";
        if (path.endsWith(".txt")) contentType = "text/plain";
        else if (path.endsWith(".json")) contentType = "application/json";
        else if (path.endsWith(".html")) contentType = "text/html";
        
        AsyncWebServerResponse* response = request->beginResponse(file, path.c_str(), contentType);
        request->send(response);
        file.close();
    });

    // File delete endpoint
    g_webServer->on("/api/storage/delete", HTTP_DELETE, [](AsyncWebServerRequest* request) {
        using namespace NightStrike::Core;
        if (!request->hasParam("path") || !request->hasParam("storage")) {
            request->send(400, "application/json", "{\"error\":\"Missing parameters\"}");
            return;
        }
        
        String path = request->getParam("path")->value();
        String storage = request->getParam("storage")->value();
        bool preferSD = (storage == "sdcard");
        
        auto& storageMgr = Storage::getInstance();
        Error err = storageMgr.deleteFile(path.c_str(), preferSD);
        
        if (err.isError()) {
            request->send(500, "application/json", "{\"error\":\"Delete failed\"}");
        } else {
            request->send(200, "application/json", "{\"status\":\"ok\"}");
        }
    });

    g_webServer->begin();
    _active = true;
    _initialized = true;

    Serial.printf("[WebUI] Started on port %d\n", port);
    return Error(ErrorCode::SUCCESS);
}

Error WebUI::shutdown() {
    if (!_initialized) {
        return Error(ErrorCode::NOT_INITIALIZED);
    }

    if (g_webServer) {
        g_webServer->end();
        delete g_webServer;
        g_webServer = nullptr;
    }

    _active = false;
    _initialized = false;
    return Error(ErrorCode::SUCCESS);
}

Error WebUI::addRoute(const std::string& method, const std::string& path, RouteHandler handler) {
    if (!_initialized || !g_webServer) {
        return Error(ErrorCode::NOT_INITIALIZED);
    }

    // TODO: Implement custom route handling
    Serial.printf("[WebUI] Route added: %s %s\n", method.c_str(), path.c_str());
    return Error(ErrorCode::SUCCESS);
}

std::string WebUI::getURL() const {
    if (!_active) {
        return "";
    }

    IPAddress ip = WiFi.softAPIP();
    if (ip == IPAddress(0, 0, 0, 0)) {
        ip = WiFi.localIP();
    }

    char url[64];
    snprintf(url, sizeof(url), "http://%d.%d.%d.%d:%d",
             ip[0], ip[1], ip[2], ip[3], _port);
    return std::string(url);
}

} // namespace Core
} // namespace NightStrike

