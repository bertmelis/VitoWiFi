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
#include <stdio.h>
#include <math.h>  // floor()
#include <string.h>
#include "DPValue.hpp"
#include "Constants.hpp"

// Encoding

/*
Encoding is always with endianess of communication in mind: little endian.
so high byte comes last:
26.3°C --> 0x0107 --> 07 01 in optolink bytestream
9.1°C  --> 0x005B --> 5B 00 in optolink bytestream

*/

// forward declare
class DPValue;

class DPType {
 public:
  DPType() : _length(0) {}
  void setLength(size_t length) { _length = length; }
  virtual const size_t getLength() const { return _length; }
 protected:
  size_t _length;
};

class convRaw : public DPType {
 public:
  void encode(uint8_t* out, DPValue in);
  DPValue decode(const uint8_t* in);
};

// naming: conv _ #bytes _ factor _ type
class conv2_10_F : public DPType {
 public:
  void encode(uint8_t* out, DPValue in);
  DPValue decode(const uint8_t* in);
  const size_t getLength() const { return 2; }
};

class conv1_1_US : public DPType {
 public:
  void encode(uint8_t* out, DPValue in);
  DPValue decode(const uint8_t* in);
  const size_t getLength() const { return 1; }
};

class conv1_1_B : public DPType {
 public:
  void encode(uint8_t* out, DPValue in);
  DPValue decode(const uint8_t* in);
  const size_t getLength() const { return 1; }
};

class conv4_1_UL : public DPType {
 public:
  void encode(uint8_t* out, DPValue in);
  DPValue decode(const uint8_t* in);
  const size_t getLength() const { return 4; }
};

class conv2_1_UL : public DPType {
 public:
  void encode(uint8_t* out, DPValue in);
  DPValue decode(const uint8_t* in);
  const size_t getLength() const { return 2; }
};

class conv4_3600_F : public DPType {
 public:
  void encode(uint8_t* out, DPValue in);
  DPValue decode(const uint8_t* in);
  const size_t getLength() const { return 4; }
};

class conv1_10_F : public DPType {
 public:
  void encode(uint8_t* out, DPValue in);
  DPValue decode(const uint8_t* in);
  const size_t getLength() const { return 1; }
};

class conv2_1_US : public DPType {
 public:
  void encode(uint8_t* out, DPValue in);
  DPValue decode(const uint8_t* in);
  const size_t getLength() const { return 2; }
};

class conv8_1_Timer : public DPType {
 public:
  void encode(uint8_t* out, DPValue in);
  DPValue decode(const uint8_t* in);
  const size_t getLength() const { return 8; }
};
