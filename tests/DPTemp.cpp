/* copyright 2019 Bert Melis */

#include <iostream>

#include <Datapoints/DPTemp.h>

#include "Includes/catch.hpp"
#include "Includes/CheckArray.h"

using Catch::Matchers::WithinAbs;

#define MAX_DP_LENGTH 9

TEST_CASE("Temperature Datapoint", "[DPTemp]") {
  float stdValue = 26.3f;
  uint8_t stdValueRaw[] = {0x07, 0x01};
  uint8_t cnvValueRaw[MAX_DP_LENGTH] = {0};
  static float cnvValue = 0.0;
  Datapoint* datapoint = new DPTemp("datapoint", 0x0000);
  reinterpret_cast<DPTemp*>(datapoint)->onData([](float val) {
    cnvValue = val;
  });

  REQUIRE(datapoint->getLength() == sizeof(stdValueRaw));

  SECTION("Encoding") {
    datapoint->encode(cnvValueRaw, sizeof(cnvValueRaw), reinterpret_cast<void*>(&stdValue));
    CHECK_THAT(cnvValueRaw, ByteArrayEqual(stdValueRaw, sizeof(stdValueRaw)));
  }

  SECTION("Decoding") {
    datapoint->decode(stdValueRaw, datapoint->getLength());
    CHECK_THAT(cnvValue, WithinAbs(stdValue, 0.01f));
  }
  
  delete datapoint;
}
