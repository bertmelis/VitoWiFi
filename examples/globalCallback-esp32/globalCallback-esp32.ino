/*

This example defines 2 datapoints of type "TEMP".
Every 60 seconds, the loop function call the updateAll-method.

For each Datapoint, the read value is returned using globalCallbackHandler

*/

#include <VitoWifi.h>

VitoWifi_setProtocol(P300);

HardwareSerial Serial1(1);

void globalCallbackHandler(const IDatapoint& dp, DPValue value) {
  Serial.print(dp.getGroup());
  Serial.print(" - ");
  Serial.print(dp.getName());
  Serial.print(": ");
  char value_str[15] = {0};
  value.getString(value_str, sizeof(value_str));
  Serial.println(value_str);
}

void setup() {
  // setup VitoWifi using a global callback handler
  VitoWifi.addDatapoint("outsidetemp", "boiler", 0x5525, TEMPL);
  VitoWifi.addDatapoint("boilertemp", "boiler", 0x0810, TEMPL);
  VitoWifi.setGlobalCallback(globalCallbackHandler);
  VitoWifi.setLogger(&Serial);
  VitoWifi.enableLogger();
  VitoWifi.setup(&Serial1, 21, 22);

  Serial.begin(115200);
  Serial.println(F("Setup finished..."));
}

void loop() {
  static unsigned long lastMillis = 0;
  if (millis() - lastMillis > 60 * 1000UL) {  // read all values every 60 seconds
    lastMillis = millis();
    Serial.println(F("reading datapoints"));
    VitoWifi.readAll();
  }

  VitoWifi.loop();
}
