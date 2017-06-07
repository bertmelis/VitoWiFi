#pragma once
#include <Arduino.h>


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
