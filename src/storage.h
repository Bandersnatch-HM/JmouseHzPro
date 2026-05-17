#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <Preferences.h>
#include "config.h"

struct JigglerConfig {
    char deviceName[32];
    uint8_t moveMode;
    uint32_t moveInterval;
    uint8_t moveAmplitude;
    uint8_t variationPercent;
    bool humanPauses;
    bool antiDetection;
    bool firstRun;
    uint32_t totalJiggles;
    char wifiSSID[32];
    char wifiPass[64];
};

class Storage {
public:
    void begin();
    void loadConfig(JigglerConfig& cfg);
    void saveConfig(const JigglerConfig& cfg);
    void resetToDefaults(JigglerConfig& cfg);
    void incrementJiggles(JigglerConfig& cfg);
    void markFirstRunDone(JigglerConfig& cfg);

private:
    Preferences _prefs;
};

#endif
