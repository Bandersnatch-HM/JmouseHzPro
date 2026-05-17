/*
 * ╔══════════════════════════════════════════╗
 * ║       Jmouse HzPro v1.0.0               ║
 * ║   Bluetooth & USB Mouse Jiggler         ║
 * ║   Windows · macOS · Android             ║
 * ╚══════════════════════════════════════════╝
 *
 * Supported chips:
 *   ESP32     → BLE Mouse (Bluetooth)
 *   ESP32-S2  → USB HID Mouse (USB cable)
 *   ESP32-S3  → BLE Mouse (BLE default) or USB HID
 *
 * Button Controls (BOOT button GPIO0):
 *   Single click  → Pause / Resume jiggle
 *   Double click  → Restart BLE advertising (BLE only)
 *   Triple click  → Force jiggle now
 *   Long press 3s → Toggle web portal
 *   Long press 5s → Unpair all devices (BLE only)
 */

#include "config.h"
#include "storage.h"
#include "button_handler.h"
#include "movement.h"
#include "web_portal.h"

// ===== Auto-detect connection mode =====
// ESP32-S2: USB only (no BLE)
// ESP32 classic / C3: BLE only (no USB OTG)
// ESP32-S3: Both available, uses BLE by default
#if defined(CONFIG_IDF_TARGET_ESP32S2)
  #define MODE_USB    1
  #define MODE_BLE    0
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
  // S3 has both — default to BLE, but can be overridden
  #ifndef FORCE_USB_MODE
    #define MODE_USB  0
    #define MODE_BLE  1
  #else
    #define MODE_USB  1
    #define MODE_BLE  0
  #endif
#else
  // Classic ESP32, ESP32-C3 etc. — BLE only
  #define MODE_USB    0
  #define MODE_BLE    1
#endif

// ===== Global Objects =====
Storage storage;
JigglerConfig cfg;
ButtonHandler button;
MovementEngine movement;
WebPortal portal;

#if MODE_BLE
  #include "ble_manager.h"
  BleManager mouseDriver;
#endif
#if MODE_USB
  #include "usb_mouse.h"
  UsbMouseManager mouseDriver;
#endif

// ===== State =====
bool paused = false;
bool portalActive = false;
unsigned long bootTime = 0;
unsigned long lastLedToggle = 0;
bool ledState = false;

// LED speed feedback state
uint8_t speedBlinksLeft = 0;
unsigned long lastSpeedBlink = 0;

// ===== Helpers: unified mouse interface =====
inline bool mouseConnected() { return mouseDriver.isConnected(); }
inline void mouseMove(int8_t x, int8_t y) { mouseDriver.move(x, y); }
inline bool mouseJustConnected() { return mouseDriver.wasJustConnected(); }
inline bool mouseJustDisconnected() { return mouseDriver.wasJustDisconnected(); }

// ===== Mouse callbacks for web portal =====
int _cbGetBondedCount() { return mouseDriver.getBondedCount(); }

#if MODE_BLE
int _cbGetBondedMacs(char macs[][18], int maxCount) {
    BondedDevice devs[MAX_DEVICE_SLOTS];
    int count = 0;
    mouseDriver.getBondedDevices(devs, count);
    if (count > maxCount) count = maxCount;
    for (int i = 0; i < count; i++) {
        snprintf(macs[i], 18, "%02X:%02X:%02X:%02X:%02X:%02X",
                 devs[i].address[0], devs[i].address[1], devs[i].address[2],
                 devs[i].address[3], devs[i].address[4], devs[i].address[5]);
    }
    return count;
}
#else
int _cbGetBondedMacs(char macs[][18], int maxCount) {
    if (maxCount > 0) { strncpy(macs[0], "USB-DIRECT", 18); return 1; }
    return 0;
}
#endif

MouseCallbacks mouseCB = {
    _cbGetBondedCount,
    _cbGetBondedMacs,
    #if MODE_USB
    true  // isUsb
    #else
    false
    #endif
};

