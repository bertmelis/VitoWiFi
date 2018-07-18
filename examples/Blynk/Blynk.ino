/*
  This example is not intended to explain Blynk in detail but
  actes as an example to bring VitoWifi into your Blynk IoT project.

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
VitoWiFi_setProtocol(KW);

DPTemp outsideTemp("outsidetemp", "boiler", 0x552);
DPTempS roomTempSet("roomtempset", "heating", 0x2306);

BlynkTimer timer;
bool updateItems = false;
WidgetTerminal terminal(V3);


// function to run every time BlynkTimer timer fires: just set a flag and handle in main loop
void update() {
  updateItems = true;
}

// fallback when no handler has been found, just print received data to Blynk terminal
void globalCallbackHandler(const IDatapoint& dp, DPValue value) {
  char value_str[15] = {0};
  value.getString(value_str, sizeof(value_str));
  terminal.printf("Received: %s - %s: %s\n", dp.getGroup(), dp.getName(), value_str);
}

// send receive outside temperature to virtual pin V1
void sendOutsidetemp(const IDatapoint& dp, DPValue value) {
  Blynk.virtualWrite(V1, value.getFloat());
}

// send receive room temperature (soll) to virtual pin V2
void sendRoomtempSet(const IDatapoint& dp, DPValue value) {
  Blynk.virtualWrite(V2, value.getU8());
}

// callback for Blynk when widget Step V (on V0) has been changed
// write the receive value to VitoWifi and read back
BLYNK_WRITE(V0) {
  uint8_t pinValue = param.asInt();
  DPValue value(pinValue);
  terminal.printf("Blynk update: V0 = %d\n", pinValue);
  VitoWiFi.writeDatapoint(roomTempSet, value);
  VitoWiFi.readDatapoint(roomTempSet);
}

void setup() {
  // VitoWifi setup
  VitoWiFi.setLogger(&terminal);  // might be too verbose/fast for Blynk to handle
  VitoWiFi.enableLogger();  // might be too verbose/fast for Blynk to handle
  outsideTemp.setCallback(sendOutsidetemp);
  roomTempSet.setWriteable(true).setCallback(sendRoomtempSet);
  VitoWiFi.setGlobalCallback(globalCallbackHandler);
  VitoWiFi.setup(&Serial);

  // Blynk setup
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(30000L, update);
}

void loop() {
  VitoWiFi.loop();
  Blynk.run();
  timer.run();

  // clear flag and read all datapoints
  if (updateItems) {
    updateItems = false;
    terminal.println("Updating items!");
    VitoWiFi.readAll();
  }
}