#include <Arduino.h>
#include "SketchUploader/SketchUploader.h"
#include "Input.h"

#include "FirebaseServer.h"

// Ultrasound sensor (deposit)
#define TRIGGER_PIN 32
#define ECHO_PIN 35
#define SOUND_SPEED 0.034
#define ULTRASOUND_ITERATIONS 1

// Deposit LED
#define DEPOSIT_LED 33

// Presence sensor
#define PRESENCE_PIN 34
#define LEDSTRIP_PIN 13

// Motor pins
#define MOTOR_A1 14 
#define MOTOR_A2 22
#define MOTOR_B1 26
#define MOTOR_B2 27

#define MOTOR_A_PWM MOTOR_A1
#define MOTOR_A_DIR MOTOR_A2
#define MOTOR_B_PWM MOTOR_B1
#define MOTOR_B_DIR MOTOR_B2

#define MOTOR_TIME 30*1000

#define BUTTON 25

const char* ssid = "Aitina";
const char* password = "270726VorGes_69*";

#define LED_TIME 20*1000
  
double measureUltrasoundDistance() {
  double distanceSum = 0;
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(10);

  for(int i = 0; i < ULTRASOUND_ITERATIONS; i++){
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);
      
    double duration = pulseIn(ECHO_PIN, HIGH);
      
    distanceSum += duration*SOUND_SPEED/2;
    delay(1);
  }
  return distanceSum/ULTRASOUND_ITERATIONS;
}

bool motorsRunning = false;
void bootUpMotors(){
  /*for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle++){   
    ledcWrite(1, dutyCycle);
    ledcWrite(2, dutyCycle);
    delay(15);
  }*/
  ledcWrite(1, 255);
  ledcWrite(2, 255);
  motorsRunning = true;
}

void bootDownMotors(){
  for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
    ledcWrite(1, dutyCycle);
    ledcWrite(2, dutyCycle); 
    delay(15);
  }
  motorsRunning = false;
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(DEPOSIT_LED, OUTPUT);

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Sketch Uploader
  SU.startServer();
  firebase.startFirebase();

  pinMode(PRESENCE_PIN, INPUT);
  pinMode(LEDSTRIP_PIN, OUTPUT);

  pinMode(MOTOR_A_PWM, OUTPUT);
  pinMode(MOTOR_A_DIR, OUTPUT);
  pinMode(MOTOR_B_PWM, OUTPUT);
  pinMode(MOTOR_B_DIR, OUTPUT);

  // PWM Channels:
  // 0 -> Led strip
  // 1 -> Motor A speed
  ledcSetup(0, 5000, 8);
  ledcAttachPin(LEDSTRIP_PIN, 0);

  ledcSetup(1, 5000, 8);
  ledcAttachPin(MOTOR_A_PWM, 1);
  ledcSetup(2, 5000, 8);
  ledcAttachPin(MOTOR_B_PWM, 2);
}

Input presence(PRESENCE_PIN, LED_TIME, false, 8000);
Input motorButton(BUTTON, MOTOR_TIME, true, 500);
bool lastLightsState = false;

void loop() {
  bool forceLightsOn = firebase.getBool("led/led", false);
  bool listenToPresenceSensor = firebase.getBool("led/presence", true);
  bool lightsOn = forceLightsOn || (presence.inputHigh() && listenToPresenceSensor);
  if(lightsOn){
    if(!lastLightsState) //SU.log("Lights on");
    ledcWrite(0, 255);
  }else{
    if(lastLightsState) //SU.log("Lights off");
    ledcWrite(0, 0);
  }
  lastLightsState = lightsOn;

  bool motorState = motorButton.inputHigh();
  if(motorState){
    if(!motorsRunning){
      bootUpMotors();
      SU.log("Motors on");
      firebase.setBool("test/motor", true);
    }
    ledcWrite(1, 255);
    ledcWrite(2, 255);
  }else{
    if(motorsRunning){
      bootDownMotors();
      SU.log("Motors off");
      firebase.setBool("test/motor", true);
    }
    ledcWrite(1, 0);
    ledcWrite(2, 0);
  }

  delay(1000);
}

/*
  xTaskCreatePinnedToCore([](void *funcParams){
      for(;;){
        for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle++){   
          ledcWrite(0, dutyCycle);
          delay(15);
        }

        // decrease the LED brightness
        for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
          ledcWrite(0, dutyCycle);   
          delay(15);
        }
      }
  }, "blink", 2000, NULL, 1, NULL, 0);*/