// ===== LED Patterns =====
void updateLED() {
    unsigned long now = millis();

    if (button.isPressed()) {
        uint32_t dur = button.getDuration();
        if (dur >= 3000 && dur < 3600) {
            // 3 rapid blinks to confirm 3s portal threshold reached
            uint32_t phase = (dur - 3000) / 100;
            digitalWrite(LED_PIN, (phase % 2 == 0) ? LED_ON : LED_OFF);
        } else if (dur >= 7000) {
            // Fast bomb-timer flashing when approaching 10s factory reset
            digitalWrite(LED_PIN, ((dur / 80) % 2 == 0) ? LED_ON : LED_OFF);
        } else if (dur < 3000) {
            // Solid ON while holding initial 1-3s
            digitalWrite(LED_PIN, LED_ON);
        }
        return; // Skip normal LED logic while button is pressed
    }

    if (speedBlinksLeft > 0) {
        // Fast blink N times to indicate speed level
        if (now - lastSpeedBlink > 150) {
            ledState = !ledState;
            digitalWrite(LED_PIN, ledState ? LED_ON : LED_OFF);
            lastSpeedBlink = now;
            if (!ledState) {
                speedBlinksLeft--;
            }
        }
        return; // Skip normal LED logic while showing speed
    }

    if (portalActive) {
        // Triple blink pattern for portal mode
        unsigned long cycle = now % 2000;
        if (cycle < 100 || (cycle > 200 && cycle < 300) ||
            (cycle > 400 && cycle < 500)) {
            digitalWrite(LED_PIN, LED_ON);
        } else {
            digitalWrite(LED_PIN, LED_OFF);
        }
    } else if (mouseConnected()) {
        if (paused) {
            // Breathing LED using ledc (PWM)
            unsigned long cycle = now % 3000;
            int brightness = (cycle < 1500) ? (cycle * 255 / 1500) : ((3000 - cycle) * 255 / 1500);
            int duty = (brightness * brightness) / 255;
            ledcSetup(0, 1000, 8);
            ledcAttachPin(LED_PIN, 0);
            ledcWrite(0, duty);
            return;
        } else {
            ledcDetachPin(LED_PIN);
            pinMode(LED_PIN, OUTPUT);
            digitalWrite(LED_PIN, LED_ON);
        }
    } else {
        ledcDetachPin(LED_PIN);
        pinMode(LED_PIN, OUTPUT);
        if (now - lastLedToggle > 1000) {
            ledState = !ledState;
            digitalWrite(LED_PIN, ledState ? LED_ON : LED_OFF);
            lastLedToggle = now;
        }
    }
}

// ===== Execute a jiggle sequence =====
void doJiggle() {
    // Visual feedback: turn OFF LED while moving
    digitalWrite(LED_PIN, LED_OFF);

    MoveStep steps[16];
    uint8_t count = 0;
    movement.getNextSequence(steps, count);
    for (uint8_t i = 0; i < count; i++) {
        if (steps[i].keyPress != 0) mouseDriver.pressKey(steps[i].keyPress);
        if (steps[i].dx != 0 || steps[i].dy != 0) mouseMove(steps[i].dx, steps[i].dy);
        if (steps[i].keyRelease != 0) mouseDriver.releaseKey(steps[i].keyRelease);
        delay(steps[i].delayMs);
    }
    storage.incrementJiggles(cfg);

    // Restore LED state
    digitalWrite(LED_PIN, ledState ? LED_ON : LED_OFF);
}

