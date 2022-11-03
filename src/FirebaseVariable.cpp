#include "FirebaseVariable.h"

#include "FirebaseServer.h"

//template <typename Type> std::vector<FirebaseVariable<Type>*> FirebaseVariable<Type>::variables;
template <typename Type> FirebaseVariable<Type>* FirebaseVariable<Type>::variables[10];
template <typename Type> uint8_t FirebaseVariable<Type>::variableCount = 0;

template <typename Type> FirebaseVariable<Type>::FirebaseVariable(String path, Type defaultValue){
    this->serverDirection = path;
    this->defaultValue = defaultValue;
    this->value = defaultValue;
    variables[variableCount++] = this;
}

template <typename Type> Type FirebaseVariable<Type>::getValue(){
    this->hasNewValue = false;
    return value;
}

template <>
void FirebaseVariable<int>::setValue(int v){
    if(v == this->value) return;
    this->updateValue(v);
    firebase.setInt(serverDirection, v);
}

template <>
void FirebaseVariable<bool>::setValue(bool v){
    if(v == this->value) return;
    this->updateValue(v);
    firebase.setBool(serverDirection, v);
}

template <>
void FirebaseVariable<String>::setValue(String v){
    if(v == this->value) return;
    this->updateValue(v);
    firebase.setString(serverDirection, v);
}

template <>
void FirebaseVariable<double>::setValue(double v){
    if(v == this->value) return;
    this->updateValue(v);
    firebase.setDouble(serverDirection, v);
}

template <>
void FirebaseVariable<int>::updateValue(){
    this->value = firebase.getInt(serverDirection, defaultValue);
    this->hasNewValue = true;
}

template <>
void FirebaseVariable<bool>::updateValue(){
    this->value = firebase.getBool(serverDirection, defaultValue);
}

template <>
void FirebaseVariable<String>::updateValue(){
    this->value = firebase.getString(serverDirection, defaultValue);
}

template <>
void FirebaseVariable<double>::updateValue(){
    this->value = firebase.getDouble(serverDirection, defaultValue);
}

template <typename Type> void FirebaseVariable<Type>::setValue(Type v){
    this->value = v;
    Serial.println("Value type not supported in find() method inside FirebaseServer.h");
    delay(10);
    abort();
}

template <typename Type> void FirebaseVariable<Type>::updateValue(Type v){
    if(v == this->value) return;
    this->value = v;
}

template <typename Type> FirebaseVariable<Type>* FirebaseVariable<Type>::find(String name){
    for(FirebaseVariable<Type>* el : variables){
        if(el == NULL) break;
        if(el->serverDirection == name) return el;
    }
    Serial.printf("Couldn't find %s\n", name.c_str());
    delay(10);
    abort();
}

template <typename Type> void FirebaseVariable<Type>::linkAllVariables(){
    for(FirebaseVariable<Type>* el : variables){
        if(el == NULL) break;
        el->updateValue();
        Serial.printf("%s = %s\n", el->serverDirection.c_str(), String(el->value).c_str());
    }
}

template class FirebaseVariable<bool>;
template class FirebaseVariable<int>;
template class FirebaseVariable<String>;
template class FirebaseVariable<double>;