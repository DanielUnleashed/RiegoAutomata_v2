#ifndef FIREBASE_SERVER_h
#define FIREBASE_SERVER_h

#include "Arduino.h"
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#include "SketchUploader/SketchUploader.h"

#include "FirebaseVariable.h"
#include <vector>
#include <functional>

#define API_KEY "AIzaSyDSoplVaOu7G2scs2VRdsy-WdoT7sespDk"
#define DATABASE_URL "https://riego-automata-default-rtdb.europe-west1.firebasedatabase.app/"

#define USER_EMAIL "dbejarcaballero@gmail.com"
#define USER_PASSWORD "D4ni31"

#define TIME_BETWEEN_CHECKS 1000
#define TIME_BETWEEN_CHECKS_SLEEP 5000
#define SLEEP_TIME 3*60*1000 // Time since last connection that will make the task enter sleep mode

class FirebaseServer {

    public:
    FirebaseServer();
    void startFirebase();
    void updateFirebase(std::function<void(void)> func);

    bool getBool(String path, bool outputIfError, bool waitForResponse=false);
    bool setBool(String path, bool value, bool waitForResponse=false);
    
    int getInt(String path, int outputIfError, bool waitForResponse=false);
    bool setInt(String path, int value, bool waitForResponse=false);
    
    String getString(String path, String outputIfError, bool waitForResponse=false);
    bool setString(String path, String value, bool waitForResponse=false);

    double getDouble(String path, double outputIfError, bool waitForResponse=false);
    bool setDouble(String path, double value, bool waitForResponse=false);

    bool updateVariable(String path);

    private:
    FirebaseData fbdo;
    FirebaseAuth auth;
    FirebaseConfig config;

    String userID = "";

    uint32_t lastTimeConnection = 0;
    bool checkConnection(bool waitForResponse, bool &ok);

};

extern FirebaseServer firebase;

#endif