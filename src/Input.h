#ifndef INPUT_h
#define INPUT_h

#include "Arduino.h"

#define NORMAL_PRESS_TIME 500

class Input {
    public:
    Input(uint8_t pin, uint32_t delay=0, uint32_t validationTime=NORMAL_PRESS_TIME);

    bool inputPressed();
    bool inputHigh();

    private:
    uint8_t pin;
    uint32_t startPressTime = 0;    // Time when first detected input (LOW to HIGH)
    uint32_t startReleaseTime = 0;  // Time when first detected release (HIGH to LOW)
    uint32_t startTime = 0;         // Start time when input is decided to be valid and so the button is properly pressed.
    bool isPressed = false;         // True when totally pressed.
    bool isHigh = false;            // True when pressed and during delay time.

    uint32_t validationTime;
    uint32_t delayTime;

    void fetchInput();


};

#endif 