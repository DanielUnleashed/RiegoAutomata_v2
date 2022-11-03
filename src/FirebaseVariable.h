#ifndef FIREBASEVARIABLE_h
#define FIREBASEVARIABLE_h

#include <Arduino.h>
#include <vector>

template <typename Type> class FirebaseVariable {
    public: 
    FirebaseVariable(String serverDirection, Type defaultValue);
    
    String serverDirection = "";
    Type defaultValue;
    Type value;
    bool hasNewValue = false;

    // Gets the value WITHOUT updating the value. Call updateValue() first.
    Type getValue();
    void updateValue();
    void updateValue(Type val);
    void setValue(Type val);

    //static std::vector<FirebaseVariable<Type>*> variables;
    static FirebaseVariable<Type>* variables[10];
    static uint8_t variableCount;
    static FirebaseVariable<Type>* find(String name);
    static void linkAllVariables();
};

#endif