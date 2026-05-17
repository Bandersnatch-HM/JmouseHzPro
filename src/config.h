#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ===== Device Identity =====
#define DEVICE_NAME         "Jmouse HzPro"
#define MANUFACTURER_NAME   "HzTech"
#define FIRMWARE_VERSION    "1.0.0"
#define BATTERY_LEVEL       100

// ===== GPIO Pins =====
#define LED_PIN             2         // Built-in LED
#define BUTTON_PIN          0         // BOOT button (GPIO0)
#define LED_ON              HIGH
#define LED_OFF             LOW

// ===== Movement Defaults =====
#define DEFAULT_MOVE_MODE       7     // Random Mix
#define DEFAULT_MOVE_INTERVAL   5000  // 5 seconds
#define DEFAULT_MOVE_AMPLITUDE  5     // 5 pixels
#define DEFAULT_VARIATION_PCT   30    // +/-30% timing variation
#define DEFAULT_HUMAN_PAUSES    true

// ===== Movement Mode IDs =====
#define MODE_MICRO_JIGGLE   0
#define MODE_HORIZONTAL     1
#define MODE_VERTICAL       2
#define MODE_CROSS          3
#define MODE_BEZIER         4
#define MODE_CIRCLE         5
#define MODE_NATURAL_DRIFT  6
#define MODE_RANDOM_MIX     7
#define MODE_COUNT          8

// ===== WiFi AP & STA =====
#define AP_SSID             "JmouseHzPro-Setup"
#define AP_PASSWORD         ""        // Open network for easy setup
#define AP_CHANNEL          1
#define AP_MAX_CONNECTIONS  2
#define PORTAL_TIMEOUT_MS   300000    // 5 minutes auto-close
#define MDNS_NAME           "jmouse"  // Access via http://jmouse.local

// ===== Button Timings =====
#define DEBOUNCE_MS         50
#define SHORT_PRESS_MAX_MS  500
#define DOUBLE_CLICK_GAP_MS 400
#define LONG_PRESS_MS       3000
#define VERY_LONG_PRESS_MS  10000

// ===== Anti-Detection =====
#define PAUSE_EVERY_MIN     5
#define PAUSE_EVERY_MAX     15
#define PAUSE_DURATION_MIN  30000    // 30s
#define PAUSE_DURATION_MAX  120000   // 2min

// ===== Max Devices =====
#define MAX_DEVICE_SLOTS    3

// ===== Serial Debug =====
#define SERIAL_BAUD         115200
#define DEBUG_ENABLED       true

#if DEBUG_ENABLED
  #define DBG(x)      Serial.print(x)
  #define DBGLN(x)    Serial.println(x)
  #define DBGF(...)   Serial.printf(__VA_ARGS__)
#else
  #define DBG(x)
  #define DBGLN(x)
  #define DBGF(...)
#endif

#endif
