#include "button_handler.h"

void ButtonHandler::begin(uint8_t pin) {
    _pin = pin;
    pinMode(_pin, INPUT_PULLUP);
}

ButtonEvent ButtonHandler::update() {
    bool reading = digitalRead(_pin);

    // Debounce
    if (reading != _lastState) {
        _lastDebounce = millis();
    }
    _lastState = reading;

    if ((millis() - _lastDebounce) < DEBOUNCE_MS) {
        return BTN_NONE;
    }

    bool buttonDown = (reading == LOW); // Active low (BOOT button)

    // Button just pressed
    if (buttonDown && !_pressed) {
        _pressed = true;
        _pressStart = millis();
        _longFired = false;
        _veryLongFired = false;
        return BTN_NONE;
    }

    // Button held down - check for very long press (10s)
    if (buttonDown && _pressed) {
        unsigned long held = millis() - _pressStart;
        if (held >= VERY_LONG_PRESS_MS && !_veryLongFired) {
            _veryLongFired = true;
            _clickCount = 0;
            _waitingForMulti = false;
            return BTN_VERY_LONG_PRESS;
        }
        return BTN_NONE;
    }

    // Button just released
    if (!buttonDown && _pressed) {
        _pressed = false;
        unsigned long held = millis() - _pressStart;

        // Ignore release if very long press already fired
        if (_veryLongFired) {
            return BTN_NONE;
        }

        // Check for 3s long press upon release
        if (held >= LONG_PRESS_MS) {
            _clickCount = 0;
            _waitingForMulti = false;
            return BTN_LONG_PRESS;
        }

        // Short press - count clicks
        if (held < SHORT_PRESS_MAX_MS) {
            _clickCount++;
            _lastRelease = millis();
            _waitingForMulti = true;
        }
        return BTN_NONE;
    }

    // Check for multi-click timeout
    if (_waitingForMulti && !_pressed &&
        (millis() - _lastRelease) > DOUBLE_CLICK_GAP_MS) {
        _waitingForMulti = false;
        uint8_t clicks = _clickCount;
        _clickCount = 0;
        switch (clicks) {
            case 1: return BTN_SINGLE_CLICK;
            case 2: return BTN_DOUBLE_CLICK;
            case 3: return BTN_TRIPLE_CLICK;
            default: return BTN_SINGLE_CLICK;
        }
    }

    return BTN_NONE;
}
