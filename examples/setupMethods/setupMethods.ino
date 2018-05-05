/*

This example defines 2 datapoints of type "TEMP".
Every 60 seconds, the loop function call the updateAll-method.

For each Datapoint, the read value is returned using globalCallbackHandler

*/

#include <VitoWifi.h>

VitoWifi_setProtocol(P300);

void globalCallbackHandler(const IDatapoint& dp, DPValue value) {
  Serial1.print(dp.getGroup());
  Serial1.print(" - ");
  Serial1.print(dp.getName());
  Serial1.print(": ");
  char value_str[15] = {0};
  value.getString(value_str, sizeof(value_str));
  Serial1.println(value_str);
}

void setup() {
  Serial1.begin(115200);

  // setup VitoWifi using a global callback handler
  VitoWifi.enableLogger();
  VitoWifi.setLogger(&Serial1);

  VitoWifi.addDatapoint("outsidetemp", "boiler", 0x5525, TEMPL);
  VitoWifi.addDatapoint("boilertemp", "boiler", 0x0810, TEMPL);
  VitoWifi.setGlobalCallback(globalCallbackHandler);
  VitoWifi.setup(&Serial);

  Serial1.begin(115200);
  Serial1.println(F("Setup finished..."));
}

void loop() {
  static unsigned long lastMillis = 0;
  if (millis() - lastMillis > 60 * 1000UL) {  // read all values every 60 seconds
    lastMillis = millis();
    VitoWifi.readAll();
  }

  VitoWifi.loop();
}
