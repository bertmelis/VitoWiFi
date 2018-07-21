/*

This example is the same is the Basic example.
The only difference is the way the datapoints are read: instead of calling VitoWifi.readAll(),
the two datapoints are updated seperately via VitoWifi.readDatapoint(const char*).
Another method would be to call VitoWifi.readGroup(const char*) where you could fill "boiler"
as group.

*/

#include <VitoWiFi.h>

VitoWiFi_setProtocol(P300);

DPTemp outsideTemp("outsidetemp", "boiler", 0x5525);
DPTemp boilerTemp("boilertemp", "boiler", 0x0810);

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
  VitoWiFi.setGlobalCallback(globalCallbackHandler);
  VitoWiFi.setup(&Serial);

  Serial1.begin(115200);
  Serial1.println(F("Setup finished..."));
}

void loop() {
  static unsigned long lastMillis = 0;
  if (millis() - lastMillis > 60 * 1000UL) {
    lastMillis = millis();
    VitoWiFi.readDatapoint(outsideTemp);
    VitoWiFi.readDatapoint(boilerTemp);
    // Calling VitoWifi.readGroup("boiler"); would have the same result in this case are there are only 2 datapoints.
    // Calling VitoWifi.readAll(); obviously also has the same result in this example.
  }
  VitoWiFi.loop();
}
