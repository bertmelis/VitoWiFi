/*

Copyright 2017 Bert Melis

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#pragma once

#include <stdint.h>
#include <math.h>  // floor()
#include <string.h>  // for strcmp()
#include <functional>
#include <vector>
#include <memory>
#include <algorithm>
#include "Constants.h"


// callback and forward declares
class DPValue;
class IDatapoint;
typedef std::function<void(const IDatapoint&, DPValue)> Callback;

// all defined datatypes
enum DPType {
  RAW,     // undefined number of bytes, no conversion
  TEMPL,   // 2 bytes, div10
  TEMPS,   // 1 byte, no factor
  STAT,    // 1 byte, to 0/1
  COUNTL,  // 4 bytes, no factor
  COUNTS,  // 2 bytes, no factor
  MODE,    // 1 byte, no factor (same as TEMPS)
  HOURS,   // 4 bytes, div3600
  COP      // 1 byte, div10
};
enum DPValueType {
  BOOL,
  UINT8_T,
  UINT16_T,
  UINT32_T,
  FLOAT,
  PTR
};

// class which holds the returned or set value
class DPValue {
 private:
  union value {
    struct b_t { DPValueType type; bool value; } b;
    struct u8_t { DPValueType type; uint8_t value; } u8;
    struct u16_t { DPValueType type; uint16_t value; } u16;
    struct u32_t { DPValueType type; uint32_t value; } u32;
    struct f_t { DPValueType type; float value; } f;
    struct raw { DPValueType type; uint8_t value[MAX_DP_LENGTH]; size_t length; } raw;
    // value() : u32{PTR, 0} {}
    value(bool b) : b{BOOL, b} {}
    value(uint8_t u8) : u8{UINT8_T, u8} {}
    value(uint16_t u16) : u16{UINT16_T, u16} {}
    value(uint32_t u32) : u32{UINT32_T, u32} {}
    value(float f) : f{FLOAT, f} {}
    value(uint8_t* r, size_t length) : raw{PTR, {0}, length} {
      if (length <= MAX_DP_LENGTH)
        memcpy(raw.value, r, length);
      else
        memcpy(raw.value, r, MAX_DP_LENGTH);
    }
  } v;

 public:
  // DPValue() : v() {}
  explicit DPValue(bool b) : v(b) {}
  explicit DPValue(uint8_t u8) : v(u8) {}
  explicit DPValue(uint16_t u16) : v(u16) {}
  explicit DPValue(uint32_t u32) : v(u32) {}
  explicit DPValue(float f) : v(f) {}
  DPValue(uint8_t* r, size_t length) : v(r, length) {}
  DPValue(DPValue const&) = default;
  ~DPValue() = default;
  bool getBool() {
    if (v.b.type == BOOL) {
      return v.b.value;
    } else {
      return false;
    }
  }
  uint8_t getU8() {
    if (v.b.type == UINT8_T) {
      return v.u8.value;
    } else {
      return 0;
    }
  }
  uint16_t getU16() {
    if (v.b.type == UINT16_T) {
      return v.u16.value;
    } else {
      return 0;
    }
  }
  uint32_t getU32() {
    if (v.b.type == UINT32_T) {
      return v.u32.value;
    } else {
      return 0;
    }
  }
  float getFloat() {
    if (v.b.type == FLOAT) {
      return v.f.value;
    } else {
      return 0.0;
    }
  }
  void getRaw(uint8_t* out) { memcpy(out, &v.raw.value[0], v.raw.length); }
  size_t getRawLength() { return v.raw.length; }
  void getString(char* c, size_t s) {
    switch (v.b.type) {
    case BOOL:
      snprintf(c, s, "%s", (v.b.value) ? "true" : "false");
      break;
    case UINT8_T:
      snprintf(c, s, "%u", v.u8.value);
      break;
    case UINT16_T:
      snprintf(c, s, "%u", v.u16.value);
      break;
    case UINT32_T:
      snprintf(c, s, "%u", v.u32.value);
      break;
    case FLOAT:
      snprintf(c, s, "%.1f", v.f.value);
      break;
    case PTR:
      for (uint8_t i = 0; i < v.raw.length; ++i) {
        snprintf(c, s, "%02x", v.raw.value[i]);
        c+=2;
      }
      break;
    }
  }
};

// Encoding

/*
Encoding is always with endianess of communication in mind: little endian.
so high byte comes last:
26.3°C --> 0x0107 --> 07 01 in optolink bytestream
9.1°C  --> 0x005B --> 5B 00 in optolink bytestream

*/

