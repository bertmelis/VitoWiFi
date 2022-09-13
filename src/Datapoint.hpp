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

// #include <stdint.h>
// #include <stdio.h>
#include <string.h>  // for strcmp()
#include <functional>
#include <vector>
#include <memory>
#include <algorithm>
#include "DPTypes.hpp"
#include "DPValue.hpp"
#include "Constants.hpp"


// callback and forward declares
class IDatapoint;
typedef std::function<void(const IDatapoint&, DPValue)> Callback;

// Datapoints
class IDatapoint {
  template<class> friend class VitoWiFiClass;

 public:
  IDatapoint(const char* name, const char* group, uint16_t address, bool writeable = false);
  ~IDatapoint();
  const char* getName() const { return _name; }
  const char* getGroup() const { return _group; }
  const uint16_t getAddress() const { return _address; }
  IDatapoint& setAddress(uint16_t address) { _address = address; return *this; }
  const bool isWriteable() const { return _writeable; }
  IDatapoint& setWriteable(bool writeable) { _writeable = writeable; return *this; }
  IDatapoint& setGlobalCallback(Callback cb) { _globalCb = cb; return *this; }
  IDatapoint& setCallback(Callback cb) { _cb = cb; return *this; }
  virtual const size_t getLength() const = 0;
  virtual IDatapoint& setLength(uint8_t length) = 0;
  virtual void encode(uint8_t* out, const DPValue in) = 0;
  virtual DPValue decode(const uint8_t* in) = 0;
  void setValue(DPValue value);

 protected:
  const char* _name;
  const char* _group;
  uint16_t _address;
  bool _writeable;
  static Callback _globalCb;
  Callback _cb;
  static std::vector<IDatapoint*> _dps;

 public:
  // better not use this publicly
  const std::vector<IDatapoint*>& getCollection() const { return _dps; }
};

template <class T>
class Datapoint : public IDatapoint {
 public:
  Datapoint(const char* name, const char* group, uint16_t address, bool writeable = false) :
    IDatapoint(name, group, address, writeable) {}

 protected:
  T _t;

 public:
  const size_t getLength() const { return _t.getLength(); }
  IDatapoint& setLength(uint8_t length) { _t.setLength(length); return *this; }
  void encode(uint8_t* out, const DPValue in)  { _t.encode(out, in); }
  DPValue decode(const uint8_t* in) { return _t.decode(in); }
};

typedef Datapoint<convRaw> DPRaw;
typedef Datapoint<conv2_10_F> DPTemp;
typedef Datapoint<conv1_1_US> DPTempS;
typedef Datapoint<conv1_1_B> DPStat;
typedef Datapoint<conv4_1_UL> DPCount;
typedef Datapoint<conv2_1_UL> DPCountS;
typedef DPTempS DPMode;
typedef Datapoint<conv4_3600_F> DPHours;
typedef Datapoint<conv1_10_F> DPCoP;
typedef Datapoint<conv8_1_Timer> DPTimer;
