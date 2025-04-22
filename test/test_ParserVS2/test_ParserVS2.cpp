/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include <unity.h>

#include <VS2/ParserVS2.h>

using VitoWiFiInternals::ParserVS2;
using VitoWiFiInternals::ParserResult;
using VitoWiFi::PacketType;
using VitoWiFi::FunctionCode;

void setUp() {}
void tearDown() {}

ParserVS2 parser;

void test_ok_request() {
  const uint8_t stream[] = {
    0x41,  // start byte
    0x05,  // length
    0x00,  // packet type (request)
    0x01,  // flags: id + function code (0 + read)
    0x55,  // address 1
    0x25,  // address 2
    0x02,  // payload length
    0x82   // cs
  };
  const std::size_t length = 8;
  const std::size_t packetLength = 6;

  std::size_t bytesRead = 0;
  ParserResult result = ParserResult::ERROR;

  while (bytesRead < length) {
    result = parser.parse(stream[bytesRead++]);
    if (result != ParserResult::CONTINUE) {
      break;
    }
  }

  TEST_ASSERT_EQUAL(ParserResult::COMPLETE, result);
  TEST_ASSERT_EQUAL_UINT(length, bytesRead);
  TEST_ASSERT_EQUAL_UINT8(packetLength, parser.packet().length());
  TEST_ASSERT_EQUAL_UINT8(PacketType::REQUEST, parser.packet().packetType());
  TEST_ASSERT_EQUAL_UINT8(0x00, parser.packet().id());
  TEST_ASSERT_EQUAL_UINT8(FunctionCode::READ, parser.packet().functionCode());
  TEST_ASSERT_EQUAL_UINT16(0x5525, parser.packet().address());
  TEST_ASSERT_EQUAL_UINT8(0x02, parser.packet().dataLength());
}

void test_ok_readresponse() {
  const uint8_t stream[] = {
    0x41,  // start byte
    0x07,  // length
    0x01,  // packet type (response)
    0x01,  // flags: id + function code (0 + read)
    0x55,  // address 1
    0x25,  // address 2
    0x02,  // payload length
    0x07,  // payload
    0x01,
    0x8D   // cs
  };
  const std::size_t length = 10;
  const std::size_t packetLength = 8;
  const uint8_t data[2] = {0x07, 0x01};

  std::size_t bytesRead = 0;
  ParserResult result = ParserResult::ERROR;

  while (bytesRead < length) {
    result = parser.parse(stream[bytesRead++]);
    if (result != ParserResult::CONTINUE) {
      break;
    }
  }

  TEST_ASSERT_EQUAL(ParserResult::COMPLETE, result);
  TEST_ASSERT_EQUAL_UINT(length, bytesRead);
  TEST_ASSERT_EQUAL_UINT8(packetLength, parser.packet().length());
  TEST_ASSERT_EQUAL_UINT8(PacketType::RESPONSE, parser.packet().packetType());
  TEST_ASSERT_EQUAL_UINT8(0x00, parser.packet().id());
  TEST_ASSERT_EQUAL_UINT8(FunctionCode::READ, parser.packet().functionCode());
  TEST_ASSERT_EQUAL_UINT16(0x5525, parser.packet().address());
  TEST_ASSERT_EQUAL_UINT8(0x02, parser.packet().dataLength());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(data, parser.packet().data(), 2);
}

void test_ok_writeresponse() {
  const uint8_t stream[] = {
    0x41,  // start byte
    0x06,  // length
    0x01,  // packet type (response)
    0x02,  // flags: id + function code (0 + write)
    0x23,  // address 1
    0x23,  // address 2
    0x01,  // payload length
    0x50   // cs
  };
  const std::size_t length = 8;
  const std::size_t packetLength = 7;

  std::size_t bytesRead = 0;
  ParserResult result = ParserResult::ERROR;

  while (bytesRead < length) {
    result = parser.parse(stream[bytesRead++]);
    if (result != ParserResult::CONTINUE) {
      break;
    }
  }

  TEST_ASSERT_EQUAL(ParserResult::COMPLETE, result);
  TEST_ASSERT_EQUAL_UINT(length, bytesRead);
  TEST_ASSERT_EQUAL_UINT8(packetLength, parser.packet().length());
  TEST_ASSERT_EQUAL_UINT8(PacketType::RESPONSE, parser.packet().packetType());
  TEST_ASSERT_EQUAL_UINT8(0x00, parser.packet().id());
  TEST_ASSERT_EQUAL_UINT8(FunctionCode::WRITE, parser.packet().functionCode());
  TEST_ASSERT_EQUAL_UINT16(0x2323, parser.packet().address());
  TEST_ASSERT_EQUAL_UINT8(0x01, parser.packet().dataLength());
  TEST_ASSERT_NULL(parser.packet().data());
}