class convRaw {
 public:
  void encode(uint8_t* out, DPValue in, size_t length);
  DPValue decode(const uint8_t* in, size_t length);
};

// naming: conv _ #bytes _ factor _ type
class conv2_10_F {
 public:
  void encode(uint8_t* out, DPValue in, size_t length);
  DPValue decode(const uint8_t* in, size_t length);
};

class conv1_1_US {
 public:
  void encode(uint8_t* out, DPValue in, size_t length);
  DPValue decode(const uint8_t* in, size_t length);
};

class conv1_1_B {
 public:
  void encode(uint8_t* out, DPValue in, size_t length);
  DPValue decode(const uint8_t* in, size_t length);
};

class conv4_1_UL {
 public:
  void encode(uint8_t* out, DPValue in, size_t length);
  DPValue decode(const uint8_t* in, size_t length);
};

class conv2_1_UL {
 public:
  void encode(uint8_t* out, DPValue in, size_t length);
  DPValue decode(const uint8_t* in, size_t length);
};

class conv4_3600_F {
 public:
  void encode(uint8_t* out, DPValue in, size_t length);
  DPValue decode(const uint8_t* in, size_t length);
};

class conv1_10_F {
 public:
  void encode(uint8_t* out, DPValue in, size_t length);
  DPValue decode(const uint8_t* in, size_t length);
};

class conv2_1_US {
 public:
  void encode(uint8_t* out, DPValue in, size_t length);
  DPValue decode(const uint8_t* in, size_t length);
};

// Datapoints
class IDatapoint {
 public:
  IDatapoint(const char* name, const char* group, uint16_t address, size_t length, bool writeable = false);
  const char* getName() const { return _name; }
  const char* getGroup() const { return _group; }
  const uint16_t getAddress() const { return _address; }
  IDatapoint& setAddress(uint16_t address) { _address = address; return *this; }
  const size_t getLength() const { return _length; }
  IDatapoint& setLength(uint8_t length) { _length = length; return *this; }
  const bool isWriteable() const { return _writeable; }
  IDatapoint& setWriteable(bool writeable) { _writeable = writeable; return *this; }
  IDatapoint& setGlobalCallback(Callback cb) { _globalCb = cb; return *this; }
  IDatapoint& setCallback(Callback cb) { _cb = cb; return *this; }
  virtual void encode(uint8_t* out, const DPValue in) {}
  virtual DPValue decode(const uint8_t* in) { uint32_t tmp = 0; DPValue out(tmp); return out; }
  void callback(DPValue value);
 protected:
  const char* _name;
  const char* _group;
  uint16_t _address;
  size_t _length;
  bool _writeable;
  static Callback _globalCb;
  Callback _cb;
};

template <class T>
class Datapoint : public IDatapoint {
 public:
  Datapoint(const char* name, const char* group, uint16_t address, size_t length, bool writeable) :
    IDatapoint(name, group, address, length, writeable) {}
 protected:
  T _t;
 public:
  void encode(uint8_t* out, const DPValue in)  { _t.encode(out, in, _length); }
  DPValue decode(const uint8_t* in) { return _t.decode(in, _length); }
};

// Manager
class DPManager {
 public:
  IDatapoint& addDP(const char* name, const char* group, uint16_t address, DPType type, bool writeable = false);
  IDatapoint& addDP(const char* name, const char* group, uint16_t address, size_t length, DPType type, bool writeable = false);
  std::vector<std::shared_ptr<IDatapoint>>::iterator begin() { return _dps.begin(); }
  std::vector<std::shared_ptr<IDatapoint>>::iterator end() { return _dps.end(); }
  size_t size() { return _dps.size(); }
  IDatapoint* at(size_t n);
 private:
  std::vector<std::shared_ptr<IDatapoint>> _dps;
};
