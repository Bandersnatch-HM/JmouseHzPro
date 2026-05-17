#include "web_portal.h"
#include "web_pages.h"

void WebPortal::begin(JigglerConfig* cfg, Storage* storage,
                      MouseCallbacks* mouse, MovementEngine* movement) {
    _cfg = cfg;
    _storage = storage;
    _mouse = mouse;
    _movement = movement;
    _startTime = millis();

    _lastInteraction = millis();

    // Start WiFi (STA + AP)
    bool staConnected = false;
    if (strlen(_cfg->wifiSSID) > 0) {
        DBGF("[Portal] Connecting to WiFi: %s\n", _cfg->wifiSSID);
        WiFi.mode(WIFI_AP_STA);
        WiFi.begin(_cfg->wifiSSID, _cfg->wifiPass);
        
        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 5000) {
            delay(100);
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            DBGF("[Portal] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
            staConnected = true;
        } else {
            DBGLN("[Portal] WiFi STA failed");
        }
    }

    if (!staConnected) {
        WiFi.mode(WIFI_AP);
    }
    
    WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL, 0, AP_MAX_CONNECTIONS);
    WiFi.setTxPower(WIFI_POWER_11dBm); // Lower power helps BT coexistence and RAM stability
    delay(500);
    DBGF("[Portal] AP started: %s  IP: %s\n", AP_SSID,
         WiFi.softAPIP().toString().c_str());

    // mDNS for easy access (http://jmouse.local)
    if (MDNS.begin(MDNS_NAME)) {
        DBGF("[Portal] mDNS responder started: http://%s.local\n", MDNS_NAME);
        MDNS.addService("http", "tcp", 80);
    }

    // DNS for captive portal
    _dns = new DNSServer();
    _dns->start(53, "*", WiFi.softAPIP());
    
    // Web server (Standard synchronous)
    _server = new WebServer(80);
    _setupRoutes();
    _server->begin();
    
    _active = true;
    DBGLN("[Portal] Lite Web Server started");
}

void WebPortal::stop() {
    if (!_active) return;
    if (_server) { _server->stop(); delete _server; _server = nullptr; }
    if (_dns) { _dns->stop(); delete _dns; _dns = nullptr; }
    MDNS.end();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    _active = false;
    DBGLN("[Portal] Stopped");
}

void WebPortal::update() {
    if (!_active) return;
    _dns->processNextRequest();
    _server->handleClient();
}

