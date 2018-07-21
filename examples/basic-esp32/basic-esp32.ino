/*

This example defines three datapoints.
The first two are DPTemp type datapoints and have their own callback.
When no specific callback is attached to a datapoint, it uses the global callback.

Note the difference in return value between the callbacks:
for tempCallback uses value.getFloat() as DPTemp datapoints return a float.
globalCallback uses value.getString(char*,size_t). This method is independent of the returned type.

*/

#include <VitoWiFi.h>

VitoWiFi_setProtocol(P300);

DPTemp outsideTemp("outsideTemp", "boiler", 0x5525);
DPTemp boilerTemp("boilertemp", "boiler", 0x0810);
DPStat pumpStat("pump", "heating1", 0x2906);

HardwareSerial Serial1(1);  // use Serial1 for VitoWifi

void tempCallbackHandler(const IDatapoint& dp, DPValue value) {
  float fahrenheit = 0;
  fahrenheit = (5.0 / 9) * (value.getFloat() + 32);
  Serial.print(dp.getGroup());
  Serial.print(" - ");
  Serial.print(dp.getName());
  Serial.print(": ");
  Serial.println(fahrenheit, 1);  // print with 1 decimal
}

void globalCallbackHandler(const IDatapoint& dp, DPValue value) {
  Serial.print(dp.getGroup());
  Serial.print(" - ");
  Serial.print(dp.getName());
  Serial.print(" is ");
  char value_str[15] = {0};
  value.getString(value_str, sizeof(value_str));
  Serial.println(value_str);
}

void setup() {
  outsideTemp.setCallback(tempCallbackHandler);
  boilerTemp.setCallback(tempCallbackHandler);
  VitoWiFi.setGlobalCallback(globalCallbackHandler);  // this callback will be used for all DPs without specific callback
                                                      // must be set after adding at least 1 datapoint
  VitoWiFi.setup(&Serial1, 21, 22);  // 
  Serial.begin(115200);
  Serial.println(F("Setup finished..."));
}

void loop() {
  static unsigned long lastMillis = 0;
  if (millis() - lastMillis > 60 * 1000UL) {  // read all values every 60 seconds
    lastMillis = millis();
    VitoWiFi.readAll();
  }
  VitoWiFi.loop();
}
