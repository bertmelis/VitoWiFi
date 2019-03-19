/* copyright 2019 Bert Melis */

#include <Datapoints/DPMode.h>

#include "Includes/catch.hpp"
#include "Includes/CheckArray.h"

#define MAX_DP_LENGTH 9

TEST_CASE("Mode (enum) / TempS Datapoint", "[DPMode]") {
  const uint8_t stdValue = 3;
  const uint8_t stdValueRaw[] = {0x03};
  static uint8_t cnvValue = 0;
  uint8_t cnvValueRaw[MAX_DP_LENGTH] = {0};
  Datapoint* datapoint = new DPMode("datapoint", 0x0000);
  reinterpret_cast<DPMode*>(datapoint)->onData([](uint8_t val) {
    cnvValue = val;
  });

  REQUIRE(datapoint->getLength() == sizeof(stdValueRaw));

  SECTION("Encoding") {
    datapoint->encode(cnvValueRaw, sizeof(cnvValueRaw), reinterpret_cast<const void*>(&stdValue));
    CHECK_THAT(cnvValueRaw, ByteArrayEqual(stdValueRaw, sizeof(stdValueRaw)));
  }

  SECTION("Deconding") {
    datapoint->decode(stdValueRaw, datapoint->getLength());
    CHECK(cnvValue == stdValue);
  }

  delete datapoint;
}
