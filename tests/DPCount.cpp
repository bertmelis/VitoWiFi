/* copyright 2019 Bert Melis */

#include <Datapoints/DPCount.h>

#include "Includes/catch.hpp"
#include "Includes/CheckArray.h"

#define MAX_DP_LENGTH 9

TEST_CASE("Count Datapoint", "[DPCount]") {
  uint32_t stdValue = 458672856;
  uint8_t stdValueRaw[] = {0xD8, 0xCA, 0x56, 0x1B};
  static uint32_t cnvValue = 0;
  uint8_t cnvValueRaw[MAX_DP_LENGTH] = {0};
  Datapoint* datapoint = new DPCount("datapoint", 0x0000);
  reinterpret_cast<DPCount*>(datapoint)->onData([](uint32_t val) {
    cnvValue = val;
  });

  REQUIRE(datapoint->getLength() == sizeof(stdValueRaw));

  SECTION("Encoding") {
    datapoint->encode(cnvValueRaw, sizeof(cnvValueRaw), reinterpret_cast<void*>(&stdValue));
    CHECK_THAT(cnvValueRaw, ByteArrayEqual(stdValueRaw, sizeof(stdValueRaw)));
  }

  SECTION("Decoding") {
    datapoint->decode(stdValueRaw, datapoint->getLength());
    CHECK(cnvValue == stdValue);
  }

  delete datapoint;
}
