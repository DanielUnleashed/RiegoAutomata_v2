#include <Input.h>

Input::Input(uint8_t pin, uint32_t delay, bool biestable, uint32_t validationTime){
  digitalWrite(pin, LOW);
  pinMode(pin, INPUT);
  this->pin = pin;
  this->delayTime = delay;
  this->biestableInput = biestable;
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
  uint32_t currentTime = millis();
  if(currentState){ // Check HIGH input
    if(!isPressed) startPressTime = millis();

    if(isPressed && (currentTime - startPressTime)>validationTime){
        if(biestableInput && (currentTime - startTime)<delayTime){ // Button was already pressed
          startTime = 0; // Release the button
          isHigh = false;
        }else{
          startTime = currentTime;
          isHigh = true;
        }
        isPressed = true;
    }
  }else{  // Check LOW input
    if(isPressed) startReleaseTime = currentTime;

    if(!isPressed && (currentTime - startReleaseTime)>validationTime){
        isPressed = false;
        isHigh = false;
    }
  }
  isPressed = currentState;

  // Maintain output high if there's delay time  
  if(!isPressed){
    if((currentTime - startTime)<delayTime){
        isHigh = true;
    }else{
        isHigh = false;
    }
  }
}