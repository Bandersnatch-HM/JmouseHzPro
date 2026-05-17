#include "movement.h"
#include <math.h>

void MovementEngine::begin(uint8_t mode, uint32_t interval, uint8_t amplitude,
                           uint8_t variationPct, bool humanPauses) {
    _mode = mode;
    _baseInterval = interval;
    _amplitude = constrain(amplitude, 1, 10);
    _variationPct = constrain(variationPct, 0, 80);
    _humanPauses = humanPauses;
    _lastMove = millis();
    _calcNextInterval();
    _nextPauseAt = random(PAUSE_EVERY_MIN, PAUSE_EVERY_MAX + 1);
}

void MovementEngine::setMode(uint8_t mode) { _mode = mode % MODE_COUNT; }
void MovementEngine::setInterval(uint32_t ms) { _baseInterval = ms; _calcNextInterval(); }
void MovementEngine::setAmplitude(uint8_t px) { _amplitude = constrain(px, 1, 10); }
void MovementEngine::setVariation(uint8_t pct) { _variationPct = constrain(pct, 0, 80); }
void MovementEngine::setHumanPauses(bool en) { _humanPauses = en; }

void MovementEngine::_calcNextInterval() {
    if (_variationPct == 0) {
        _nextInterval = _baseInterval;
        return;
    }
    int32_t variation = (_baseInterval * _variationPct) / 100;
    _nextInterval = _baseInterval + random(-variation, variation + 1);
    if (_nextInterval < 2000) _nextInterval = 2000;
}

void MovementEngine::_checkPause() {
    if (!_humanPauses) return;
    _jiggleCount++;
    if (_jiggleCount >= _nextPauseAt) {
        _jiggleCount = 0;
        _nextPauseAt = random(PAUSE_EVERY_MIN, PAUSE_EVERY_MAX + 1);
        uint32_t pauseDur = random(PAUSE_DURATION_MIN, PAUSE_DURATION_MAX + 1);
        _pauseUntil = millis() + pauseDur;
        DBGF("[Movement] Human pause for %lu ms\n", pauseDur);
    }
}

bool MovementEngine::shouldMove() {
    if (millis() < _pauseUntil) return false;
    if ((millis() - _lastMove) >= _nextInterval) {
        _lastMove = millis();
        _calcNextInterval();
        _checkPause();
        return true;
    }
    return false;
}

int8_t MovementEngine::_clamp(float v) {
    if (v > 127) return 127;
    if (v < -127) return -127;
    return (int8_t)v;
}

// --- Movement Generators ---

void MovementEngine::_genMicroJiggle(MoveStep* s, uint8_t& c) {
    s[0] = {1, 0, 10};
    s[1] = {-1, 0, 10};
    c = 2;
}

void MovementEngine::_genHorizontal(MoveStep* s, uint8_t& c) {
    int8_t a = _amplitude;
    s[0] = {a, 0, 15};
    s[1] = {(int8_t)-a, 0, 15};
    c = 2;
}

void MovementEngine::_genVertical(MoveStep* s, uint8_t& c) {
    int8_t a = _amplitude;
    s[0] = {0, a, 15};
    s[1] = {0, (int8_t)-a, 15};
    c = 2;
}

void MovementEngine::_genCross(MoveStep* s, uint8_t& c) {
    int8_t a = _amplitude;
    s[0] = {a, 0, 12};
    s[1] = {0, a, 12};
    s[2] = {(int8_t)-a, 0, 12};
    s[3] = {0, (int8_t)-a, 12};
    c = 4;
}

