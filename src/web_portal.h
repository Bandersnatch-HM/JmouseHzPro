#ifndef WEB_PORTAL_H
#define WEB_PORTAL_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include "config.h"
#include "storage.h"
#include "movement.h"

// Callbacks so the portal works with any mouse driver (BLE or USB)
struct MouseCallbacks {
    int (*getBondedCount)();
    // getBondedMacs fills a buffer with MAC strings, returns count
    int (*getBondedMacs)(char macs[][18], int maxCount);
    bool isUsb; // true = USB mode, false = BLE mode
};

class WebPortal {
public:
    void begin(JigglerConfig* cfg, Storage* storage,
               MouseCallbacks* mouse, MovementEngine* movement);
    void stop();
    void update();
    bool isActive() { return _active; }

    // External state setters
    void setPaused(bool p) { _paused = p; }
    void setJiggles(uint32_t j) { _jiggles = j; }
    void setConnected(bool c) { _bleConnected = c; }

    // Callbacks
    bool hasPendingCommand() { return _cmdPending; }
    String getPendingCommand() { _cmdPending = false; return _lastCmd; }
    int getPendingArg() { return _lastArg; }

    // Activity tracking
    unsigned long getLastInteraction() { return _lastInteraction; }
    void recordInteraction() { _lastInteraction = millis(); }

private:
    WebServer* _server = nullptr;
    DNSServer* _dns = nullptr;
    JigglerConfig* _cfg = nullptr;
    Storage* _storage = nullptr;
    MouseCallbacks* _mouse = nullptr;
    MovementEngine* _movement = nullptr;

    bool _active = false;
    bool _paused = false;
    bool _bleConnected = false;
    uint32_t _jiggles = 0;
    unsigned long _startTime = 0;
    unsigned long _lastBroadcast = 0;
    unsigned long _lastInteraction = 0;

    bool _cmdPending = false;
    String _lastCmd;
    int _lastArg = 0;

    void _setupRoutes();
    String _buildStatusJson();
    void _broadcastStatus();
};

#endif
