#ifndef UTILS_h
#define UTILS_h

#include <Arduino.h>

class Utils{
    public:
    static String dateFormater(String);
};

inline String Utils::dateFormater(String formDate){
    int splitT = formDate.indexOf("T");
  int splitBar = formDate.indexOf("-");
  String date = formDate.substring(splitBar+1, splitT); // Month and day only
  int splitBar2 = date.indexOf("-");
  date = date.substring(splitBar2+1, date.length()) + "/" + date.substring(0, splitBar2);
  String timeStamp = formDate.substring(splitT+1, formDate.length()-1);
  return date + " " + timeStamp;
}

#endif