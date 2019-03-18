/* Copyright 2019 Bert Melis */

#include <Datapoints/Datapoint.h>

#include <assert.h>
#include <stdio.h>
#include <iostream>
using std::cout;

#define MAX_DP_LENGTH 9

// size should be checked first for equality
template <typename T>
bool equal(T* arr1, T* arr2, size_t size) {
  for (size_t i = 0; i < size; ++i) {
    if (arr1[i] != arr2[i]) return false;
  }
  return true;
}

int main() {
  {
  cout << "TEST DPTemp DATAPOINT\n";
  // test: 26.3°C is encoded to 0x0701
  float stdValue = 26.3;
  uint8_t stdValueRaw[] = {0x07, 0x01};
  static float cnvValue = 0.0;
  uint8_t cnvValueRaw[MAX_DP_LENGTH] = {0};
  DPTemp dpTemp("dpTemp", 0x0000);
  dpTemp.onData([](float val) {
    cnvValue = val;
  });
  dpTemp.encode(cnvValueRaw, sizeof(cnvValueRaw), stdValue);
  dpTemp.decode(cnvValueRaw, dpTemp.getLength());

  cout << "TEST size\n";
  assert(dpTemp.getLength() == sizeof(stdValueRaw) && "FAIL size");

  cout << "TEST encoding\n";
  assert(equal(stdValueRaw, cnvValueRaw, dpTemp.getLength()) && "FAIL encoding");

  cout << "TEST decoding\n";
  assert(stdValue == cnvValue && "FAIL decoding");
  }

  {
  cout << "TEST DPStat DATAPOINT\n";
  // test: True is encoded to 0x01
  bool stdValue = true;
  uint8_t stdValueRaw[] = {0x01};
  static bool cnvValue = false;
  uint8_t cnvValueRaw[MAX_DP_LENGTH] = {0};
  DPStat dpStat("dpStat", 0x0000);
  dpStat.onData([](bool val) {
    cnvValue = val;
  });
  dpStat.encode(cnvValueRaw, sizeof(cnvValueRaw), stdValue);
  dpStat.decode(cnvValueRaw, dpStat.getLength());

  cout << "TEST size\n";
  assert(dpStat.getLength() == sizeof(stdValueRaw) && "FAIL size");

  cout << "TEST encoding\n";
  assert(equal(stdValueRaw, cnvValueRaw, dpStat.getLength()) && "FAIL encoding");

  cout << "TEST decoding\n";
  assert(stdValue == cnvValue && "FAIL decoding");
  }

  {
  cout << "TEST DPCountS DATAPOINT\n";
  // test: 45862 is encoded to 0x26B3
  uint16_t stdValue = 45862;
  uint8_t stdValueRaw[] = {0x26, 0xB3};
  static uint16_t cnvValue = 0;
  uint8_t cnvValueRaw[MAX_DP_LENGTH] = {0};
  DPCountS dpCountS("dpCountS", 0x0000);
  dpCountS.onData([](uint16_t val) {
    cnvValue = val;
  });
  dpCountS.encode(cnvValueRaw, sizeof(cnvValueRaw), stdValue);
  dpCountS.decode(cnvValueRaw, dpCountS.getLength());

  cout << "TEST size\n";
  assert(dpCountS.getLength() == sizeof(stdValueRaw) && "FAIL size");

  cout << "TEST encoding\n";
  assert(equal(stdValueRaw, cnvValueRaw, dpCountS.getLength()) && "FAIL encoding");

  cout << "TEST decoding\n";
  assert(stdValue == cnvValue && "FAIL decoding");
  }

  // test: 16°C is encoded to 0x16
  // test: 458620 is encoded to 0x7CFF06
  // test: 458.62 is encoded to ...
  // test: 2.6 is encoded to ...
  // test: just copy bytes from in to out
  // test: print all DPs and length

  return 0;
}
