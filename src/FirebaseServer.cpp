#include "FirebaseServer.h"

FirebaseServer firebase;

FirebaseServer::FirebaseServer(){}

void FirebaseServer::startFirebase(){
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;

    if(Firebase.signUp(&config, &auth, "", "")){
        Serial.println("User entered");
        signUp = true;
    }else{
        Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }

    config.token_status_callback = tokenStatusCallback;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    SU.log("Firebase ready!");
}

bool FirebaseServer::getBool(String path, bool outputIfError, bool waitForResponse){
    bool ok = false;
    if(!checkConnection(waitForResponse, ok)) return outputIfError;
    if(Firebase.RTDB.getBool(&fbdo, path.c_str()) && fbdo.dataType() == "bool"){
        ok = true;
        return fbdo.boolData();
    }else{
        String mssg = "Fail to fetch bool to " +  path + " (" + fbdo.errorReason() + ")";
        Serial.println(mssg);
        ////SU.log(mssg);
        ok = false;
    }
    return outputIfError;
}

bool FirebaseServer::setBool(String path, bool value, bool waitForResponse){
    bool ok = false;
    if(!checkConnection(waitForResponse, ok)) return ok;
    if(Firebase.RTDB.setBool(&fbdo, path.c_str(), value)){
        ok = true;
    }else{
        String mssg = "Fail to send bool " + String(value) + " to " +  path + " (" + fbdo.errorReason() + ")";
        Serial.println(mssg);
        //SU.log(mssg);
        ok = false;
    }
    return ok;
}

int FirebaseServer::getInt(String path, int outputIfError, bool waitForResponse){
    bool ok = false;
    if(!checkConnection(waitForResponse, ok)) return outputIfError;
    if(Firebase.RTDB.getInt(&fbdo, path.c_str()) && fbdo.dataType() == "int"){
        ok = true;
        return fbdo.intData();
    }else{
        String mssg = "Fail to fetch int to " +  path + " (" + fbdo.errorReason() + ")";
        Serial.println(mssg);
        //SU.log(mssg);
        ok = false;
    }
    return outputIfError;

}

bool FirebaseServer::setInt(String path, int value, bool waitForResponse){
    bool ok = false;
    if(!checkConnection(waitForResponse, ok)) return ok;
    if(Firebase.RTDB.setInt(&fbdo, path.c_str(), value)){
        ok = true;
    }else{
        String mssg = "Fail to send int " + String(value) + " to " +  path + " (" + fbdo.errorReason() + ")";
        Serial.println(mssg);
        //SU.log(mssg);
        ok = false;
    }
    return ok;
}

String FirebaseServer::getString(String path, String outputIfError, bool waitForResponse){
    bool ok = false;
    if(!checkConnection(waitForResponse, ok)) return outputIfError;
    if(Firebase.RTDB.getString(&fbdo, path.c_str()) && fbdo.dataType() == "string"){
        ok = true;
        return fbdo.stringData();
    }else{
        String mssg = "Fail to fetch to " +  path + " (" + fbdo.errorReason() + ")";
        Serial.println(mssg);
        //SU.log(mssg);
        ok = false;
    }
    return outputIfError;
}

bool FirebaseServer::setString(String path, String value, bool waitForResponse){
    bool ok = false;
    if(!checkConnection(waitForResponse, ok)) return ok;
    if(Firebase.RTDB.setString(&fbdo, path.c_str(), value)){
        ok = true;
    }else{
        String mssg = "Fail to send " + String(value) + " to " +  path + " (" + fbdo.errorReason() + ")";
        Serial.println(mssg);
        //SU.log(mssg);
        ok = false;
    }
    return ok;
}

bool FirebaseServer::checkConnection(bool waitForResponse, bool &ok){
    uint32_t timeNow = millis();
    bool timeIntervalLongEnough = (timeNow-lastTimeConnection)>TIME_BETWEEN_CONECTIONS;
    if(waitForResponse && !timeIntervalLongEnough){
        delay(timeNow-lastTimeConnection);
        timeIntervalLongEnough = true;
    }
    bool ans = Firebase.ready() && signUp && timeIntervalLongEnough;
    ok = ans;
    if(ans) lastTimeConnection = timeNow;
    return ans;
}