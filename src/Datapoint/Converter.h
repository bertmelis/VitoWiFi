/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include <cassert>
#include <cstdint>
#include <cmath>
#include <cstring>

#include "../Logging.h"
#include "ConversionHelpers.h"

namespace VitoWiFi {

class VariantValue {
 public:
  explicit VariantValue(uint8_t value): _value(value) {}
  explicit VariantValue(uint16_t value): _value(value) {}
  explicit VariantValue(uint32_t value): _value(value) {}
  explicit VariantValue(uint64_t value): _value(value) {}
  explicit VariantValue(float value): _value(value) {}
  explicit VariantValue(bool value) : _value(value) {}
  operator uint8_t() const { return _value._uint8Val; }
  operator uint16_t() const { return _value._uint16Val; }
  operator uint32_t() const { return _value._uint32Val; }
  operator uint64_t() const { return _value._uint64Val; }
  operator float() const { return _value._floatVal; }
  operator bool() const { return _value._uint8Val; }

 protected:
  union _Value {
    _Value(uint8_t v): _uint8Val(v) {}
    _Value(uint16_t v): _uint16Val(v) {}
    _Value(uint32_t v): _uint32Val(v) {}
    _Value(uint64_t v): _uint64Val(v) {}
    _Value(float v): _floatVal(v) {}
    _Value(bool v): _uint8Val(v) {}
    uint8_t _uint8Val;
    uint16_t _uint16Val;
    uint32_t _uint32Val;
    uint64_t _uint64Val;
    float _floatVal;
  } _value;
};

class Converter {
 public:
  virtual VariantValue decode(const uint8_t* data, uint8_t len) const = 0;
  virtual void encode(uint8_t* buf, uint8_t len, const VariantValue& val) const = 0;
  bool operator==(const Converter& rhs) const {
    return (this == &rhs);
  }
};

class Div10Convert : public Converter {
 public:
  VariantValue decode(const uint8_t* data, uint8_t len) const override;
  void encode(uint8_t* buf, uint8_t len, const VariantValue& val) const override;
};

class Div2Convert : public Converter {
 public:
  VariantValue decode(const uint8_t* data, uint8_t len) const override;
  void encode(uint8_t* buf, uint8_t len, const VariantValue& val) const override;
};

class Div3600Convert : public Converter {
 public:
  VariantValue decode(const uint8_t* data, uint8_t len) const override;
  void encode(uint8_t* buf, uint8_t len, const VariantValue& val) const override;
};

class NoconvConvert : public Converter {
 public:
  VariantValue decode(const uint8_t* data, uint8_t len) const override;
  void encode(uint8_t* buf, uint8_t len, const VariantValue& val) const override;
};

extern Div10Convert div10;
extern Div2Convert div2;
extern Div3600Convert div3600;
extern NoconvConvert noconv;

}  // end namespace VitoWiFi
