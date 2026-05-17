#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>
#include "config.h"

enum ButtonEvent {
    BTN_NONE,
    BTN_SINGLE_CLICK,
    BTN_DOUBLE_CLICK,
    BTN_TRIPLE_CLICK,
    BTN_LONG_PRESS,
    BTN_VERY_LONG_PRESS
};

class ButtonHandler {
public:
    void begin(uint8_t pin = BUTTON_PIN);
    ButtonEvent update();
    bool isPressed() const { return _pressed; }
    uint32_t getDuration() const { return _pressed ? (millis() - _pressStart) : (_lastRelease - _pressStart); }


private:
    uint8_t _pin;
    bool _lastState = HIGH;
    bool _currentState = HIGH;
    unsigned long _lastDebounce = 0;
    unsigned long _pressStart = 0;
    unsigned long _lastRelease = 0;
    uint8_t _clickCount = 0;
    bool _pressed = false;
    bool _longFired = false;
    bool _veryLongFired = false;
    bool _waitingForMulti = false;
};

#endif
