#ifndef FIREBASE_SERVER_h
#define FIREBASE_SERVER_h

#include "Arduino.h"
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#include "SketchUploader/SketchUploader.h"

#define API_KEY "AIzaSyDSoplVaOu7G2scs2VRdsy-WdoT7sespDk"
#define DATABASE_URL "https://riego-automata-default-rtdb.europe-west1.firebasedatabase.app/"

#define USER_EMAIL "dbejarcaballero@gmail.com"
#define USER_PASSWORD "D4ni31 B3j4r."

#define TIME_BETWEEN_CONECTIONS 50

class FirebaseServer {

    public:
    FirebaseServer();
    void startFirebase();

    /* getBool() fetches a boolean from the server. Saves in @param ok if the connection went ok.
    */
    bool getBool(String path, bool outputIfError, bool waitForResponse=false);
    bool setBool(String path, bool value, bool waitForResponse=false);
    
    int getInt(String path, int outputIfError, bool waitForResponse=false);
    bool setInt(String path, int value, bool waitForResponse=false);
    
    String getString(String path, String outputIfError, bool waitForResponse=false);
    bool setString(String path, String value, bool waitForResponse=false);

    private:
    FirebaseData fbdo;
    FirebaseAuth auth;
    FirebaseConfig config;

    bool signUp = false;

    uint32_t lastTimeConnection = 0;
    bool checkConnection(bool waitForResponse, bool &ok);

};

extern FirebaseServer firebase;

#endif