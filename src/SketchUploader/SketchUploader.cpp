#include "SketchUploader.h"

AsyncWebServer SketchUploader::server(80);
AsyncEventSource SketchUploader::events("/events");

WiFiUDP SketchUploader::ntpUDP;
NTPClient SketchUploader::timeClient(SketchUploader::ntpUDP);

SketchUploader SU;

SketchUploader::SketchUploader(){}

void SketchUploader::startServer(){
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

  timeClient.begin();
  timeClient.setTimeOffset(3600*2); // GMT+2 Horario de verano, GMT+1 Horario de invierno

  log("Server ready!");
}

void SketchUploader::log(String str){
  static uint32_t lastTimeLog = 0;
  uint32_t startTime = millis();

  // Wait for 500 ms between messages
  if((startTime-lastTimeLog) < 250) return;

  while(!timeClient.update() && (millis()-startTime)<2000){
    timeClient.forceUpdate();
  }
  String formDate = timeClient.getFormattedDate();
  //Example:  2018-05-28T16:00:13Z

  int splitT = formDate.indexOf("T");
  int splitBar = formDate.indexOf("-");
  String date = formDate.substring(splitBar+1, splitT); // Month and day only
  int splitBar2 = date.indexOf("-");
  date = date.substring(splitBar2+1, date.length()) + "/" + date.substring(0, splitBar2);
  String timeStamp = formDate.substring(splitT+1, formDate.length()-1);

  if(logPointer == MAX_LOG_MEMORY-1){
    for(uint8_t i = 0; i < MAX_LOG_MEMORY-1; i++){
      lastLogs[i] = lastLogs[i+1];
    }
    logPointer--;
  }
  String message = date + " " + timeStamp + " > " + str;
  lastLogs[logPointer++] = message;
  events.send(message.c_str(), "console", startTime);

  Serial.println(str);

  lastTimeLog = startTime;
}

void SketchUploader::retrieveLastLogs(){
  events.send("******", "console", millis());
  for(uint8_t i = 0; i < logPointer; i++){
    events.send(lastLogs[i].c_str(), "console", millis());
  }
  events.send("******", "console", millis());
}