#include "usb_mouse.h"

#if HAS_USB_HID

void UsbMouseManager::begin(const char* name) {
    _mouse.begin();
    USB.productName(name);
    USB.manufacturerName(MANUFACTURER_NAME);
    USB.begin();
    _started = true;
    _connectedSince = millis();
    DBGF("[USB] Started as '%s'\n", name);
}

void UsbMouseManager::end() {
    _started = false;
    DBGLN("[USB] Stopped");
}

bool UsbMouseManager::isConnected() {
    _prevConnected = _connected;
    // USB HID is "connected" whenever the USB cable is plugged in and
    // the device has been initialized. We detect via USB being started.
    _connected = _started;
    if (_connected && !_prevConnected) {
        _connectedSince = millis();
    }
    return _connected;
}

bool UsbMouseManager::wasJustConnected() {
    return _connected && !_prevConnected;
}

bool UsbMouseManager::wasJustDisconnected() {
    return !_connected && _prevConnected;
}

void UsbMouseManager::move(int8_t x, int8_t y, int8_t wheel) {
    if (_started) {
        _mouse.move(x, y, wheel);
    }
}

#endif // HAS_USB_HID
