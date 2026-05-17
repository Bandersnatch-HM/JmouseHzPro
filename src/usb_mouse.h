#ifndef USB_MOUSE_H
#define USB_MOUSE_H

#include "config.h"

#if HAS_USB_HID

#include <Arduino.h>
#include "USB.h"
#include "USBHIDMouse.h"
#include "config.h"

class UsbMouseManager {
public:
    void begin(const char* name);
    void end();
    bool isConnected();
    void move(int8_t x, int8_t y, int8_t wheel = 0);

    // USB doesn't have bonding, but we keep a compatible API
    int getBondedCount() { return _started ? 1 : 0; }
    void removeAllBonds() {} // No-op for USB
    void restartAdvertising() {} // No-op for USB

    bool wasJustConnected();
    bool wasJustDisconnected();

private:
    USBHIDMouse _mouse;
    bool _started = false;
    bool _connected = false;
    bool _prevConnected = false;
    unsigned long _connectedSince = 0;
};

#endif // HAS_USB_HID
#endif // USB_MOUSE_H
