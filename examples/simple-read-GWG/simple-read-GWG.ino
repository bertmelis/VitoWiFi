#include <Arduino.h>

#include <VitoWiFi.h>

#if defined(ARDUINO_ARCH_ESP8266)
// optolink on full UART, logging output on secondary
#define SERIAL1 Serial
#define SERIAL2 Serial1
#define SERIALBAUDRATE 74880
#elif defined(ARDUINO_ARCH_ESP32)
// optolink on UART2, logging output on UART1 (connected to USB)
#define SERIAL1 Serial1
#define SERIAL2 Serial
#define SERIALBAUDRATE 115200
#endif

#ifndef SERIALBAUDRATE
  #error Target platform not supported
#endif

VitoWiFi::VitoWiFi<VitoWiFi::GWG> vitoWiFi(&SERIAL1);
ssize_t datapointIndex = -1;
constexpr size_t numberDatapoints = 1;
VitoWiFi::Datapoint datapoints[] = {
  VitoWiFi::Datapoint("temp", 0x6F, 1, VitoWiFi::div2),
};


void onResponse(const uint8_t* data, uint8_t length, const VitoWiFi::Datapoint& request) {
  // raw data can be accessed through the 'response' argument
  SERIAL2.print("Raw data received:");
  for (uint8_t i = 0; i < length; ++i) {
    SERIAL2.printf(" %02x", data[i]);
  }
  SERIAL2.print("\n");

  // the raw data can be decoded using the datapoint. Be sure to use the correct type
  SERIAL2.printf("%s: ", request.name());
  if (request.converter() == VitoWiFi::div2) {
    float value = request.decode(data, length);
    SERIAL2.printf("%.1f\n", value);
  }
}

void onError(VitoWiFi::OptolinkResult error, const VitoWiFi::Datapoint& request) {
  SERIAL2.printf("Datapoint \"%s\" error: ", request.name());
  if (error == VitoWiFi::OptolinkResult::TIMEOUT) {
    SERIAL2.print("timeout\n");
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
    datapointIndex = 0;
  }

  if (datapointIndex >= 0) {
    if (vitoWiFi.read(datapoints[datapointIndex])) {
      ++datapointIndex;
    }
    if (datapointIndex == numberDatapoints) {
      datapointIndex = -1;
    }
  }

  vitoWiFi.loop();
}
