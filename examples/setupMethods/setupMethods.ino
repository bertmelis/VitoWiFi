/*

This example defines 2 datapoints of type "TEMP".
Every 60 seconds, the loop function call the updateAll-method.

For each Datapoint, the read value is returned using globalCallbackHandler

*/

#include <VitoWifi.h>

#ifdef ARDUINO_ARCH_ESP32
  HardwareSerial Serial1(2);
#endif


void globalCallbackHandler(const char* name, const char* group, const char* value) {
  Serial1.print(group);
  Serial1.print(" - ");
  Serial1.print(name);
  Serial1.print(": ");
  Serial1.println(value);
}


void setup() {
  Serial1.begin(115200);

  //setup VitoWifi using a global callback handler
  VitoWifi.enableLogger();
  VitoWifi.setLoggingPrinter(&Serial1);

  VitoWifi.addDatapoint("outsidetemp", "boiler", 0x5525, TEMP);
  VitoWifi.addDatapoint("boilertemp",  "boiler", 0x0810, TEMP);
  VitoWifi.setGlobalCallback(globalCallbackHandler);
  VitoWifi.setup(&Serial);

  Serial1.begin(115200);
  Serial1.println(F("Setup finished..."));
}

void loop() {

  static unsigned long lastMillis = 0;
  if (millis() - lastMillis > 60 * 1000UL) {  //read all values every 30 seconds
    lastMillis = millis();
    VitoWifi.readAll();
  }

  VitoWifi.loop();
}
