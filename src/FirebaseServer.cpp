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

    FirebaseVariable<int>::linkAllVariables();
    FirebaseVariable<bool>::linkAllVariables();
    FirebaseVariable<String>::linkAllVariables();

    SU.log("Firebase ready!");
}

void FirebaseServer::updateFirebase(std::function<void(void)> func){
    static int lastReadings = 0;
    static bool isSleeping = false;

    String variableChanged = firebase.getString("updateVariable", "none");
    if(variableChanged != "none"){
        firebase.updateVariable(variableChanged);
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
    bool ans = Firebase.ready() && signUp/* && timeIntervalLongEnough*/;
    ok = ans;
    //if(ans) lastTimeConnection = timeNow;
    return ans;
}