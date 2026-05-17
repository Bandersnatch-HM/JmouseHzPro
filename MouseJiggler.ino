#include <BleMouse.h>
#include "config.h"

BleMouse bleMouse(DEVICE_NAME, MANUFACTURER, BATTERY_LEVEL);

unsigned long lastMoveTime = 0;
bool wasConnected = false;
int8_t stepX = JIGGLE_PX;
int8_t stepY = 0;
uint8_t moveCount = 0;

void setup() {
  #if DEBUG_ENABLED
    Serial.begin(SERIAL_BAUD);
  #endif

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF);

  DEBUG_PRINTLN("Starting " DEVICE_NAME "...");
  DEBUG_PRINTLN("Pair via Bluetooth as a mouse.");
  bleMouse.begin();
}

void jiggleOnce() {
  bleMouse.move(stepX, stepY, 0);
  delay(10);
  bleMouse.move(-stepX, -stepY, 0);
  moveCount++;
  DEBUG_PRINT("Jiggle #"); DEBUG_PRINTLN(moveCount);
}

void updateDirection() {
  switch (MOVEMENT_MODE) {
    case 0:
      stepX = (moveCount % 2 == 0) ? JIGGLE_PX : -JIGGLE_PX;
      stepY = 0;
      break;
    case 1:
      stepX = 0;
      stepY = (moveCount % 2 == 0) ? JIGGLE_PX : -JIGGLE_PX;
      break;
    case 2:
      if (moveCount % 2 == 0) {
        stepX = ((moveCount / 2) % 2 == 0) ? JIGGLE_PX : -JIGGLE_PX;
        stepY = 0;
      } else {
        stepX = 0;
        stepY = ((moveCount / 2) % 2 == 0) ? JIGGLE_PX : -JIGGLE_PX;
      }
      break;
    case 3:
      stepX = (random(0, 2) == 0) ? JIGGLE_PX : -JIGGLE_PX;
      stepY = (random(0, 2) == 0) ? JIGGLE_PX : -JIGGLE_PX;
      if (random(0, 3) == 0) stepY = 0;
      if (random(0, 3) == 0) stepX = 0;
      break;
  }
}

void handleConnection() {
  if (bleMouse.isConnected()) {
    if (!wasConnected) {
      wasConnected = true;
      DEBUG_PRINTLN("Connected!");
    }
    digitalWrite(LED_PIN, LED_ON);

    unsigned long now = millis();
    if (now - lastMoveTime >= MOVE_INTERVAL_MS) {
      updateDirection();
      jiggleOnce();
      lastMoveTime = now;
    }
  } else {
    wasConnected = false;
    digitalWrite(LED_PIN, (millis() / LED_BLINK_MS) % 2 == 0 ? LED_ON : LED_OFF);
  }
}

void enterDeepSleep() {
  DEBUG_PRINTLN("Entering deep sleep...");
  digitalWrite(LED_PIN, LED_OFF);
  esp_sleep_enable_timer_wakeup(DEEP_SLEEP_SEC * 1000000ULL);
  esp_deep_sleep_start();
}

void loop() {
  #if ENABLE_DEEP_SLEEP
    handleConnection();
    enterDeepSleep();
  #else
    handleConnection();
    delay(LOOP_DELAY_MS);
  #endif
}