void WebPortal::_setupRoutes() {
    // Main page
    _server->on("/", HTTP_GET, [this]() {
        recordInteraction();
        _server->sendHeader("Connection", "close");
        _server->send(200, "text/html", INDEX_HTML);
    });

    // Favicon (empty to save RAM/requests)
    _server->on("/favicon.ico", HTTP_GET, [this]() {
        _server->sendHeader("Connection", "close");
        _server->send(204, "text/plain", ""); 
    });

    // API: Status
    _server->on("/api/status", HTTP_GET, [this]() {
        recordInteraction();
        _server->sendHeader("Connection", "close");
        _server->send(200, "application/json", _buildStatusJson());
    });

    // API: Command
    _server->on("/api/cmd", HTTP_POST, [this]() {
        recordInteraction();
        _server->sendHeader("Connection", "close");
        String body = _server->arg("plain");
        int cmdStart = body.indexOf("\"cmd\":\"") + 7;
        int cmdEnd = body.indexOf("\"", cmdStart);
        if (cmdStart > 6 && cmdEnd > cmdStart) {
            _lastCmd = body.substring(cmdStart, cmdEnd);
            int argStart = body.indexOf("\"arg\":") + 6;
            if (argStart > 5) {
                _lastArg = body.substring(argStart).toInt();
            }
            _cmdPending = true;
        }
        _server->send(200, "application/json", "{\"ok\":true}");
    });

    // API: Save movement
    _server->on("/api/movement", HTTP_POST, [this]() {
        recordInteraction();
        _server->sendHeader("Connection", "close");
        String body = _server->arg("plain");
        int modeStart = body.indexOf("\"mode\":") + 7;
        if (modeStart > 6) _cfg->moveMode = body.substring(modeStart).toInt();
        int intStart = body.indexOf("\"interval\":") + 11;
        if (intStart > 10) _cfg->moveInterval = body.substring(intStart).toInt() * 1000;
        int ampStart = body.indexOf("\"amplitude\":") + 12;
        if (ampStart > 11) _cfg->moveAmplitude = body.substring(ampStart).toInt();
        
        _cfg->humanPauses = body.indexOf("\"humanPauses\":true") >= 0;
        _cfg->antiDetection = body.indexOf("\"antiDetection\":true") >= 0;

        _storage->saveConfig(*_cfg);
        _movement->setMode(_cfg->moveMode);
        _movement->setInterval(_cfg->moveInterval);
        _movement->setAmplitude(_cfg->moveAmplitude);
        
        _server->send(200, "application/json", "{\"ok\":true}");
    });

    // API: Save WiFi
    _server->on("/api/wifi", HTTP_POST, [this]() {
        recordInteraction();
        _server->sendHeader("Connection", "close");
        String body = _server->arg("plain");
        int ssidStart = body.indexOf("\"ssid\":\"") + 8;
        int ssidEnd = body.indexOf("\"", ssidStart);
        if (ssidStart > 7 && ssidEnd > ssidStart) {
            strncpy(_cfg->wifiSSID, body.substring(ssidStart, ssidEnd).c_str(), 31);
        }
        int passStart = body.indexOf("\"pass\":\"") + 8;
        int passEnd = body.indexOf("\"", passStart);
        if (passStart > 7 && passEnd > passStart) {
            strncpy(_cfg->wifiPass, body.substring(passStart, passEnd).c_str(), 63);
        }
        _storage->saveConfig(*_cfg);
        _server->send(200, "application/json", "{\"ok\":true}");
    });

    // API: Save Name
    _server->on("/api/name", HTTP_POST, [this]() {
        recordInteraction();
        _server->sendHeader("Connection", "close");
        String body = _server->arg("plain");
        int nameStart = body.indexOf("\"name\":\"") + 8;
        int nameEnd = body.indexOf("\"", nameStart);
        if (nameStart > 7 && nameEnd > nameStart) {
            strncpy(_cfg->deviceName, body.substring(nameStart, nameEnd).c_str(), 31);
            _storage->saveConfig(*_cfg);
        }
        _server->send(200, "application/json", "{\"ok\":true}");
    });

    // Captive Portal Redirects (Apple, Android, Windows)
    _server->on("/generate_204", [this](){ _server->sendHeader("Location", "/", true); _server->send(302, "text/plain", ""); });
    _server->on("/fwlink", [this](){ _server->sendHeader("Location", "/", true); _server->send(302, "text/plain", ""); });
    _server->on("/hotspot-detect.html", [this](){ _server->sendHeader("Location", "/", true); _server->send(302, "text/plain", ""); });
    _server->on("/connectivity-check.html", [this](){ _server->sendHeader("Location", "/", true); _server->send(302, "text/plain", ""); });
    _server->on("/check_network_status", [this](){ _server->sendHeader("Location", "/", true); _server->send(302, "text/plain", ""); });

    // Catch-all for captive portal
    _server->onNotFound([this]() {
        _server->sendHeader("Location", "/", true);
        _server->send(302, "text/plain", "");
    });
}

String WebPortal::_buildStatusJson() {
    String json = "{";
    json += "\"connected\":" + String(_bleConnected ? "true" : "false");
    json += ",\"paused\":" + String(_paused ? "true" : "false");
    json += ",\"mode\":" + String(_cfg->moveMode);
    json += ",\"interval\":" + String(_cfg->moveInterval / 1000);
    json += ",\"amplitude\":" + String(_cfg->moveAmplitude);
    json += ",\"jiggles\":" + String(_cfg->totalJiggles);
    json += ",\"uptime\":" + String(millis() - _startTime);
    json += ",\"deviceName\":\"" + String(_cfg->deviceName) + "\"";
    json += ",\"bonds\":[]"; // Simplified for Lite
    json += "}";
    return json;
}

void WebPortal::_broadcastStatus() {
    // Not used in Lite (polling only)
}