// ===== Handle Button Events =====
void handleButton(ButtonEvent evt) {
    switch (evt) {
        case BTN_SINGLE_CLICK:
            paused = !paused;
            DBGF("[Main] %s\n", paused ? "PAUSED" : "RESUMED");
            break;

        case BTN_DOUBLE_CLICK: {
            // Cycle speed (interval)
            const uint32_t INTERVALS[] = {5000, 15000, 30000, 60000};
            uint8_t currentIdx = 0;
            for (uint8_t i = 0; i < 4; i++) {
                if (cfg.moveInterval <= INTERVALS[i]) {
                    currentIdx = i;
                    break;
                }
            }
            currentIdx = (currentIdx + 1) % 4;
            cfg.moveInterval = INTERVALS[currentIdx];
            movement.setInterval(cfg.moveInterval);
            storage.saveConfig(cfg);
            
            DBGF("[Main] Speed changed to %lu ms\n", cfg.moveInterval);
            
            // Trigger visual feedback (1 to 4 blinks)
            speedBlinksLeft = currentIdx + 1;
            ledState = false;
            lastSpeedBlink = millis();
            digitalWrite(LED_PIN, LED_OFF);
            break;
        }

        case BTN_TRIPLE_CLICK:
            DBGLN("[Main] Triple click: Restarting BLE advertising & Force jiggle!");
            #if MODE_BLE
            mouseDriver.restartAdvertising();
            #endif
            if (mouseConnected()) {
                doJiggle();
            }
            break;

        case BTN_LONG_PRESS:
            if (portalActive) {
                DBGLN("[Main] Stopping web portal. Restarting...");
                portal.stop();
                portalActive = false;
                delay(500);
                ESP.restart();
            } else {
                DBGLN("[Main] Starting web portal (Mouse paused for stability)");
                #if MODE_BLE
                mouseDriver.end();
                #endif
                delay(500);
                portal.begin(&cfg, &storage, &mouseCB, &movement);
                portalActive = true;
            }
            break;

        case BTN_VERY_LONG_PRESS:
            DBGLN("[Main] ⚠️ FACTORY RESET (10s Hold)");
            storage.resetToDefaults(cfg);
            #if MODE_BLE
                mouseDriver.removeAllBonds();
            #endif
            delay(1000);
            ESP.restart();
            break;

        default:
            break;
    }
}

// ===== Handle Web Portal Commands =====
void handlePortalCmd(const String& cmd, int arg) {
    if (cmd == "toggle") {
        paused = !paused;
        DBGF("[Portal] %s\n", paused ? "PAUSED" : "RESUMED");
    } else if (cmd == "jiggle") {
        if (mouseConnected()) doJiggle();
    } else if (cmd == "advertise") {
        #if MODE_BLE
            mouseDriver.restartAdvertising();
        #endif
    } else if (cmd == "unpairAll") {
        #if MODE_BLE
            mouseDriver.removeAllBonds();
            mouseDriver.restartAdvertising();
        #endif
    } else if (cmd == "unpair") {
        #if MODE_BLE
            BondedDevice devs[MAX_DEVICE_SLOTS];
            int count = 0;
            mouseDriver.getBondedDevices(devs, count);
            if (arg >= 0 && arg < count) {
                mouseDriver.removeBond(devs[arg].address);
            }
        #endif
    } else if (cmd == "restart") {
        DBGLN("[Portal] Restarting...");
        delay(500);
        ESP.restart();
    } else if (cmd == "factory") {
        storage.resetToDefaults(cfg);
        #if MODE_BLE
            mouseDriver.removeAllBonds();
        #endif
        DBGLN("[Portal] Factory reset done. Restarting...");
        delay(500);
        ESP.restart();
    } else if (cmd == "welcomeDone") {
        storage.markFirstRunDone(cfg);
        DBGLN("[Main] Welcome dismissed. Restarting to enable BLE...");
        delay(1000);
        ESP.restart();
    }
}