void test_spuriousbytes() {
  const uint8_t stream[] = {
    0x05,
    0x01,
    0x41,  // start byte
    0x07,  // length
    0x01,  // packet type (response)
    0x01,  // flags: id + function code (0 + read)
    0x55,  // address 1
    0x25,  // address 2
    0x02,  // payload length
    0x07,  // payload
    0x01,
    0x8D   // cs
  };
  const std::size_t length = 12;
  const std::size_t packetLength = 8;
  const uint8_t data[2] = {0x07, 0x01};

  std::size_t bytesRead = 0;
  ParserResult result = ParserResult::ERROR;

  while (bytesRead < length) {
    result = parser.parse(stream[bytesRead++]);
    if (result != ParserResult::CONTINUE) {
      break;
    }
  }

  TEST_ASSERT_EQUAL(ParserResult::COMPLETE,  result);
  TEST_ASSERT_EQUAL_UINT(length, bytesRead);
  TEST_ASSERT_EQUAL_UINT8(packetLength, parser.packet().length());
  TEST_ASSERT_EQUAL_UINT8(PacketType::RESPONSE, parser.packet().packetType());
  TEST_ASSERT_EQUAL_UINT8(0x00, parser.packet().id());
  TEST_ASSERT_EQUAL_UINT8(FunctionCode::READ, parser.packet().functionCode());
  TEST_ASSERT_EQUAL_UINT16(0x5525, parser.packet().address());
  TEST_ASSERT_EQUAL_UINT8(0x02, parser.packet().dataLength());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(data, parser.packet().data(), 2);
}

void test_invalidLength() {
  const uint8_t stream[] = {
    0x41,  // start byte
    0x00,  // length
    0x01,  // packet type (response)
    0x01,  // flags: id + function code (0 + read)
    0x55,  // address 1
    0x25,  // address 2
    0x02,  // payload length
    0x07,  // payload
    0x01,
    0x8D   // cs
  };
  const std::size_t length = 12;

  std::size_t bytesRead = 0;
  ParserResult result = ParserResult::ERROR;

  while (bytesRead < length) {
    result = parser.parse(stream[bytesRead++]);
    if (result != ParserResult::CONTINUE) {
      break;
    }
  }

  TEST_ASSERT_EQUAL(ParserResult::ERROR, result);
  TEST_ASSERT_EQUAL_UINT(2, bytesRead);
}

void test_invalidPacketType() {
  const uint8_t stream[] = {
    0x41,  // start byte
    0x07,  // length
    0x06,  // packet type (invalid)
    0x01,  // flags: id + function code (0 + read)
    0x55,  // address 1
    0x25,  // address 2
    0x02,  // payload length
    0x07,  // payload
    0x01,
    0x8D   // cs
  };
  const std::size_t length = 12;

  std::size_t bytesRead = 0;
  ParserResult result = ParserResult::ERROR;

  while (bytesRead < length) {
    result = parser.parse(stream[bytesRead++]);
    if (result != ParserResult::CONTINUE) {
      break;
    }
  }

  TEST_ASSERT_EQUAL(ParserResult::ERROR, result);
  TEST_ASSERT_EQUAL_UINT(3, bytesRead);
}

void test_invalidFunctionCode() {
  const uint8_t stream[] = {
    0x41,  // start byte
    0x07,  // length
    0x01,  // packet type (response)
    0x05,  // flags: id + function code (0 + read)
    0x55,  // address 1
    0x25,  // address 2
    0x02,  // payload length
    0x07,  // payload
    0x01,
    0x8D   // cs
  };
  const std::size_t length = 12;

  std::size_t bytesRead = 0;
  ParserResult result = ParserResult::ERROR;

  while (bytesRead < length) {
    result = parser.parse(stream[bytesRead++]);
    if (result != ParserResult::CONTINUE) {
      break;
    }
  }

  TEST_ASSERT_EQUAL(ParserResult::ERROR, result);
  TEST_ASSERT_EQUAL_UINT(4, bytesRead);
}

void test_invalidChecksum() {
  const uint8_t stream[] = {
    0x41,  // start byte
    0x07,  // length
    0x01,  // packet type (response)
    0x01,  // flags: id + function code (0 + read)
    0x55,  // address 1
    0x25,  // address 2
    0x02,  // payload length
    0x07,  // payload
    0x01,
    0x8E   // cs
  };
  const std::size_t length = 12;

  std::size_t bytesRead = 0;
  ParserResult result = ParserResult::ERROR;

  while (bytesRead < length) {
    result = parser.parse(stream[bytesRead++]);
    if (result != ParserResult::CONTINUE) {
      break;
    }
  }

  TEST_ASSERT_EQUAL(ParserResult::CS_ERROR, result);
  TEST_ASSERT_EQUAL_UINT(10, bytesRead);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_ok_request);
  RUN_TEST(test_ok_readresponse);
  RUN_TEST(test_ok_writeresponse);
  RUN_TEST(test_spuriousbytes);
  RUN_TEST(test_invalidLength);
  RUN_TEST(test_invalidPacketType);
  RUN_TEST(test_invalidFunctionCode);
  RUN_TEST(test_invalidChecksum);
  return UNITY_END();
}