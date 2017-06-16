#pragma once
#include <Arduino.h>
#include "Constants.h"

//datapoints type names
enum DPType {
  TEMP,
  STAT
  //MODE
  //HOURS
  //HOURSL
  //COUNT
};


//callback function typedefs, names correspond to DPType
typedef void (*GlobalCallbackFunction)(const char*, const char*, const char*);
typedef void (*TempCallbackFunction)(const char*, const char*, float);
typedef void (*StatCallbackFunction)(const char*, const char*, bool);
/*
typedef void (*ModeCallbackFunction)(const char*, const char*, uint8_t);
typedef void (*HoursCallbackFunction)(const char*, const char*, uint32_t);
typedef void (*HoursLCallbackFunction)(const char*, const char*, uint32_t);
typedef void (*CountCallbackFunction)(const char*, const char*, uint32_t);
*/

/*
const char* VWerror0 PROGMEM = "Succes";
const char* VWerror1 PROGMEM = "Connection error";
const char* VWerror2 PROGMEM = "Checksum error";
const char* VWerror3 PROGMEM = "Vitotronic returned 'DP error'";
const char* VWerror4 PROGMEM = "Message length check error";
const char* VWerror5 PROGMEM = "error5";

const char* const errorMessages[6] PROGMEM = {
  VWerror0,
  VWerror1,
  VWerror2,
  VWerror3,
  VWerror4,
  VWerror5
};
*/
