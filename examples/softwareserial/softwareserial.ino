#include <Arduino.h>

#include <VitoWiFi.h>
#include <SoftwareSerial.h>
const int sRX = 4; // software RX on D2(GPIO 4)
const int sTX = 5; // software TX on D1(GPIO 5)

EspSoftwareSerial::UART swSer(sRX, sTX);

// optolink on softwareserial, logging output on hardware UART

VitoWiFi::VitoWiFi<VitoWiFi::VS1> vitoWiFi(&swSer);
bool readValues = false;
uint8_t datapointIndex = 0;

VitoWiFi::Datapoint datapoints[] = {
  VitoWiFi::Datapoint("outsidetemp", 0x5525, 2, VitoWiFi::div10),
  VitoWiFi::Datapoint("boilertemp", 0x0810, 2, VitoWiFi::div10),
  VitoWiFi::Datapoint("pump", 0x2906, 1, VitoWiFi::noconv)
};

void onResponse(const uint8_t* data, uint8_t length, const VitoWiFi::Datapoint& request) {
  // raw data can be accessed through the 'response' argument
  Serial.print("Raw data received:");
  for (uint8_t i = 0; i < length; ++i) {
    Serial.printf(" %02x", data[i]);
  }
  Serial.print("\n");

  // the raw data can be decoded using the datapoint. Be sure to use the correct type
  Serial.printf("%s: ", request.name());
  if (request.converter() == VitoWiFi::div10) {
    float value = request.decode(data, length);
    Serial.printf("%.1f\n", value);
  } else if (request.converter() == VitoWiFi::noconv) {
    // in this example, the response is one byte
    Serial.printf("%s\n", (data[0] > 0) ? "ON" : "OFF");
  }
}

void onError(VitoWiFi::OptolinkResult error, const VitoWiFi::Datapoint& request) {
  Serial.printf("Datapoint \"%s\" error: ", request.name());
  if (error == VitoWiFi::OptolinkResult::TIMEOUT) {
    Serial.print("timeout\n");
  } else if (error == VitoWiFi::OptolinkResult::LENGTH) {
    Serial.print("length\n");
  } else if (error == VitoWiFi::OptolinkResult::NACK) {
    Serial.print("nack\n");
  } else if (error == VitoWiFi::OptolinkResult::CRC) {
    Serial.print("crc\n");
  } else if (error == VitoWiFi::OptolinkResult::ERROR) {
    Serial.print("error\n");
  }
}

void setup() {
  delay(1000);
  Serial.begin(74880);
  Serial.print("Setting up vitoWiFi\n");

  vitoWiFi.onResponse(onResponse);
  vitoWiFi.onError(onError);
  vitoWiFi.begin();

  Serial.print("Setup finished\n");
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