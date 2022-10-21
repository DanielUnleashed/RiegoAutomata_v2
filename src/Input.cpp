#include <Input.h>

Input::Input(uint8_t pin, uint32_t delay, uint32_t validationTime){
    pinMode(pin, INPUT);
    this->pin = pin;
    this->delayTime = delay;
    this->validationTime = validationTime;
}

bool Input::inputPressed(){
    fetchInput();
    return this->isPressed;
}

bool Input::inputHigh(){
    fetchInput();
    return this->isHigh;
}

void Input::fetchInput(){
  bool currentState = digitalRead(pin);
  if(currentState){
    if(!isPressed) startPressTime = millis();

    if(isPressed && (millis() - startPressTime)>validationTime){
        startTime = millis();
        isPressed = true;
        isHigh = true;
    }
  }else{
    if(isPressed) startReleaseTime = millis();

    if(!isPressed && (millis() - startReleaseTime)>validationTime){
        isPressed = false;
        isHigh = false;
    }
  }
  isPressed = currentState;

    // Maintain output high if there's delay time
  if(!isPressed){
    if((millis() - startTime)<delayTime){
        isHigh = true;
    }else{
        isHigh = false;
    }
  }
}