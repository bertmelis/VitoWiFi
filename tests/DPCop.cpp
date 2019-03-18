/* copyright 2019 Bert Melis */

#include <Datapoints/DPCop.h>

#include "Includes/catch.hpp"
#include "Includes/CheckArray.h"

using Catch::Matchers::WithinAbs;

#define MAX_DP_LENGTH 9

TEST_CASE("COP Datapoint", "[DPCop]") {
  const float stdValue = 2.4;
  const uint8_t stdValueRaw[] = {0x18};
  static float cnvValue = 0.0;
  uint8_t cnvValueRaw[MAX_DP_LENGTH] = {0};
  Datapoint* datapoint = new DPCop("datapoint", 0x0000);
  reinterpret_cast<DPCop*>(datapoint)->onData([](float val) {
    cnvValue = val;
  });

  REQUIRE(datapoint->getLength() == sizeof(stdValueRaw));

  SECTION("Encoding") {
    datapoint->encode(cnvValueRaw, sizeof(cnvValueRaw), reinterpret_cast<const void*>(&stdValue));
    CHECK_THAT(cnvValueRaw, ByteArrayEqual(stdValueRaw, sizeof(stdValueRaw)));
  }

  SECTION("Decoding") {
    datapoint->decode(stdValueRaw, datapoint->getLength());
    CHECK_THAT(cnvValue, WithinAbs(stdValue, 0.01f));
  }
}
