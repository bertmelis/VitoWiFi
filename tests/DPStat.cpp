/* copyright 2019 Bert Melis */

#include <Datapoints/DPStat.h>

#include "Includes/catch.hpp"
#include "Includes/CheckArray.h"

#define MAX_DP_LENGTH 9

TEST_CASE("Status Datapoint", "[DPStat]") {
  const bool stdValue = true;
  const uint8_t stdValueRaw[] = {0x01};
  static bool cnvValue = false;
  uint8_t cnvValueRaw[MAX_DP_LENGTH] = {0};
  Datapoint* datapoint = new DPStat("datapoint", 0x0000);
  reinterpret_cast<DPStat*>(datapoint)->onData([](bool val) {
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
