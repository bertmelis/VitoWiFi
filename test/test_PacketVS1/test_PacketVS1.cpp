/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include <unity.h>

#include <VS1/PacketVS1.h>

using VitoWiFi::PacketVS1;
using VitoWiFi::PacketVS1Type;

void setUp() {}
void tearDown() {}

void test_ok_requestRead() {
  const uint8_t data[] = {
    0xF7,  // read
    0x55,  // address 1
    0x25,  // address 2
    0x02   // payload length
  };
  const std::size_t length = 4;

  PacketVS1 packet;
  packet.createPacket(PacketVS1Type.READ,
                      0x5525,
                      2);

  TEST_ASSERT_TRUE(packet);
  TEST_ASSERT_EQUAL(length, packet.length());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(data, &packet[0], length);
}

void test_ok_requestWrite() {
  const uint8_t data[] = {
    0xF4,  // read
    0x23,  // address 1
    0x23,  // address 2
    0x01,  // payload length
    0x01
  };
  const std::size_t length = 5;
  uint8_t payload[1] = {0x01};

  PacketVS1 packet;
  packet.createPacket(PacketVS1Type.WRITE,
                      0x2323,
                      1,
                      payload);

  TEST_ASSERT_TRUE(packet);
  TEST_ASSERT_EQUAL(length, packet.length());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(data, &packet[0], length);
}

void test_packetType() {
  PacketVS1 packet;
  packet.createPacket(0x05,
                      0x5525,
                      2);

  TEST_ASSERT_FALSE(packet ? true : false);  // contextually convert to bool
}

void test_payloadData() {
  PacketVS1 packet;
  packet.createPacket(PacketVS1Type.WRITE,
                      0x5525,
                      2);

  TEST_ASSERT_FALSE(packet ? true : false);  // contextually convert to bool
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_ok_requestRead);
  RUN_TEST(test_ok_requestWrite);
  RUN_TEST(test_packetType);
  RUN_TEST(test_payloadData);
  return UNITY_END();
}