void MovementEngine::_genBezier(MoveStep* s, uint8_t& c) {
    // Quadratic Bezier: P0(0,0) -> P1(random) -> P2(0,0)
    float p1x = (float)random(-_amplitude * 2, _amplitude * 2 + 1);
    float p1y = (float)random(-_amplitude * 2, _amplitude * 2 + 1);
    int steps = 8;
    float prevX = 0, prevY = 0;
    c = 0;
    for (int i = 1; i <= steps; i++) {
        float t = (float)i / steps;
        float u = 1.0f - t;
        // Bezier: B(t) = u^2*P0 + 2*u*t*P1 + t^2*P2, P0=P2=(0,0)
        float x = 2 * u * t * p1x;
        float y = 2 * u * t * p1y;
        float dx = x - prevX;
        float dy = y - prevY;
        s[c] = {_clamp(dx), _clamp(dy), 20};
        c++;
        prevX = x;
        prevY = y;
    }
    // Return to origin
    s[c] = {_clamp(-prevX), _clamp(-prevY), 15};
    c++;
}

void MovementEngine::_genCircle(MoveStep* s, uint8_t& c) {
    float r = _amplitude;
    int steps = 12;
    float prevX = r, prevY = 0;
    c = 0;
    for (int i = 1; i <= steps; i++) {
        float angle = (2.0f * PI * i) / steps;
        float x = r * cos(angle);
        float y = r * sin(angle);
        s[c] = {_clamp(x - prevX), _clamp(y - prevY), 15};
        c++;
        prevX = x;
        prevY = y;
    }
}

void MovementEngine::_genNaturalDrift(MoveStep* s, uint8_t& c) {
    c = 0;
    int totalX = 0, totalY = 0;
    int steps = random(3, 7);
    for (int i = 0; i < steps && c < 14; i++) {
        int8_t dx = random(-_amplitude, _amplitude + 1);
        int8_t dy = random(-_amplitude, _amplitude + 1);
        totalX += dx;
        totalY += dy;
        s[c] = {dx, dy, (uint8_t)random(10, 40), 0, 0};
        c++;
    }
    // Return to origin
    s[c] = {(int8_t)-totalX, (int8_t)-totalY, 15, 0, 0};
    c++;
}

void MovementEngine::_genFullScreenShift(MoveStep* s, uint8_t& c) {
    c = 0;
    int totalX = 0, totalY = 0;
    int steps = 6;
    for (int i = 0; i < steps; i++) {
        // Movimientos grandes (amplitud multiplicada por 8 para barrer toda la pantalla)
        int8_t dx = random(-_amplitude * 8, _amplitude * 8 + 1);
        int8_t dy = random(-_amplitude * 8, _amplitude * 8 + 1);
        totalX += dx;
        totalY += dy;
        // En el primer step se pulsa Shift (0x81), en el penúltimo se suelta
        uint8_t kPress = (i == 0) ? 0x81 : 0;
        uint8_t kRel = (i == steps - 2) ? 0x81 : 0;
        s[c] = {dx, dy, (uint8_t)random(20, 50), kPress, kRel};
        c++;
    }
    // Retornar al centro aproximadamente sin pulsar teclas
    s[c] = {(int8_t)-totalX, (int8_t)-totalY, 30, 0, 0};
    c++;
}

void MovementEngine::getNextSequence(MoveStep* steps, uint8_t& count) {
    uint8_t mode = _mode;
    if (mode == MODE_RANDOM_MIX) {
        mode = random(0, MODE_RANDOM_MIX); // Pick 0-6
    }
    switch (mode) {
        case MODE_MICRO_JIGGLE:      _genMicroJiggle(steps, count); break;
        case MODE_HORIZONTAL:        _genHorizontal(steps, count); break;
        case MODE_VERTICAL:          _genVertical(steps, count); break;
        case MODE_CROSS:             _genCross(steps, count); break;
        case MODE_BEZIER:            _genBezier(steps, count); break;
        case MODE_CIRCLE:            _genCircle(steps, count); break;
        case MODE_NATURAL_DRIFT:     _genNaturalDrift(steps, count); break;
        case MODE_FULL_SCREEN_SHIFT: _genFullScreenShift(steps, count); break;
        default:                     _genMicroJiggle(steps, count); break;
    }
}
