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

#include "DPTypes.hpp"

void convRaw::encode(uint8_t* out, DPValue in) {
  uint8_t tmp[MAX_DP_LENGTH] = { 0 };
  in.getRaw(tmp);
  memcpy(out, tmp, _length);
}
DPValue convRaw::decode(const uint8_t* in) {
  uint8_t tmp[MAX_DP_LENGTH] = { 0 };
  memcpy(tmp, in, _length);
  DPValue out(tmp, _length);
  return out;
}

void conv2_10_F::encode(uint8_t* out, DPValue in) {
  int16_t tmp = floor((in.getFloat() * 10) + 0.5);
  out[1] = tmp >> 8;
  out[0] = tmp & 0xFF;
}
DPValue conv2_10_F::decode(const uint8_t* in) {
  int16_t tmp = in[1] << 8 | in[0];
  DPValue out(tmp / 10.0f);
  return out;
}

void conv1_1_US::encode(uint8_t* out, DPValue in) {
  uint8_t tmp = in.getU8();
  out[0] = tmp;
}
DPValue conv1_1_US::decode(const uint8_t* in) {
  DPValue out(in[0]);
  return out;
}

void conv1_1_B::encode(uint8_t* out, DPValue in) {
  out[0] = in.getBool() ? 0x01 : 0x00;
}
DPValue conv1_1_B::decode(const uint8_t* in) {
  DPValue out((*in) ? true : false);
  return out;
}

void conv4_1_UL::encode(uint8_t* out, DPValue in) {
  uint32_t tmp = in.getU32();
  out[3] = tmp >> 24;
  out[2] = tmp >> 16;
  out[1] = tmp >> 8;
  out[0] = tmp & 0xFF;
}

DPValue conv4_1_UL::decode(const uint8_t* in) {
  uint32_t tmp = in[3] << 24 | in[2] << 16 | in[1] << 8 | in[0];
  DPValue out(tmp);
  return out;
}

void conv2_1_UL::encode(uint8_t* out, DPValue in) {
  uint16_t tmp = in.getU16();
  out[1] = tmp >> 8;
  out[0] = tmp & 0xFF;
}
DPValue conv2_1_UL::decode(const uint8_t* in) {
  uint16_t tmp = in[1] << 8 | in[0];
  DPValue out(tmp);
  return out;
}

void conv4_3600_F::encode(uint8_t* out, DPValue in) {
  int32_t tmp = floor((in.getFloat() * 3600) + 0.5);
  out[3] = tmp >> 24;
  out[2] = tmp >> 16;
  out[1] = tmp >> 8;
  out[0] = tmp & 0xFF;
}
DPValue conv4_3600_F::decode(const uint8_t* in) {
  int32_t tmp = in[3] << 24 | in[2] << 16 | in[1] << 8 | in[0];
  DPValue out(tmp / 3600.0f);
  return out;
}

void conv1_10_F::encode(uint8_t* out, DPValue in) {
  out[0] = floor((in.getFloat() * 10) + 0.5);
}
DPValue conv1_10_F::decode(const uint8_t* in) {
  DPValue out(in[0] / 10.0f);
  return out;
}

void conv2_1_US::encode(uint8_t* out, DPValue in) {
  uint16_t tmp = in.getU16();
  out[1] = tmp >> 8;
  out[0] = tmp & 0xFF;
}
DPValue conv2_1_US::decode(const uint8_t* in) {
  uint16_t tmp = in[1] << 8 | in[0];
  DPValue out(tmp);
  return out;
}
