#include <Arduino.h>
#include "SketchUploader/SketchUploader.h"
#include "Input.h"
#include "Utils.h"

#include "FirebaseServer.h"

#include <NTPClient.h>

// Ultrasound sensor (deposit)
#define TRIGGER_PIN 32
#define ECHO_PIN 35
#define SOUND_SPEED 0.034
#define ULTRASOUND_ITERATIONS 5

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
const char* password = "270726VorGes_69#";

#define LED_TIME 20*1000
String ledMode = "Off";
uint8_t ledBrightness = 255;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

double depositLevel = 0.0;

#define DEPOSIT_LEVEL "deposit/level"

#define LED_BRIGHTNESS "led/brightness"
#define LED_LED "led/led"
#define LED_MODE "led/mode"
#define LED_PRESENCE "led/presence"
#define LED_PRESENCEREADING "led/presencereading"
#define LED_PRESENCETIME "led/presencetime"

#define WATER_ALARM_1 "water/alarm_1"
#define WATER_ALARMTIME_1 "water/alarmtime_1"
#define WATER_LASTWATERING "water/lastwatering"
#define WATER_NOW "water/now"
#define WATER_TIME "water/time"

#define MESSAGES "messages"

FirebaseVariable<double> deposit_level(DEPOSIT_LEVEL, 0);

FirebaseVariable<int> led_brightness(LED_BRIGHTNESS, 100);
FirebaseVariable<bool> led_led(LED_LED, true);
FirebaseVariable<String> led_mode(LED_MODE, "Continuo");
FirebaseVariable<bool> led_presence(LED_PRESENCE, true);
FirebaseVariable<bool> led_presencereading(LED_PRESENCEREADING, false);
FirebaseVariable<int> led_presencetime(LED_PRESENCETIME, 10);

FirebaseVariable<bool> water_alarm_1(WATER_ALARM_1, true);
FirebaseVariable<String> water_alarmtime_1(WATER_ALARMTIME_1, "00:00");
FirebaseVariable<String> water_lastwatering(WATER_LASTWATERING, "00:00");
FirebaseVariable<String> water_now(WATER_NOW, "off");
FirebaseVariable<int> water_time(WATER_TIME, 30);

FirebaseVariable<String> messages(MESSAGES, "");

uint32_t measureUltrasoundDistance() {
  uint32_t distanceSum = 0;
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(10);

  for(uint8_t i = 0; i < ULTRASOUND_ITERATIONS; i++){
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);
      
    uint32_t duration = pulseIn(ECHO_PIN, HIGH);
    if(duration==0) return -1;
      
    distanceSum += duration;
    delay(20);
  }
  return distanceSum/ULTRASOUND_ITERATIONS;
}

double updateDepositLevel(){
  static uint32_t lastTimeHere = 0;

  if(millis() - lastTimeHere < 2000) return depositLevel;
  else lastTimeHere = millis();

  depositLevel = 137.06572 - measureUltrasoundDistance()*0.04639;
  Serial.printf("Deposit level: %d\n", depositLevel);
  deposit_level.setValue(depositLevel);
  return depositLevel;
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
    delay(4);
  }
  motorsRunning = false;
}

Input presence(PRESENCE_PIN, LED_TIME, false, 2000);
Input motorButton(BUTTON, MOTOR_TIME, true, 500);
bool lastLightsState = false;

uint32_t startWateringTime = 0;
bool startWateringLock = false;
bool waterAlarmLock = false;
uint16_t wateringAlarmDuration = 0; 

uint8_t hourAlarm, minuteAlarm;

