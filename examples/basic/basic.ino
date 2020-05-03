#include <VitoWiFi.h>  // don't forget to include this lib

// first define your heater
// first argument is the protocol (P300 or KW), second is the serial interface on which you connect the optolink
VitoWiFi vitodens200(P300, &Serial);

// next, define some datapoints, see the "datapoints" section for more info
DPTemp outsideTemp("outsideTemp", 0x5525);
DPTemp boilerTemp("boilertemp", 0x0810);
DPStat pumpStat("pump", 0x2906);

// create a callback function to do something useful with the data
// this one is dedicated for temperature values
void tempCallback(const char* name, float value) {
  float fahrenheit = 0;
  fahrenheit = (5.0 / 9) * (value + 32);
  Serial1.print(name);
  Serial1.print(": ");
  Serial1.println(fahrenheit, 1);  // print with 1 decimal
}

// create a general callback for datapoints that don't get a dedicated one
void dataCallback(const uint8_t* data, uint8_t length, Datapoint* dp) {
  Serial1.print(dp->getName());
  Serial1.print(" is 0x");
  for (uint8_t i = 0; i < length; ++i) Serial1.printf("%02x", data[i]);
  Serial1.print("\n");
}

// create a callback to read error messages
void errorCallback(uint8_t error, Datapoint* dp) {
  Serial1.printf("%s error %u\n", dp->getName(), error);
}

void setup() {
  // attach the "temp" callback to the two temperature datapoints
  outsideTemp.onData([](float value) {
    tempCallback(outsideTemp.getName(), value);
  });
  boilerTemp.onData([](float value) {
    tempCallback(outsideTemp.getName(), value);
  });
  // when attaching callback not on specific datapoints, use your VitoWiFi
  // object instead:
  vitodens200.onData(dataCallback);
  // attach the errorCallback
  vitodens200.onError(errorCallback);
  
  delay(5);

  // setup is done, start VitoWiFi
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