/*
  This example is not intended to explain Blynk in detail but
  acts as an example to bring VitoWiFi into your Blynk IoT project.

  V0: step V (set room temp)
  V1: labeled value (outside temp)
  V2: labeled value (set room temp)
  V3: terminal
*/

#include <Arduino.h>
#include <VitoWiFi.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

const char ssid[] = "xxxx";
const char pass[] = "xxxx";
const char auth[] = "xxxx";
VitoWiFi vitodens200(P300, &Serial);

DPTemp outsideTemp("outsidetemp" 0x552);
DPTempS roomTempSet("roomtempset" 0x2306);

BlynkTimer timer;
bool updateItems = false;
WidgetTerminal terminal(V3);


// function to run every time BlynkTimer timer fires: just set a flag and handle in main loop
void update() {
  updateItems = true;
}

// fallback when no handler has been found, just print received data to Blynk terminal
void dataCallback(const uint8_t* data, uint8_t length, Datapoint* dp) {
  terminal.print(dp->getName());
  terminal.print(" is 0x");
  for (uint8_t i = 0; i < length; ++i) terminal.printf("%02x", data[i]);
  terminal.print("\n");
}

// send receive outside temperature to virtual pin V1
void sendOutsidetemp(float value) {
  Blynk.virtualWrite(V1, value);
}

// send receive room temperature (soll) to virtual pin V2
void sendRoomtempSet(uint8_t value) {
  Blynk.virtualWrite(V2, value);
}

// callback for Blynk when widget Step V (on V0) has been changed
// write the receive value to VitoWifi and read back
BLYNK_WRITE(V0) {
  uint8_t pinValue = param.asInt();
  DPValue value(pinValue);
  terminal.printf("Blynk update: V0 = %d\n", pinValue);
  vitodens200.write(roomTempSet, value);
  vitodens200.readDatapoint(roomTempSet);
}

// log errors to terminal
void errorCallback(uint8_t error, Datapoint* dp) {
  terminal.printf("%s error %u\n", dp->getName(), error);
}

void setup() {
  // VitoWifi setup
  outsideTemp.setCallback(sendOutsidetemp);
  roomTempSet.setCallback(sendRoomtempSet);
  Datapoint::onData(dataCallback);
  vitodens200.onError(onError);
  vitodens200.begin();

  // Blynk setup
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(30000L, update);
}

void loop() {
  vitodens200.loop();
  Blynk.run();
  timer.run();

  // clear flag and read all datapoints
  if (updateItems) {
    updateItems = false;
    terminal.println("Updating items!");
    vitodens200.readAll();
  }
}