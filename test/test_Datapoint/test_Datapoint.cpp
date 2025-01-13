/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include <unity.h>

#include <Datapoint/Datapoint.h>
#include <Datapoint/Converter.h>

using VitoWiFi::Datapoint;
using VitoWiFi::PacketVS2;
using VitoWiFi::PacketType;
using VitoWiFi::FunctionCode;
using VitoWiFi::VariantValue;

void setUp() {}
void tearDown() {}

void test_Converter() {
  VitoWiFi::Converter* myConv = &VitoWiFi::div10;
  VitoWiFi::Converter* myOtherConv = &VitoWiFi::div3600;

  TEST_ASSERT_TRUE(*myConv == VitoWiFi::div10);
  TEST_ASSERT_FALSE(*myOtherConv == VitoWiFi::div10);
}

void test_Bool() {
  Datapoint dp("temp", 0x0000, 2, VitoWiFi::div10);
  Datapoint empty(nullptr, 0x0000, 0, VitoWiFi::noconv);

  TEST_ASSERT_TRUE(dp);
  TEST_ASSERT_FALSE(empty);
}

void test_TempDecode() {
  Datapoint dp("temp", 0x0000, 2, VitoWiFi::div10);
  const uint8_t data[] = {0x07, 0x01};
  const float expected = 26.3;
  PacketVS2 packet;
  packet.createPacket(PacketType::RESPONSE, FunctionCode::READ, 0, 0x5525, 2, data);

  float result = dp.decode(packet);

  TEST_ASSERT_EQUAL_FLOAT(expected, result);
}

void test_TempEncode() {
  Datapoint dp("temp", 0x0000, 2, VitoWiFi::div10);
  const uint8_t expected[] = {0x07, 0x01};
  const float value = 26.3;
  const uint8_t len = 2;
  uint8_t buffer[len] = {0};

  dp.encode(buffer, len, VariantValue(value));

  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, buffer, len);
}

void test_TempShortDecode() {
  Datapoint dp("temp", 0x0000, 1, VitoWiFi::noconv);
  const uint8_t data[] = {0x10};
  const uint8_t expected = 16;
  PacketVS2 packet;
  packet.createPacket(PacketType::RESPONSE, FunctionCode::READ, 0, 0x5525, 1, data);

  uint8_t result = dp.decode(packet);

  TEST_ASSERT_EQUAL_UINT8(expected, result);
}

void test_TempShortEncode() {
  Datapoint dp("temp", 0x0000, 1, VitoWiFi::noconv);
  const uint8_t expected[] = {0x10};
  const uint8_t value = 16;
  const uint8_t len = 1;
  uint8_t buffer[len] = {0};

  dp.encode(buffer, len, VariantValue(value));

  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, buffer, len);
}

void test_StatusDecode() {
  Datapoint dp("temp", 0x0000, 1, VitoWiFi::noconv);
  const uint8_t data[] = {0x01};
  PacketVS2 packet;
  packet.createPacket(PacketType::RESPONSE, FunctionCode::READ, 0, 0x5525, 1, data);

  bool result = dp.decode(packet);

  TEST_ASSERT_TRUE(result);
}

void test_StatusEncode() {
  Datapoint dp("temp", 0x0000, 1, VitoWiFi::noconv);
  const uint8_t expected[] = {0x01};
  const bool value = true;
  const uint8_t len = 1;
  uint8_t buffer[len] = {0};

  dp.encode(buffer, len, VariantValue(value));

  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, buffer, len);
}

void test_HourDecode() {
  Datapoint dp("hour", 0x0000, 4, VitoWiFi::div3600);
  const uint8_t data[] = {0x80, 0x8F, 0x21, 0x61};
  const float expected = 452663.72;
  PacketVS2 packet;
  packet.createPacket(PacketType::RESPONSE, FunctionCode::READ, 0, 0x0000, 4, data);

  float result = dp.decode(packet);

  TEST_ASSERT_EQUAL_FLOAT(expected, result);
}

void test_HourEncode() {
  Datapoint dp("hour", 0x0000, 4, VitoWiFi::div3600);
  const uint8_t expected[] = {0x80, 0x8F, 0x21, 0x61};
  const float value = 452663.72;
  const uint8_t len = 4;
  uint8_t buffer[len] = {0};

  dp.encode(buffer, len, VariantValue(value));

  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, buffer, len);
}


