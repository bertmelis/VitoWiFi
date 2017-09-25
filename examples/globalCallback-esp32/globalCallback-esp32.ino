/*

This example defines 2 datapoints of type "TEMP".
Every 60 seconds, the loop function call the updateAll-method.

For each Datapoint, the read value is returned using globalCallbackHandler

*/

#include <VitoWifi.h>

HardwareSerial Serial1(1);


void globalCallbackHandler(const char* name, const char* group, const char* value) {
  Serial.print(group);
  Serial.print(" - ");
  Serial.print(name);
  Serial.print(": ");
  Serial.println(value);
}


void setup() {
  //setup VitoWifi using a global callback handler
  VitoWifi.addDatapoint("outsidetemp", "boiler", 0x5525, TEMP);
  VitoWifi.addDatapoint("boilertemp",  "boiler", 0x0810, TEMP);
  VitoWifi.setGlobalCallback(globalCallbackHandler);
  VitoWifi.setLoggingPrinter(&Serial);
  VitoWifi.enableLogger();
  VitoWifi.setup(&Serial1, 21, 22);

  Serial.begin(115200);
  Serial.println(F("Setup finished..."));
}

void loop() {

  static unsigned long lastMillis = 0;
  if (millis() - lastMillis > 60 * 1000UL) {  //read all values every 60 seconds
    lastMillis = millis();
	Serial.println(F("reading datapoints"));
    VitoWifi.readAll();
  }

  VitoWifi.loop();
}
