#include <signal.h>
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>  // is already included by VitoWiFi
#include <VitoWiFi.h>

VitoWiFi::VitoWiFi<VitoWiFi::VS2> vitoWiFi("/dev/ttyUSB0");

bool exitProgram = false;
bool readValues = false;
uint8_t datapointIndex = 0;

VitoWiFi::Datapoint datapoints[] = {
  VitoWiFi::Datapoint("outsidetemp", 0x5525, 2, VitoWiFi::div10),
  VitoWiFi::Datapoint("boilertemp", 0x0810, 2, VitoWiFi::div10),
  VitoWiFi::Datapoint("pump", 0x2906, 1, VitoWiFi::noconv)
};

void signalHandler(int signum) {
   std::cout << "Caught signal " << signum << std::endl;
   exitProgram = true;
}

void onResponse(const VitoWiFi::PacketVS2& response, const VitoWiFi::Datapoint& request) {
  // raw data can be accessed through the 'response' argument
  std::cout << "Raw data received: " << std::endl;
  const uint8_t* data = response.data();
  for (uint8_t i = 0; i < response.dataLength(); ++i) {
    std::cout << std::hex << (int)data[i] << " ";
  }
  std::cout << std::endl;

  // the raw data can be decoded using the datapoint. Be sure to use the correct type
  std::cout << request.name() <<": ";
  if (request.converter() == VitoWiFi::div10) {
    float value = request.decode(response);
    std::cout << std::setprecision(2) << value << std::endl;
  } else if (request.converter() == VitoWiFi::noconv) {
    bool value = request.decode(response);
    if (value) {
      std::cout << "ON" << std::endl;
    } else {
      std::cout << "OFF" << std::endl;
    }
    // alternatively, we can just cast response.data()[0] to bool
  }
}

void onError(VitoWiFi::OptolinkResult error, const VitoWiFi::Datapoint& request) {
  printf("Datapoint \"%s\" error: ", request.name());
  if (error == VitoWiFi::OptolinkResult::TIMEOUT) {
    std::cout << "timeout" << std::endl;
  } else if (error == VitoWiFi::OptolinkResult::LENGTH) {
    std::cout << "length" << std::endl;
  } else if (error == VitoWiFi::OptolinkResult::NACK) {
    std::cout << "nack" << std::endl;
  } else if (error == VitoWiFi::OptolinkResult::CRC) {
    std::cout << "crc" << std::endl;
  } else if (error == VitoWiFi::OptolinkResult::ERROR) {
    std::cout << "error" << std::endl;
  }
}

void setup() {
  sleep(2);
  std::cout << "Setting up VitoWiFi" << std::endl;

  vitoWiFi.onResponse(onResponse);
  vitoWiFi.onError(onError);
  vitoWiFi.begin();

  std::cout << "Setup finished" << std::endl;
}

void loop() {
  static uint32_t lastMillis = 0;
  if (vw_millis() - lastMillis > 60000UL) {  // read all values every 60 seconds
    std::cout << "reading datapoints" << std::endl;
    lastMillis = vw_millis();
    readValues = true;
    datapointIndex = 0;
  }

  if (readValues) {
    if (vitoWiFi.read(datapoints[datapointIndex])) {
      std::cout << "datapoint \"" << datapoints[datapointIndex].name() << "\" requested" << std::endl;
      ++datapointIndex;
    }
    if (datapointIndex == 3) {
      readValues = false;
    }
  }

  vitoWiFi.loop();
}


int main() {
  setup();
  while(1) {
    loop();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (exitProgram) break;
  }
  vitoWiFi.end();
  return EXIT_SUCCESS;
}