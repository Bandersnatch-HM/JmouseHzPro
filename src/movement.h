#ifndef MOVEMENT_H
#define MOVEMENT_H

#include <Arduino.h>
#include "config.h"

struct MoveStep {
    int8_t dx;
    int8_t dy;
    uint8_t delayMs; // delay after this step
    uint8_t keyPress;   // 0 if none, e.g. 0x81 (KEY_LEFT_SHIFT)
    uint8_t keyRelease; // 0 if none, e.g. 0x81
};

class MovementEngine {
public:
    void begin(uint8_t mode, uint32_t interval, uint8_t amplitude,
               uint8_t variationPct, bool humanPauses);
    void setMode(uint8_t mode);
    void setInterval(uint32_t ms);
    void setAmplitude(uint8_t px);
    void setVariation(uint8_t pct);
    void setHumanPauses(bool en);
    bool shouldMove();
    void getNextSequence(MoveStep* steps, uint8_t& count);
    uint8_t getMode() { return _mode; }

private:
    uint8_t _mode = MODE_RANDOM_MIX;
    uint32_t _baseInterval = 30000;
    uint8_t _amplitude = 2;
    uint8_t _variationPct = 30;
    bool _humanPauses = true;

    unsigned long _lastMove = 0;
    uint32_t _nextInterval = 30000;
    uint32_t _jiggleCount = 0;
    uint32_t _pauseUntil = 0;
    uint32_t _nextPauseAt = 0;

    void _calcNextInterval();
    void _checkPause();
    void _genMicroJiggle(MoveStep* s, uint8_t& c);
    void _genRandomScreen(MoveStep* s, uint8_t& c);
    void _genVertical(MoveStep* s, uint8_t& c);
    void _genCross(MoveStep* s, uint8_t& c);
    void _genBezier(MoveStep* s, uint8_t& c);
    void _genCircle(MoveStep* s, uint8_t& c);
    void _genNaturalDrift(MoveStep* s, uint8_t& c);
    void _genFullScreenShift(MoveStep* s, uint8_t& c);
    int8_t _clamp(float v);
};

#endif