void updateParameters(){
  String messageFromServer = messages.getValue();
  if(messageFromServer == "update_deposit"){
    updateDepositLevel();
  }
  messages.setValue("none");

  bool forceLightsOn = led_led.getValue();
  bool lightsOn = forceLightsOn;
  presence.delayTime = led_presencetime.getValue()*1000;
  if(lightsOn){
    bool printMessage = led_mode.hasNewValue || led_brightness.hasNewValue;
    ledMode = led_mode.getValue();
    ledBrightness = led_brightness.getValue()*2.55;
    if(!lastLightsState || printMessage) SU.log("Lights on. Mode " + ledMode + " (" + String(ledBrightness) + ")");
  }else{
    ledMode = "Off";
    if(lastLightsState) SU.log("Lights off");
  }
  lastLightsState = lightsOn;


  // Water alarm
  String wateringTimeAlarm = water_alarmtime_1.getValue();
  bool waterAlarmStart = false;
  if(wateringTimeAlarm != "" && water_alarm_1.getValue()){
    uint8_t indexOfTwoDots = wateringTimeAlarm.indexOf(':');
    hourAlarm = wateringTimeAlarm.substring(0, indexOfTwoDots).toInt();
    minuteAlarm = wateringTimeAlarm.substring(indexOfTwoDots+1, wateringTimeAlarm.length()).toInt();
    waterAlarmStart = timeClient.getHours()==hourAlarm && timeClient.getMinutes()==minuteAlarm;

    // This lock is needed so the water alarm doesn't get triggered again in the same minute if the watering time is less than 60s.
    if(waterAlarmLock && timeClient.getMinutes()!=minuteAlarm){
      waterAlarmLock = false;
    }
  }

  bool requestWatering = (waterAlarmStart && !waterAlarmLock) || water_now.getValue()=="go";

  bool startWateringIsValid = false;
  if(water_now.getValue() == "stop" || (requestWatering && updateDepositLevel()<10.0)){
    wateringAlarmDuration = 0;
    startWateringTime = 0;
  }else{
    if(!startWateringLock && requestWatering){ 
      startWateringTime = millis();
      wateringAlarmDuration = water_time.getValue();
      water_lastwatering.setValue(timeClient.getFormattedTime());

      startWateringLock = true;
      if(waterAlarmStart) waterAlarmLock = true;
    }
    startWateringIsValid =  (startWateringTime + wateringAlarmDuration*1000) > millis();
  }

  // Finally release the lock when the time from the first button is pressed has passed.
  if(!startWateringIsValid) startWateringLock = false;

  bool motorState = (startWateringIsValid || motorButton.inputHigh()) && updateDepositLevel()>10;
  if(motorState){
    if(!motorsRunning){
      bootUpMotors();
      SU.log("Motors on");
      water_now.setValue("on");
    }
  }else{
    if(motorsRunning){
      bootDownMotors();
      SU.log("Motors off");
      water_now.setValue("off");
    }
  }
}

void createAlarmTask(){
  BaseType_t xReturn = xTaskCreate([](void* funcParams){
    for(;;){
      bool waterAlarmStart = timeClient.getHours()==hourAlarm && timeClient.getMinutes()==minuteAlarm;
      if(waterAlarmStart){
        updateParameters(); // Para comenzar el riego
        delay((wateringAlarmDuration+1)*1000);
        updateParameters(); // Para detener el riego
      }else{
        delay((60 - timeClient.getSeconds())*1000); // Wait for the next minute
      }
    }
  }, "WaterAlarm Task", 5000, NULL, 1, NULL);
  if(xReturn != pdTRUE) Serial.println("Couldn't create WaterAlarm Task!");
}

void createLEDTask(){
  BaseType_t xReturn = xTaskCreatePinnedToCore([](void *funcParams){
    for(;;){
      if(ledMode == "Continuo" || presence.inputHigh() && led_presence.getValue()){
        ledcWrite(0, ledBrightness);
      }else if(ledMode == "Seno"){
        double omega = map(ledBrightness, 0, 255, 0, TWO_PI*2);
        ledcWrite(0, (uint32_t) (128.0*(1.0 + sin(omega*(millis()/1000.0)))) );
      }else if(ledMode == "Parpadeo"){
        double omega = map(ledBrightness, 0, 255, 0, TWO_PI*2);
        ledcWrite(0, (sin(omega*millis()/1000.0)>0)?255:0);
      }else{
        ledcWrite(0,0);
      }

      led_presencereading.setValue(presence.inputPressed());

      if(motorsRunning){
        digitalWrite(DEPOSIT_LED, HIGH);
      }else{
        digitalWrite(DEPOSIT_LED, LOW);
      }
      delay(34);
    }
  }, "LedTask", 4000, NULL, 1, NULL, 0);
  if(xReturn != pdTRUE) Serial.println("Couldn't create LedTask!");
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(DEPOSIT_LED, OUTPUT);

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.printf("Connecting to %s", ssid);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED && millis()<45000) {
    delay(500);
    Serial.print(".");
  }

  if(WiFi.status() != WL_CONNECTED) ESP.restart();

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  timeClient.setTimeOffset(3600); // GMT+1 Horario de invierno

  // Sketch Uploader
  SU.startServer(&timeClient);
  
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

  createLEDTask();
  createAlarmTask();

  updateDepositLevel();

  firebase.setString("water/now", "off");
}

void loop() {
  firebase.updateFirebase(updateParameters);

  // This is the only signal that doesn't come from the server.
  if(motorButton.inputHigh()) updateParameters();
}