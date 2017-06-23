/*

This example defines 2 datapoints of type "TEMP".
Every 60 seconds, the loop function call the updateAll-method.

For each Datapoint, the read value is returned using globalCallbackHandler

*/

#include <VitoWifi.h>

#ifdef ARDUINO_ARCH_ESP32
  HardwareSerial Serial1(2);
#endif


void floatCallbackHandler(const char* name, const char* group, float value) {
  float fahrenheit = 0;
  fahrenheit = (5.0/9) * (value + 32);
  Serial1.print(group);
  Serial1.print(" - ");
  Serial1.print(name);
  Serial1.print(": ");
  Serial1.println(fahrenheit, 1);  //print with 1 decimal
}


void statCallbackHandler(const char* name, const char* group, bool value) {
  Serial1.print(group);
  Serial1.print(" - ");
  Serial1.print(name);
  Serial1.print(" is ");
  const char* status = (value) ? "on" : "off";
  Serial1.println(status);
}


void setup() {
  //setup VitoWifi using a global callback handler
  //you can also specify a global callback handler
  //for datapoints without specific callback, the global will be used
  VitoWifi.addDatapoint("outsidetemp", "boiler",   0x5525, TEMP).setCallback(floatCallbackHandler);
  VitoWifi.addDatapoint("boilertemp",  "boiler",   0x0810, TEMP).setCallback(floatCallbackHandler);
  VitoWifi.addDatapoint("pump",        "heating1", 0x2906, STAT).setCallback(statCallbackHandler);
  VitoWifi.setup(&Serial);

  Serial1.begin(115200);
  Serial1.println(F("Setup finished..."));
}

void loop(){

  static unsigned long lastMillis = 0;
  if (millis() - lastMillis > 60 * 1000UL) {  //read all values every 30 seconds
    lastMillis = millis();
    VitoWifi.readAll();
  }

  VitoWifi.loop();
}
