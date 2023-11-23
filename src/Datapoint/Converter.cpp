/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "Converter.h"

namespace VitoWiFi {

VariantValue emptyValue(nullptr);

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
  int16_t tmp = floor((srcVal * 10.f) + 0.5);
  if (len == 2) {
    buf[1] = tmp >> 8;
  }
  buf[0] = tmp & 0xFF;
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
  uint32_t tmp = floor((srcVal * 3600.f) + 0.5);
  buf[3] = tmp >> 24;
  buf[2] = tmp >> 16;
  buf[1] = tmp >> 8;
  buf[0] = tmp & 0xFF;
}

VariantValue NoconvConvert::decode(const uint8_t* data, uint8_t len) const {
  assert(len == 1 || len == 2 || len == 4 || len == 8);
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
    uint64_t retVal = ((uint64_t)data[7]) << 56 |
                      ((uint64_t)data[6]) << 48 |
                      ((uint64_t)data[5]) << 40 |
                      ((uint64_t)data[4]) << 32 |
                      ((uint64_t)data[3]) << 24 |
                      ((uint64_t)data[2]) << 16 |
                      ((uint64_t)data[1]) << 8 |
                      data[0];
    return VariantValue(retVal);
  }
}

void NoconvConvert::encode(uint8_t* buf, uint8_t len, const VariantValue& val) const {
  assert(len == 1 || len == 2 || len == 4 || len == 8);
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
    uint64_t srcVal = val;
    buf[7] = srcVal >> 56;
    buf[6] = srcVal >> 48;
    buf[5] = srcVal >> 40;
    buf[4] = srcVal >> 32;
    buf[3] = srcVal >> 24;
    buf[2] = srcVal >> 16;
    buf[1] = srcVal >> 8;
    buf[0] = srcVal & 0xFF;
  }
}

VariantValue NoconvRawConvert::decode(const uint8_t* data, uint8_t len) const {
  (void) len;
  return VariantValue(data);
}

void NoconvRawConvert::encode(uint8_t* buf, uint8_t len, const VariantValue& val) const {
  const uint8_t* data = val;
  std::memcpy(buf, data, len);
}

Div10Convert div10;
Div3600Convert div3600;
NoconvConvert noconv;
NoconvRawConvert raw;

}  // end namespace VitoWiFi