// ===== Setup =====
void setup() {
#if DEBUG_ENABLED
    Serial.begin(SERIAL_BAUD);
    delay(100);
#endif

    // Emergency fail-safe: Hard hardware NVS wipe if button is held during power-on
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    delay(20);
    if (digitalRead(BUTTON_PIN) == LOW) {
        DBGLN("[Main] ⚠️ BOOT button held on startup! Waiting 3s to confirm emergency wipe...");
        pinMode(LED_PIN, OUTPUT);
        digitalWrite(LED_PIN, LED_ON);
        unsigned long bootHoldStart = millis();
        bool wipeConfirmed = true;
        while (millis() - bootHoldStart < 3000) {
            if (digitalRead(BUTTON_PIN) == HIGH) {
                wipeConfirmed = false;
                break;
            }
            delay(10);
        }
        if (wipeConfirmed) {
            DBGLN("[Main] ⚠️ EMERGENCY NVS WIPE CONFIRMED! Formateando...");
            for (int i = 0; i < 15; i++) {
                digitalWrite(LED_PIN, (i % 2 == 0) ? LED_ON : LED_OFF);
                delay(100);
            }
            storage.begin();
            storage.resetToDefaults(cfg);
            #if MODE_BLE
            mouseDriver.begin(DEVICE_NAME);
            mouseDriver.removeAllBonds();
            #endif
            DBGLN("[Main] NVS wipe completed. Restarting...");
            delay(500);
            ESP.restart();
        } else {
            DBGLN("[Main] Boot hold released. Continuing normal start.");
            digitalWrite(LED_PIN, LED_OFF);
        }
    }

    DBGLN("╔══════════════════════════════════════╗");
    DBGLN("║       Jmouse HzPro v1.0.0           ║");
    #if MODE_BLE
    DBGLN("║   Mode: Bluetooth (BLE)             ║");
    #else
    DBGLN("║   Mode: USB HID                     ║");
    #endif
    DBGLN("╚══════════════════════════════════════╝");

    // Initialize hardware
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LED_OFF);
    button.begin(BUTTON_PIN);
    bootTime = millis();

    // Load configuration from NVS
    storage.begin();
    storage.loadConfig(cfg);
    DBGF("[Main] Config loaded: mode=%d interval=%lu amp=%d\n",
         cfg.moveMode, cfg.moveInterval, cfg.moveAmplitude);

    // Initialize movement engine
    movement.begin(cfg.moveMode, cfg.moveInterval, cfg.moveAmplitude,
                   cfg.variationPercent, cfg.humanPauses);

    // Start mouse by default. Portal is accessed via 3s button hold.
    DBGLN("[Main] Normal run: Starting mouse only (Press BOOT 3s for Web Portal)");
    mouseDriver.begin(cfg.deviceName);
    portalActive = false;

    DBGF("[Main] Ready! Free Heap: %d\n", ESP.getFreeHeap());
    #if MODE_BLE
    DBGLN("[Main] Pair via Bluetooth as a mouse.");
    #endif
    DBGLN("[Main] BOOT button: click=pause, 3s=portal/restart");
}

// ===== Main Loop =====
void loop() {
    // 1. Process button
    ButtonEvent evt = button.update();
    if (evt != BTN_NONE) {
        handleButton(evt);
    }

    // 2. Update web portal
    if (portalActive) {
        if (millis() - portal.getLastInteraction() > PORTAL_TIMEOUT_MS) {
            DBGLN("[Main] ⏳ Web Portal inactivity timeout (5 min). Closing portal & resuming mouse...");
            portal.stop();
            portalActive = false;
            #if MODE_BLE
            mouseDriver.begin(cfg.deviceName);
            #endif
        }

        portal.update();
        portal.setPaused(paused);
        portal.setJiggles(cfg.totalJiggles);
        portal.setConnected(mouseConnected());

        if (portal.hasPendingCommand()) {
            handlePortalCmd(portal.getPendingCommand(), portal.getPendingArg());
        }
    } else {
        // Portal NOT active
    }

    // 3. Check connection & BLE Watchdog
    bool connected = mouseConnected();
    static unsigned long lastBleConnTime = millis();
    if (connected || portalActive || MODE_USB) {
        lastBleConnTime = millis();
    } else {
        if (millis() - lastBleConnTime > 180000) { // 3 minutes
            DBGLN("[Main] 🔄 BLE Watchdog: Disconnected for >3 min. Restarting advertising...");
            #if MODE_BLE
            mouseDriver.restartAdvertising();
            #endif
            lastBleConnTime = millis();
        }
    }

    if (mouseJustConnected()) {
        DBGLN("[Main] ✓ Device connected!");
    }
    if (mouseJustDisconnected()) {
        DBGLN("[Main] ✗ Device disconnected");
    }

    // 4. Execute jiggle if connected and not paused (AND portal is NOT active)
    if (connected && !paused && !portalActive) {
        if (movement.shouldMove()) {
            doJiggle();
            DBGF("[Main] Jiggle #%lu (mode %d)\n", cfg.totalJiggles, cfg.moveMode);
        }
    }

    // 5. Update LED
    updateLED();

    // 6. Small delay and yield for network stack
    delay(10);
    yield();
}
