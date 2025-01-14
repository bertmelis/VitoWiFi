#include <Arduino.h>

#include <VitoWiFi.h>

/*
This example is to show how you could build your own
interface and serves as a compilation test
*/

// Dummy class that has all the methods for VitoWiFi to work
// but doesn't do anything in this case.
// Use as skeleton for your own implementation
class DummyInterface {
 public:
  bool begin() {
    // prepare the interface
    // optolink comm at 4800 baud, 8 bits, even parity and 2 stop bits
    // called at VitoWiFi::begin()
    return true;
  }
  void end() {
    // stop the interface
    // called at VitoWiFi::end()
  }
  std::size_t write(const uint8_t* data, uint8_t length) {
    // tries to write `data` with length `length` to the interface
    // returns the actually written data
    return 0;
  }
  uint8_t read() {
    // read one byte from the interface
    // availability of data is checked first
    return 0;
  }
  size_t available() {
    // check if data is available
    return 0;
  }
};

// optolink on Dummy Interface, logging output on UART1 (connected to USB)
DummyInterface dummyInterface;
#define SERIAL1 dummyInterface
#define SERIAL2 Serial
#define SERIALBAUDRATE 115200

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
