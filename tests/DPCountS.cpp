/* copyright 2019 Bert Melis */

#include <Datapoints/DPCountS.h>

#include "Includes/catch.hpp"
#include "Includes/CheckArray.h"

#define MAX_DP_LENGTH 9

TEST_CASE("Count (short) Datapoint", "[DPCountS]") {
  const uint16_t stdValue = 4586;
  const uint8_t stdValueRaw[] = {0xEA, 0x11};
  static uint16_t cnvValue = 0;
  uint8_t cnvValueRaw[MAX_DP_LENGTH] = {0};
  Datapoint* datapoint = new DPCountS("datapoint", 0x0000);
  reinterpret_cast<DPCountS*>(datapoint)->onData([](uint16_t val) {
    cnvValue = val;
  });

  REQUIRE(datapoint->getLength() == sizeof(stdValueRaw));

  SECTION("Encoding") {
    datapoint->encode(cnvValueRaw, sizeof(cnvValueRaw), reinterpret_cast<const void*>(&stdValue));
    CHECK_THAT(cnvValueRaw, ByteArrayEqual(stdValueRaw, sizeof(stdValueRaw)));
  }

  SECTION("Decoding") {
    datapoint->decode(stdValueRaw, datapoint->getLength());
    CHECK(cnvValue == stdValue);
  }

  delete datapoint;
}
