/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "Converter.h"

namespace VitoWiFi {

VariantValue Div10Convert::decode(const uint8_t* data, uint8_t len) const {
  assert(len == 1 || len == 2);
  float retVal = 0;
  if (len == 1) {
    int8_t val = data[0];
    retVal = val / 10.f;
  }
  if (len == 2) {
    int16_t val = data[1] << 8 | data[0];
    retVal = val / 10.f;
  }
  return VariantValue(retVal);
}

void Div10Convert::encode(uint8_t* buf, uint8_t len, const VariantValue& val) const {
  assert(len == 1 || len == 2);
  (void) len;
  float srcVal = val;
  int16_t tmp = std::floor((srcVal * 10.f) + 0.5f);
  if (len == 2) {
    buf[1] = tmp >> 8;
  }
  buf[0] = tmp & 0xFF;
}

VariantValue Div2Convert::decode(const uint8_t* data, uint8_t len) const {
  assert(len == 1);
  float retVal = 0;
  int8_t val = data[0];
  retVal = val / 2.f;
    return VariantValue(retVal);
}

void Div2Convert::encode(uint8_t* buf, uint8_t len, const VariantValue& val) const {
  assert(len == 1);
  (void) len;
  float srcVal = val;
  int8_t tmp = std::floor((srcVal * 2.f) + 0.5f);
  buf[0] = tmp;
}

VariantValue Div3600Convert::decode(const uint8_t* data, uint8_t len) const {
  assert(len == 4);
  (void) len;
  uint32_t val = data[3] << 24 | data[2] << 16 | data[1] << 8 | data[0];
  float retVal = val / 3600.f;
  return VariantValue(retVal);
}

void Div3600Convert::encode(uint8_t* buf, uint8_t len, const VariantValue& val) const {
  assert(len == 4);
  (void) len;
  float srcVal = val;
  uint32_t tmp = std::floor((srcVal * 3600.f) + 0.5f);
  buf[3] = tmp >> 24;
  buf[2] = tmp >> 16;
  buf[1] = tmp >> 8;
  buf[0] = tmp & 0xFF;
}

VariantValue NoconvConvert::decode(const uint8_t* data, uint8_t len) const {
  // assert(len == 1 || len == 2 || len == 4);
  if (len == 1) {
    uint8_t retVal = data[0];
    return VariantValue(retVal);
  } else if (len == 2) {
    uint16_t retVal = data[1] << 8 | data[0];
    return VariantValue(retVal);
  } else if (len == 4) {
    uint32_t retVal = data[3] << 24 | data[2] << 16 | data[1] << 8 | data[0];
    return VariantValue(retVal);
  } else {
    // decoding should be done in user code
    uint32_t retVal = 0;
    return VariantValue(retVal);
  }
}

void NoconvConvert::encode(uint8_t* buf, uint8_t len, const VariantValue& val) const {
  // assert(len == 1 || len == 2 || len == 4);
  if (len == 1) {
    uint8_t srcVal = val;
    buf[0] = srcVal;
  } else if (len == 2) {
    uint16_t srcVal = val;
    buf[1] = srcVal >> 8;
    buf[0] = srcVal & 0xFF;
  } else if (len == 4) {
    uint32_t srcVal = val;
    buf[3] = srcVal >> 24;
    buf[2] = srcVal >> 16;
    buf[1] = srcVal >> 8;
    buf[0] = srcVal & 0xFF;
  } else {
    // encoding should be done by user
    std::memset(buf, 0, len);
  }
}

Div10Convert div10;
Div2Convert div2;
Div3600Convert div3600;
NoconvConvert noconv;

}  // end namespace VitoWiFi
