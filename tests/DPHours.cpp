/* copyright 2019 Bert Melis */

#include <Datapoints/DPHours.h>

#include "Includes/catch.hpp"
#include "Includes/CheckArray.h"

using Catch::Matchers::WithinAbs;

#define MAX_DP_LENGTH 9

TEST_CASE("Hours Datapoint", "[DPHours]") {
  const float stdValue = 452663.72;
  const uint8_t stdValueRaw[] = {0x80, 0x8F, 0x21, 0x61};
  static float cnvValue = 0.0;
  uint8_t cnvValueRaw[MAX_DP_LENGTH] = {0};
  Datapoint* datapoint = new DPHours("datapoint", 0x0000);
  reinterpret_cast<DPHours*>(datapoint)->onData([](float val) {
    cnvValue = val;
  });

  REQUIRE(datapoint->getLength() == sizeof(stdValueRaw));

  SECTION("Encoding") {
    datapoint->encode(cnvValueRaw, sizeof(cnvValueRaw), reinterpret_cast<const void*>(&stdValue));
    CHECK_THAT(cnvValueRaw, ByteArrayEqual(stdValueRaw, sizeof(stdValueRaw)));
  }

  SECTION("Decoding") {
    datapoint->decode(stdValueRaw, datapoint->getLength());
    CHECK_THAT(cnvValue, Catch::Matchers::WithinAbs(stdValue, 0.01f));
  }
}
