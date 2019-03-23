/*

This example defines three datapoints.
The first two are TEMPL type datapoints and have their own callback.
When no specific callback is attached to a datapoint, it uses the global callback.

Note the difference in return value between the callbacks:
for tempCallback uses value.getFloat() as TEMPL datapoints return a float.
globalCallback uses value.getString(char*,size_t). This method is independent of the returned type.

*/

#include <VitoWiFi.h>

VitoWiFi vitodens200(P300, &Serial);

DPTemp outsideTemp("outsideTemp", 0x5525);
DPTemp boilerTemp("boilertemp", 0x0810);
DPStat pumpStat("pump", 0x2906);

void tempCallback(const char* name, float value) {
  float fahrenheit = 0;
  fahrenheit = (5.0 / 9) * (value + 32);
  Serial1.print(name);
  Serial1.print(": ");
  Serial1.println(fahrenheit, 1);  // print with 1 decimal
}

void dataCallback(const uint8_t* data, uint8_t length, Datapoint* dp) {
  Serial1.print(dp->getName());
  Serial1.print(" is 0x");
  for (uint8_t i = 0; i < length; ++i) Serial1.printf("%02x", data[i]);
  Serial1.print("\n");
}

void errorCallback(uint8_t error, Datapoint* dp) {
  Serial1.printf("%s error %u\n", dp->getName(), error);
}

void setup() {
  outsideTemp.onData([](float value) {
    tempCallback(outsideTemp.getName(), value);
  });
  boilerTemp.onData([](float value) {
    tempCallback(outsideTemp.getName(), value);
  });
  Datapoint::stdOnData(dataCallback);  // this callback will be used for all DPs without specific callback
  Datapoint::stdOnError(errorCallback);
  vitodens200.begin();
  Serial1.begin(115200);
  Serial1.println(F("Setup finished..."));
}

void loop() {
  static unsigned long lastMillis = 0;
  if (millis() - lastMillis > 60 * 1000UL) {  // read all values every 60 seconds
    lastMillis = millis();
    vitodens200.readAll();
  }
  vitodens200.loop();
}
