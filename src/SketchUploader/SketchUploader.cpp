#include "SketchUploader.h"

AsyncWebServer SketchUploader::server(80);
AsyncEventSource SketchUploader::events("/events");

SketchUploader SU;

SketchUploader::SketchUploader(){}

void SketchUploader::startServer(NTPClient* tc){
  timeClient = tc;

  const char* host = "esp32";
    if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  MDNS.addService(host, "udp", 5600);
  Serial.printf("mDNS responder started. Search by service: %s\n", host);

  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if(!request->authenticate(USERNAME, PASSWORD)){
      return request->requestAuthentication();
    }
    //server.sendHeader("Connection", "close");
    request->send(200, "text/html", server_html);
  });

  server.on("/logout", [](AsyncWebServerRequest *request){
    request->send(401);
  });
  
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
    if(!request->authenticate(USERNAME, PASSWORD)){
      return request->requestAuthentication();
    }
    //server.sendHeader("Connection", "close");
    //request->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
      String message = "Uploading: "+ filename + "\n";
      SU.log(message);
      Serial.print(message);
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } 
    
    if (len) {
      /* flashing firmware to ESP*/
      if (Update.write(data, len) != len) {
        Update.printError(Serial);
      }
    }
    
    if (final) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u bytes\nRebooting...\n", index+len);
        SU.log("Update Success!");
      } else {
        Update.printError(Serial);
      }
    }
  });

  server.on("/fetchLastLogs", [](AsyncWebServerRequest *request){
    SU.retrieveLastLogs();
  });

  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  server.begin();

  log("Server ready!");
}

void SketchUploader::log(String str){
  static uint32_t lastTimeLog = 0;
  static uint8_t accumulatedMessages = 0;
  uint32_t startTime = millis();

  while(!timeClient->update() && (millis()-startTime)<2000){
    timeClient->forceUpdate();
  }
  String formDate = timeClient->getFormattedDate();
  //Example:  2018-05-28T16:00:13Z
  String date = Utils::dateFormater(formDate);

  if(logPointer == MAX_LOG_MEMORY-1){
    for(uint8_t i = 0; i < MAX_LOG_MEMORY-1; i++){
      lastLogs[i] = lastLogs[i+1];
    }
    logPointer--;
  }
  String message = date + " > " + str;
  lastLogs[logPointer++] = message;

  Serial.println(message);

  accumulatedMessages++;
    // Wait for 250 ms between messages
  if((startTime-lastTimeLog) < 250){
    return;
  }
  for(uint8_t i = logPointer-accumulatedMessages; i < logPointer; i++){
    events.send(lastLogs[i].c_str(), "console", startTime);
  }
  accumulatedMessages = 0;

  lastTimeLog = startTime;
}

void SketchUploader::retrieveLastLogs(){
  events.send("******", "console", millis());
  for(uint8_t i = 0; i < logPointer; i++){
    events.send(lastLogs[i].c_str(), "console", millis());
  }
  events.send("******", "console", millis());
}