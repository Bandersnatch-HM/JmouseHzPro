#include "storage.h"

void Storage::begin() {
    _prefs.begin("jmouse", false);
}

void Storage::loadConfig(JigglerConfig& cfg) {
    cfg.firstRun = _prefs.getBool("firstRun", true);
    String name = _prefs.getString("devName", DEVICE_NAME);
    strncpy(cfg.deviceName, name.c_str(), sizeof(cfg.deviceName) - 1);
    cfg.deviceName[sizeof(cfg.deviceName) - 1] = '\0';
    cfg.moveMode = _prefs.getUChar("moveMode", DEFAULT_MOVE_MODE);
    cfg.moveInterval = _prefs.getUInt("moveInt", DEFAULT_MOVE_INTERVAL);
    cfg.moveAmplitude = _prefs.getUChar("moveAmp", DEFAULT_MOVE_AMPLITUDE);
    cfg.variationPercent = _prefs.getUChar("varPct", DEFAULT_VARIATION_PCT);
    cfg.humanPauses = _prefs.getBool("humanP", DEFAULT_HUMAN_PAUSES);
    cfg.antiDetection = _prefs.getBool("antiDet", true);
    cfg.totalJiggles = _prefs.getUInt("totalJ", 0);
    String ssid = _prefs.getString("wifiSSID", "");
    String pass = _prefs.getString("wifiPass", "");
    strncpy(cfg.wifiSSID, ssid.c_str(), sizeof(cfg.wifiSSID) - 1);
    strncpy(cfg.wifiPass, pass.c_str(), sizeof(cfg.wifiPass) - 1);

    // Sanity / Self-healing check in case NVS memory is corrupt
    if (cfg.moveMode >= MODE_COUNT || cfg.moveInterval < 500 || cfg.moveAmplitude == 0 || cfg.moveAmplitude > 100) {
        DBGLN("[Storage] ⚠️ Corrupt NVS configuration detected! Self-healing to factory defaults...");
        resetToDefaults(cfg);
    }
}

void Storage::saveConfig(const JigglerConfig& cfg) {
    _prefs.putString("devName", cfg.deviceName);
    _prefs.putUChar("moveMode", cfg.moveMode);
    _prefs.putUInt("moveInt", cfg.moveInterval);
    _prefs.putUChar("moveAmp", cfg.moveAmplitude);
    _prefs.putUChar("varPct", cfg.variationPercent);
    _prefs.putBool("humanP", cfg.humanPauses);
    _prefs.putBool("antiDet", cfg.antiDetection);
    _prefs.putString("wifiSSID", cfg.wifiSSID);
    _prefs.putString("wifiPass", cfg.wifiPass);
}

void Storage::resetToDefaults(JigglerConfig& cfg) {
    strncpy(cfg.deviceName, DEVICE_NAME, sizeof(cfg.deviceName));
    cfg.moveMode = DEFAULT_MOVE_MODE;
    cfg.moveInterval = DEFAULT_MOVE_INTERVAL;
    cfg.moveAmplitude = DEFAULT_MOVE_AMPLITUDE;
    cfg.variationPercent = DEFAULT_VARIATION_PCT;
    cfg.humanPauses = DEFAULT_HUMAN_PAUSES;
    cfg.antiDetection = true;
    cfg.totalJiggles = 0;
    cfg.firstRun = true;
    memset(cfg.wifiSSID, 0, sizeof(cfg.wifiSSID));
    memset(cfg.wifiPass, 0, sizeof(cfg.wifiPass));
    saveConfig(cfg);
    _prefs.putBool("firstRun", true);
    _prefs.putUInt("totalJ", 0);
}

void Storage::incrementJiggles(JigglerConfig& cfg) {
    cfg.totalJiggles++;
    if (cfg.totalJiggles % 50 == 0) { // Save every 50 to reduce flash wear
        _prefs.putUInt("totalJ", cfg.totalJiggles);
    }
}

void Storage::markFirstRunDone(JigglerConfig& cfg) {
    cfg.firstRun = false;
    _prefs.putBool("firstRun", false);
}
