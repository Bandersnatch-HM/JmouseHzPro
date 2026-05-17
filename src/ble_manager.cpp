#include "ble_manager.h"

#if HAS_BLE

void BleManager::begin(const char* name) {
    strncpy(_name, name, sizeof(_name) - 1);
    _name[sizeof(_name) - 1] = '\0';
    if (_combo) {
        delete _combo;
    }
    _combo = new BleCombo(_name, MANUFACTURER_NAME, BATTERY_LEVEL);
    _combo->begin();
    _connected = false;
    _prevConnected = false;
    DBGF("[BLE] Started as '%s'\n", _name);
}

void BleManager::end() {
    if (_combo) {
        _combo->end();
        delete _combo;
        _combo = nullptr;
    }
    _connected = false;
    DBGLN("[BLE] Stopped");
}

bool BleManager::isConnected() {
    _prevConnected = _connected;
    _connected = (_combo != nullptr) && _combo->isConnected();
    if (_connected && !_prevConnected) {
        _connectedSince = millis();
    }
    return _connected;
}

bool BleManager::wasJustConnected() {
    return _connected && !_prevConnected;
}

bool BleManager::wasJustDisconnected() {
    return !_connected && _prevConnected;
}

void BleManager::move(int8_t x, int8_t y, int8_t wheel) {
    if (_combo && _connected) {
        _combo->move(x, y, wheel);
    }
}

void BleManager::pressKey(uint8_t key) {
    if (_combo && _connected) {
        _combo->press(key);
    }
}

void BleManager::releaseKey(uint8_t key) {
    if (_combo && _connected) {
        _combo->release(key);
    }
}

void BleManager::setBatteryLevel(uint8_t level) {
    if (_combo) {
        _combo->setBatteryLevel(level);
    }
}

int BleManager::getBondedCount() {
    return esp_ble_get_bond_device_num();
}

void BleManager::getBondedDevices(BondedDevice* list, int& count) {
    int num = esp_ble_get_bond_device_num();
    if (num == 0) { count = 0; return; }
    if (num > MAX_DEVICE_SLOTS) num = MAX_DEVICE_SLOTS;

    esp_ble_bond_dev_t* devList =
        (esp_ble_bond_dev_t*)malloc(sizeof(esp_ble_bond_dev_t) * num);
    if (!devList) { count = 0; return; }

    esp_ble_get_bond_device_list(&num, devList);
    count = num;
    for (int i = 0; i < num; i++) {
        memcpy(list[i].address, devList[i].bd_addr, 6);
        list[i].valid = true;
    }
    free(devList);
}

void BleManager::removeAllBonds() {
    int num = esp_ble_get_bond_device_num();
    if (num == 0) return;

    esp_ble_bond_dev_t* devList =
        (esp_ble_bond_dev_t*)malloc(sizeof(esp_ble_bond_dev_t) * num);
    if (!devList) return;

    esp_ble_get_bond_device_list(&num, devList);
    for (int i = 0; i < num; i++) {
        esp_ble_remove_bond_device(devList[i].bd_addr);
    }
    free(devList);
    DBGLN("[BLE] All bonds removed");
}

void BleManager::removeBond(uint8_t* address) {
    esp_ble_remove_bond_device(address);
    DBGLN("[BLE] Bond removed");
}

void BleManager::restartAdvertising() {
    DBGLN("[BLE] Restarting advertising...");
    end();
    delay(500);
    begin(_name);
}

#endif // HAS_BLE
