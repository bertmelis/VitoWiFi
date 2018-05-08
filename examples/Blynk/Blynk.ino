/*
  This example is not intended to explain Blynk in detail but
  actes as an example to bring VitoWifi into your Blynk IoT project.

  V0: step V (set room temp)
  V1: labeled value (outside temp)
  V2: labeled value (set room temp)
  V3: terminal
*/

#include <Arduino.h>
#include <VitoWifi.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

const char ssid[] = "xxxx";
const char pass[] = "xxxx";
const char auth[] = "xxxx";
VitoWifi_setProtocol(KW);
bool updateItems = false;

BlynkTimer timer;
WidgetTerminal terminal(V3);


// function to run every time BlynkTimer timer fires: just set a flag and handle in main loop
void update() {
  updateItems = true;
}

// fallback when no handler has been found, just print received data to Blynk terminal
void globalCallbackHandler(const char* name, const char* group, const char* value) {
  terminal.printf("Received: %s - %s: %s\n", group, name, value);
}

// send receive outside temperature to virtual pin V1
void sendOutsidetemp(const char* name, const char* group, float value) {
  Blynk.virtualWrite(V1, value);
}

// send receive room temperature (soll) to virtual pin V2
void sendRoomtempSet(const char* name, const char* group, byte value) {
  Blynk.virtualWrite(V2, value);
}

// callback for Blynk when widget Step V (on V0) has been changed
// write the receive value to VitoWifi and read back
BLYNK_WRITE(V0) {
  int pinValue = param.asInt();
  terminal.printf("Blynk update: V0 = %d\n", pinValue);
  VitoWifi.writeDatapoint("roomtempset", pinValue);
  VitoWifi.readDatapoint("roomtempset");
}

void setup() {
  // VitoWifi setup
  VitoWifi.setLogger(&terminal);  // might be too verbose/fast for Blynk to handle
  VitoWifi.enableLogger();  // might be too verbose/fast for Blynk to handle
  VitoWifi.addDatapoint("outsidetemp", "boiler", 0x5525, TEMP).setCallback(sendOutsidetemp);
  VitoWifi.addDatapoint("roomtempset", "heating", 0x2306, TEMPS).setWriteable().setCallback(sendRoomtempSet);
  VitoWifi.setGlobalCallback(globalCallbackHandler);
  VitoWifi.setup(&Serial);

  // Blynk setup
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(30000L, update);
}

void loop() {
  VitoWifi.loop();
  Blynk.run();
  timer.run();

  // clear flag and read all datapoints
  if (updateItems) {
    updateItems = false;
    terminal.println("Updating items!");
    VitoWifi.readAll();
  }
}