void test_CountDecode() {
  Datapoint dp("count", 0x0000, 4, VitoWiFi::noconv);
  const uint8_t data[] = {0xD8, 0xCA, 0x56, 0x1B};
  const uint32_t expected = 458672856;
  PacketVS2 packet;
  packet.createPacket(PacketType::RESPONSE, FunctionCode::READ, 0, 0x0000, 4, data);

  uint32_t result = dp.decode(packet);

  TEST_ASSERT_EQUAL_UINT32(expected, result);
}

void test_CountEncode() {
  Datapoint dp("count", 0x0000, 4, VitoWiFi::noconv);
  const uint8_t expected[] = {0xD8, 0xCA, 0x56, 0x1B};
  const uint32_t value = 458672856;
  const uint8_t len = 4;
  uint8_t buffer[len] = {0};

  dp.encode(buffer, len, VariantValue(value));

  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, buffer, len);
}

void test_CountShortDecode() {
  Datapoint dp("count", 0x0000, 2, VitoWiFi::noconv);
  const uint8_t data[] = {0x26, 0xB3};
  const uint16_t expected = 45862;
  PacketVS2 packet;
  packet.createPacket(PacketType::RESPONSE, FunctionCode::READ, 0, 0x0000, 2, data);

  uint16_t result = dp.decode(packet);

  TEST_ASSERT_EQUAL_UINT16(expected, result);
}

void test_CountShortEncode() {
  Datapoint dp("count", 0x0000, 2, VitoWiFi::noconv);
  const uint8_t expected[] = {0x26, 0xB3};
  const uint16_t value = 45862;
  const uint8_t len = 2;
  uint8_t buffer[len] = {0};

  dp.encode(buffer, len, VariantValue(value));

  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, buffer, len);
}

void test_COPDecode() {
  Datapoint dp("count", 0x0000, 1, VitoWiFi::div10);
  const uint8_t data[] = {0x1A};
  const float expected = 2.6;
  PacketVS2 packet;
  packet.createPacket(PacketType::RESPONSE, FunctionCode::READ, 0, 0x0000, 1, data);

  float result = dp.decode(packet);

  TEST_ASSERT_EQUAL_FLOAT(expected, result);
}

void test_COPEncode() {
  Datapoint dp("count", 0x0000, 2, VitoWiFi::div10);
  const uint8_t expected[] = {0x1A};
  const float value = 2.6;
  const uint8_t len = 1;
  uint8_t buffer[len] = {0};

  dp.encode(buffer, len, VariantValue(value));

  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, buffer, len);
}

void test_ScheduleEncode() {
  const char* schedule = "7:30 08:30 16:20 23:10";
  const uint8_t expected[] = {0x3B, 0x43, 0x82, 0xB9, 0x00, 0x00, 0x00, 0x00};
  const size_t numSchedules = 2;
  const std::size_t len = 8;
  uint8_t buffer[len];

  std::size_t result = VitoWiFi::encodeSchedule(schedule, buffer);

  TEST_ASSERT_EQUAL(numSchedules, result);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, buffer, len);
}

void test_ScheduleDecode() {
  const uint8_t data[] = {0x3B, 0x43, 0x82, 0xB9, 0x00, 0x00, 0x00, 0x00};
  const std::size_t len = 8;
  const char* expected = "07:30 08:30 16:20 23:10 00:00 00:00 00:00 00:00";
  const std::size_t bufferLen = 48;
  char buffer[bufferLen];

  std::size_t result = VitoWiFi::decodeSchedule(data, len, buffer, bufferLen);

  TEST_ASSERT_EQUAL(bufferLen, result);
  TEST_ASSERT_EQUAL_STRING_LEN(expected, buffer, bufferLen)
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_Converter);
  RUN_TEST(test_Bool);
  RUN_TEST(test_TempDecode);
  RUN_TEST(test_TempEncode);
  RUN_TEST(test_TempShortDecode);
  RUN_TEST(test_TempShortEncode);
  RUN_TEST(test_StatusDecode);
  RUN_TEST(test_StatusEncode);
  RUN_TEST(test_HourDecode);
  RUN_TEST(test_HourEncode);
  RUN_TEST(test_CountDecode);
  RUN_TEST(test_CountEncode);
  RUN_TEST(test_CountShortDecode);
  RUN_TEST(test_CountShortEncode);
  RUN_TEST(test_COPDecode);
  RUN_TEST(test_COPEncode);
  RUN_TEST(test_ScheduleEncode);
  RUN_TEST(test_ScheduleDecode);
  return UNITY_END();
}