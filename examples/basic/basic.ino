/*

This example defines three datapoints.
The first two are TEMPL type datapoints and have their own callback.
When no specific callback is attached to a datapoint, it uses the global callback.

Note the difference in return value between the callbacks:
for tempCallback uses value.getFloat() as TEMPL datapoints return a float.
globalCallback uses value.getString(char*,size_t). This method is independent of the returned type.

*/

#include <VitoWifi.h>

VitoWifi_setProtocol(P300);

void tempCallbackHandler(const IDatapoint& dp, DPValue value) {
  float fahrenheit = 0;
  fahrenheit = (5.0 / 9) * (value.getFloat() + 32);
  Serial1.print(dp.getGroup());
  Serial1.print(" - ");
  Serial1.print(dp.getName());
  Serial1.print(": ");
  Serial1.println(fahrenheit, 1);  // print with 1 decimal
}

void globalCallbackHandler(const IDatapoint& dp, DPValue value) {
  Serial1.print(dp.getGroup());
  Serial1.print(" - ");
  Serial1.print(dp.getName());
  Serial1.print(" is ");
  char value_str[15] = {0};
  value.getString(value_str, sizeof(value_str));
  Serial1.println(value_str);
}

void setup() {
  VitoWifi.addDatapoint("outsidetemp", "boiler", 0x5525, TEMPL).setCallback(tempCallbackHandler);
  VitoWifi.addDatapoint("boilertemp", "boiler", 0x0810, TEMPL).setCallback(tempCallbackHandler);
  VitoWifi.addDatapoint("pump", "heating1", 0x2906, STAT);
  VitoWifi.setGlobalCallback(globalCallbackHandler);  // this callback will be used for all DPs without specific callback
                                                      // must be set after adding at least 1 datapoint
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
