#include "FirebaseServer.h"

FirebaseServer firebase;

FirebaseServer::FirebaseServer(){}

void FirebaseServer::startFirebase(){
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;

    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(4096);

    config.token_status_callback = tokenStatusCallback;
    config.max_token_generation_retry = 5;

    Firebase.begin(&config, &auth);

    Serial.print("Getting user token");
    while(auth.token.uid == ""){
        Serial.print(".");
        delay(1000);
    }

    this->userID = auth.token.uid.c_str();
    Serial.printf(" TOKEN: %s\n", this->userID.c_str());

    FirebaseVariable<int>::linkAllVariables();
    FirebaseVariable<bool>::linkAllVariables();
    FirebaseVariable<String>::linkAllVariables();

    SU.log("Firebase ready! User ID: " + this->userID);
}

void FirebaseServer::updateFirebase(std::function<void(void)> func){
    if(Firebase.isTokenExpired()) Firebase.refreshToken(&config);

    static int lastReadings = 0;
    static bool isSleeping = false;

    String updateVariable = firebase.getString("updateVariable", "none");
    if(updateVariable != "none"){
        if(updateVariable == "you_online"){
            firebase.setString("messages", "im_online");
            firebase.setString("updateVariable", "none");
            return;
        }

        firebase.updateVariable(updateVariable);
        func();
        lastReadings = 0;
    }else{
        if(lastReadings < SLEEP_TIME/TIME_BETWEEN_CHECKS){
            delay(TIME_BETWEEN_CHECKS);
            lastReadings++;
            if(isSleeping){
                SU.log("FirebaseServer: Woke up!");
                isSleeping = false;
            }
        }else{
            if(!isSleeping){
                SU.log("FirebaseServer: Enter sleep mode");
                isSleeping = true;
            }
            delay(TIME_BETWEEN_CHECKS_SLEEP);
        }
    }
}

bool FirebaseServer::getBool(String path, bool outputIfError, bool waitForResponse){
    bool ok = false;
    bool wrongVariable = true;
    if(!checkConnection(waitForResponse, ok)) return outputIfError;
    if(Firebase.RTDB.getBool(&fbdo, path)){
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
    if(!checkConnection(waitForResponse, ok)){
        Serial.println("Couldn't check connection on setBool()");
        return ok;
    }
    if(Firebase.RTDB.setBool(&fbdo, path, value)){
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
    String ans = getString(path, "", waitForResponse);
    return (ans=="")?outputIfError:ans.toInt();
}

bool FirebaseServer::setInt(String path, int value, bool waitForResponse){
    bool ok = false;
    if(!checkConnection(waitForResponse, ok)){
        Serial.println("Couldn't check connection on setInt()");
        return ok;
    }if(Firebase.RTDB.setInt(&fbdo, path, value)){
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
    if(Firebase.RTDB.getString(&fbdo, path)){
        ok = true;
        return fbdo.stringData();
    }else{
        String mssg = "Fail to fetch string to " +  path + " (" + fbdo.errorReason() + ")";
        Serial.println(mssg);
        //SU.log(mssg);
        ok = false;
    }
    return outputIfError;
}

bool FirebaseServer::setString(String path, String value, bool waitForResponse){
    bool ok = false;
    if(!checkConnection(waitForResponse, ok)){
        Serial.println("Couldn't check connection on setString()");
        return ok;
    }
    if(Firebase.RTDB.setString(&fbdo, path, value)){
        ok = true;
    }else{
        String mssg = "Fail to send " + String(value) + " to " +  path + " (" + fbdo.errorReason() + ")";
        Serial.println(mssg);
        //SU.log(mssg);
        ok = false;
    }
    return ok;
}

double FirebaseServer::getDouble(String path, double outputIfError, bool waitForResponse){
    bool ok = false;
    if(!checkConnection(waitForResponse, ok)) return outputIfError;
    if(Firebase.RTDB.getDouble(&fbdo, path)){
        ok = true;
        return fbdo.doubleData();
    }else{
        String mssg = "Fail to fetch string to " +  path + " (" + fbdo.errorReason() + ")";
        Serial.println(mssg);
        //SU.log(mssg);
        ok = false;
    }
    return outputIfError;
}

bool FirebaseServer::setDouble(String path, double value, bool waitForResponse){
    bool ok = false;
    if(!checkConnection(waitForResponse, ok)){
        Serial.println("Couldn't check connection on setString()");
        return ok;
    }
    if(Firebase.RTDB.setDouble(&fbdo, path, value)){
        ok = true;
    }else{
        String mssg = "Fail to send " + String(value) + " to " +  path + " (" + fbdo.errorReason() + ")";
        Serial.println(mssg);
        //SU.log(mssg);
        ok = false;
    }
    return ok;
}

bool FirebaseServer::updateVariable(String path){
    bool ok = false;
    if(!checkConnection(true, ok)) return ok;
    if(Firebase.RTDB.get(&fbdo, path)){
        if(fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_integer){
            FirebaseVariable<int> *var = FirebaseVariable<int>::find(path);
            var->updateValue(fbdo.intData());
        }else if(fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_boolean){
            FirebaseVariable<bool> *var = FirebaseVariable<bool>::find(path);
            var->updateValue(fbdo.boolData());
        }else if(fbdo.dataTypeEnum() == fb_esp_rtdb_data_type_string){
            FirebaseVariable<String> *var = FirebaseVariable<String>::find(path);
            var->updateValue(fbdo.stringData());
        }else{
            return false;
        }
    }else{
        String mssg = "Fail to fetch from " +  path + " (" + fbdo.errorReason() + ")";
        Serial.println(mssg);
        return false;
    }

    if(ok) setString("updateVariable", "none", true);

    return ok;
}

bool FirebaseServer::checkConnection(bool waitForResponse, bool &ok){
    /*uint32_t timeNow = millis();
    bool timeIntervalLongEnough = (timeNow-lastTimeConnection)>TIME_BETWEEN_CONECTIONS;
    if(waitForResponse && !timeIntervalLongEnough){
        delay(timeNow-lastTimeConnection);
        timeIntervalLongEnough = true;
    }*/
    ok = Firebase.ready()/* && timeIntervalLongEnough*/;
    //if(ans) lastTimeConnection = timeNow;
    return ok;
}