#ifndef CONFIG_H
#define CONFIG_H

// ===== Device Configuration =====
#define DEVICE_NAME         "ESP32 Mouse Jiggler"
#define MANUFACTURER        "Espressif"
#define BATTERY_LEVEL       100

// ===== Movement Configuration =====
#define MOVE_INTERVAL_MS    10000     // Time between jiggles (ms)
#define JIGGLE_PX           1         // Pixels to move per jiggle
#define LOOP_DELAY_MS       50        // Main loop delay (ms)

// ===== Movement Mode =====
// 0 = Horizontal only (left-right)
// 1 = Vertical only (up-down)
// 2 = Cross (alternates H and V)
// 3 = Random direction
#define MOVEMENT_MODE       2

// ===== LED Pin =====
#define LED_PIN             2         // Built-in LED on most ESP32 dev boards
#define LED_ON              HIGH
#define LED_OFF             LOW
#define LED_BLINK_MS        500       // Blink interval while connected

// ===== Power Saving =====
// Set to true to enable deep sleep between movements (battery powered)
#define ENABLE_DEEP_SLEEP   false
#define DEEP_SLEEP_SEC      10        // Sleep time in seconds

// ===== Serial Debug =====
#define SERIAL_BAUD         115200
#define DEBUG_ENABLED       true

#if DEBUG_ENABLED
  #define DEBUG_PRINT(x)    Serial.print(x)
  #define DEBUG_PRINTLN(x)  Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

#endif
