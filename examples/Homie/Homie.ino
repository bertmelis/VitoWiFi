/* copyright 2019 Bert Melis 

  see https://homieiot.github.io/ for more info
*/

#include <Arduino.h>
#include <Homie.h>
#include <Ticker.h>
#include <VitoWiFi.h>

Ticker timer;
volatile bool doUpdate = false;
bool stop = false;

VitoWiFi vitodens200(P300, &Serial);

const char* outsideTempName = "outsideTemp";
const char* boilerTempName = "boilerTemp";
const char* dhwTempName = "dhwTemp";
const char* dhwSolarTempName = "dhwSolarTemp";
const char* dhwSolarCollTempName = "dhwSolarCollTemp";
const char* solarPumpStatName = "solarPumpStat";
const char* solarPumpHoursName = "solarPumpHours";

const char* tempType = "temperature";
const char* switchType = "switch";
const char* counterType = "counter";

DPTemp outsideTemp(outsideTempName, 0x5525);
DPTemp boilerTemp(boilerTempName, 0x0810);
DPTemp dhwTemp(dhwTempName, 0x0812);
DPTemp dhwSolarTemp(dhwSolarTempName, 0x6566);
DPTemp dhwSolarCollTemp(dhwSolarCollTempName, 0x6564);
DPStat dhwSolarPumpStat(solarPumpStatName, 0x6552);
DPCountS dhwSolarPumpHours(solarPumpHoursName, 0x6568);

HomieNode outsideTempNode(outsideTempName, outsideTempName, tempType);
HomieNode boilerTempNode(boilerTempName, boilerTempName, tempType);
HomieNode dhwTempNode(dhwTempName, dhwTempName, tempType);
HomieNode dhwSolarTempNode(dhwSolarTempName, dhwSolarTempName, tempType);
HomieNode dhwSolarCollTempNode(dhwSolarCollTempName, dhwSolarCollTempName, tempType);
HomieNode dhwSolarPumpStatNode(solarPumpStatName, solarPumpStatName, switchType);
HomieNode dhwSolarPumpHoursNode(solarPumpHoursName, solarPumpHoursName, counterType);

HomieNode loggerNode("logger", "logger", "logger");

void updateAll() {
  doUpdate = true;
}

void setup() {
  outsideTemp.onData([](float value) {
    outsideTempNode.setProperty("degrees").send(String(value, 1));
  });
  vitodens200.addDatapoint(&outsideTemp);
  boilerTemp.onData([](float value) {
    boilerTempNode.setProperty("degrees").send(String(value, 1));
  });
  vitodens200.addDatapoint(&boilerTemp);
  dhwTemp.onData([](float value) {
    dhwTempNode.setProperty("degrees").send(String(value, 1));
  });
  vitodens200.addDatapoint(&dhwTemp);
  dhwSolarTemp.onData([](float value) {
    dhwSolarTempNode.setProperty("degrees").send(String(value, 1));
  });
  vitodens200.addDatapoint(&dhwSolarTemp);
  dhwSolarCollTemp.onData([](float value) {
    dhwSolarCollTempNode.setProperty("degrees").send(String(value, 1));
  });
  vitodens200.addDatapoint(&dhwSolarCollTemp);
  dhwSolarPumpStat.onData([](bool value) {
    dhwSolarPumpStatNode.setProperty("status").send(String(value ? "true" : "false"));
  });
  vitodens200.addDatapoint(&dhwSolarPumpStat);
  dhwSolarPumpHours.onData([](float value) {
    dhwSolarPumpHoursNode.setProperty("hours").send(String(value, 1));
  });
  vitodens200.addDatapoint(&dhwSolarPumpHours);
  vitodens200.onError([](uint8_t error, const Datapoint* dp) {
    // log error to loggerNode
  });
  vitodens200.begin();

  outsideTempNode.advertise("degrees")
    .setDatatype("float")
    .setUnit("°C");
  boilerTempNode.advertise("degrees")
    .setDatatype("float")
    .setUnit("°C");
  dhwTempNode.advertise("degrees")
    .setDatatype("float")
    .setUnit("°C");
  dhwSolarTempNode.advertise("degrees")
    .setDatatype("float")
    .setUnit("°C");
  dhwSolarCollTempNode.advertise("degrees")
    .setUnit("°C");
  dhwSolarCollTempNode.advertise("status")
    .setDatatype("boolean");
  dhwSolarCollTempNode.advertise("hours")
    .setDatatype("float")
    .setUnit("h");
  loggerNode.advertise("log")
    .setDatatype("string")
    .setRetained(false);
  Homie_setBrand("vhs100-heating");
  Homie_setFirmware("vhs100", "1.0.0");
  Homie.disableLogging();
  Homie.setup();

  timer.attach(60, updateAll);
}

void loop() {
  if (doUpdate) {
    doUpdate = false;
    vitodens200.readAll();
  }
  vitodens200.loop();
  Homie.loop();
  delay(1);
}
