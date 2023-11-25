/*

This example defines three datapoints.
The first two are TEMPL type datapoints and have their own callback.
When no specific callback is attached to a datapoint, it uses the global callback.

Note the difference in return value between the callbacks:
for tempCallback uses value.getFloat() as TEMPL datapoints return a float.
globalCallback uses value.getString(char*,size_t). This method is independent of the returned type.

*/

#include <Arduino.h>

#include <VitoWiFi.h>

#ifdef ESP8266
// optolink on full UART, logging output on secondary
#define SERIAL1 Serial
#define SERIAL2 Serial1
#define SERIALBAUDRATE 74880
#endif

#ifdef ESP32
// optolink on UART2, logging output on UART1 (connected to USB)
#define SERIAL1 Serial1
#define SERIAL2 Serial
#define SERIALBAUDRATE 115200
#endif

#ifndef SERIALBAUDRATE
  #error Target platform not supported
#endif

VitoWiFi::VitoWiFi<VitoWiFi::VS2> vitoWiFi(&SERIAL1);
bool readValues = false;
uint8_t datapointIndex = 0;

VitoWiFi::Datapoint datapoints[] = {
  VitoWiFi::Datapoint("outsidetemp", 0x5525, 2, VitoWiFi::div10),
  VitoWiFi::Datapoint("boilertemp", 0x0810, 2, VitoWiFi::div10),
  VitoWiFi::Datapoint("pump", 0x2906, 1, VitoWiFi::noconv)
};

void onResponse(const VitoWiFi::PacketVS2& response, const VitoWiFi::Datapoint& request) {
  // raw data can be accessed through the 'response' argument
  SERIAL2.print("Raw data received:");
  const uint8_t* data = response.data();
  for (uint8_t i = 0; i < response.dataLength(); ++i) {
    SERIAL2.printf(" %02x", data[i]);
  }
  SERIAL2.print("\n");

  // the raw data can be decoded using the datapoint. Be sure to use the correct type
  SERIAL2.printf("%s: ", request.name());
  if (request.converter() == VitoWiFi::div10) {
    float value = request.decode(response);
    SERIAL2.printf("%.1f\n", value);
  } else if (request.converter() == VitoWiFi::noconv) {
    bool value = request.decode(response);
    // alternatively, we can just cast response.data()[0] to bool
    SERIAL2.printf("%s\n", value ? "ON" : "OFF");
  }
}

void onError(VitoWiFi::OptolinkResult error, const VitoWiFi::Datapoint& request) {
  SERIAL2.printf("Datapoint \"%s\" error: ", request.name());
  if (error == VitoWiFi::OptolinkResult::TIMEOUT) {
    SERIAL2.print("timeout\n");
  } else if (error == VitoWiFi::OptolinkResult::LENGTH) {
    SERIAL2.print("length\n");
  } else if (error == VitoWiFi::OptolinkResult::NACK) {
    SERIAL2.print("nack\n");
  } else if (error == VitoWiFi::OptolinkResult::CRC) {
    SERIAL2.print("crc\n");
  } else if (error == VitoWiFi::OptolinkResult::ERROR) {
    SERIAL2.print("error\n");
  }
}

void setup() {
  delay(1000);
  SERIAL2.begin(SERIALBAUDRATE);
  SERIAL2.print("Setting up vitoWiFi\n");

  vitoWiFi.onResponse(onResponse);
  vitoWiFi.onError(onError);
  vitoWiFi.begin();

  SERIAL2.print("Setup finished\n");
}

void loop() {
  static uint32_t lastMillis = 0;
  if (millis() - lastMillis > 60000UL) {  // read all values every 60 seconds
    lastMillis = millis();
    readValues = true;
    datapointIndex = 0;
  }

  if (readValues) {
    if (vitoWiFi.read(datapoints[datapointIndex])) {
      ++datapointIndex;
    }
    if (datapointIndex == 3) {
      readValues = false;
    }
  }

  vitoWiFi.loop();
}