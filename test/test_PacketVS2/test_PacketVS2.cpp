/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include <unity.h>

#include <VS2/PacketVS2.h>

using VitoWiFi::PacketVS2;
using VitoWiFi::PacketType;
using VitoWiFi::FunctionCode;

void setUp() {}
void tearDown() {}

void test_ok_requestRead() {
  const uint8_t data[] = {
    0x05,  // length
    0x00,  // packet type (request)
    0x01,  // flags: id + function code (0 + read)
    0x55,  // address 1
    0x25,  // address 2
    0x02   // payload length
  };
  const std::size_t length = 5;
  const uint8_t checksum = 0x82;

  PacketVS2 packet;
  packet.createPacket(PacketType::REQUEST,
                      FunctionCode::READ,
                      0,
                      0x5525,
                      2);

  TEST_ASSERT_TRUE(packet);
  TEST_ASSERT_EQUAL(length, packet.length());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(data, &packet[0], length);
  TEST_ASSERT_EQUAL_UINT8(checksum, packet.checksum());
}

void test_ok_requestWrite() {
  const uint8_t data[] = {
    0x06,  // length
    0x00,  // packet type (request)
    0x02,  // flags: id + function code (0 + read)
    0x23,  // address 1
    0x23,  // address 2
    0x01,  // payload length
    0x01   // payload
  };
  const std::size_t length = 6;
  const uint8_t checksum = 0x50;

  uint8_t payload[1] = {0x01};
  PacketVS2 packet;
  packet.createPacket(PacketType::REQUEST,
                      FunctionCode::WRITE,
                      0,
                      0x2323,
                      1,
                      payload);

  TEST_ASSERT_TRUE(packet);
  TEST_ASSERT_EQUAL(length, packet.length());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(data, &packet[0], length);
  TEST_ASSERT_EQUAL_UINT8(checksum, packet.checksum());
}

void test_packetId() {
  const uint8_t data[] = {
    0x05,  // length
    0x00,  // packet type (request)
    0xA1,  // flags: id + function code (5 + read)
    0x55,  // address 1
    0x25,  // address 2
    0x02   // payload length
  };
  const uint8_t packetId = 5;
  const std::size_t length = 5;
  const uint8_t checksum = 0x22;

  PacketVS2 packet;
  packet.createPacket(PacketType::REQUEST,
                      FunctionCode::READ,
                      5,
                      0x5525,
                      2);

  TEST_ASSERT_TRUE(packet);
  TEST_ASSERT_EQUAL(length, packet.length());
  TEST_ASSERT_EQUAL(packetId, packet.id());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(data, &packet[0], length);
  TEST_ASSERT_EQUAL_UINT8(checksum, packet.checksum());
}

void test_payloadLength() {
  PacketVS2 packet;
  packet.createPacket(PacketType::REQUEST,
                      FunctionCode::READ,
                      5,
                      0x5525,
                      0);

  TEST_ASSERT_FALSE(packet ? true : false);  // contextually convert to bool
}

void test_payloadData() {
  PacketVS2 packet;
  packet.createPacket(PacketType::REQUEST,
                      FunctionCode::WRITE,
                      5,
                      0x5525,
                      2);

  TEST_ASSERT_FALSE(packet ? true : false);  // contextually convert to bool
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_ok_requestRead);
  RUN_TEST(test_ok_requestWrite);
  RUN_TEST(test_packetId);
  RUN_TEST(test_payloadLength);
  RUN_TEST(test_payloadData);
  return UNITY_END();
}