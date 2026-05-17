#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include "config.h"

#if HAS_BLE

#include <BleCombo.h>
#include "config.h"

// Bond management via ESP-IDF
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"

struct BondedDevice {
    uint8_t address[6];
    bool valid;
};

class BleManager {
public:
    void begin(const char* name);
    void end();
    bool isConnected();
    void move(int8_t x, int8_t y, int8_t wheel = 0);
    void pressKey(uint8_t key);
    void releaseKey(uint8_t key);
    void setBatteryLevel(uint8_t level);

    // Bond management
    int getBondedCount();
    void getBondedDevices(BondedDevice* list, int& count);
    void removeAllBonds();
    void removeBond(uint8_t* address);
    void restartAdvertising();

    // Status
    unsigned long getConnectedSince() { return _connectedSince; }
    bool wasJustConnected();
    bool wasJustDisconnected();

private:
    BleCombo* _combo = nullptr;
    bool _connected = false;
    bool _prevConnected = false;
    unsigned long _connectedSince = 0;
    char _name[32];
};

#endif // HAS_BLE
#endif

