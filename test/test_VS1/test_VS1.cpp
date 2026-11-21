/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include <unity.h>

#include <deque>
#include <vector>

#include <VS1/VS1.h>

using VitoWiFi::Datapoint;
using VitoWiFi::OptolinkResult;
using VitoWiFi::VS1;

// Minimal serial interface for VS1's GenericInterface template:
// bytes in rx are what the boiler sends, tx collects what VS1 writes.
class MockInterface {
 public:
  bool begin() { return true; }
  void end() {}
  std::size_t write(const uint8_t* data, uint8_t length) {
    tx.insert(tx.end(), data, data + length);
    return length;
  }
  uint8_t read() {
    uint8_t b = rx.front();
    rx.pop_front();
    return b;
  }
  size_t available() { return rx.size(); }

  std::deque<uint8_t> rx;
  std::vector<uint8_t> tx;
};

MockInterface* mock = nullptr;
VS1* optolink = nullptr;
std::vector<uint8_t> response;
unsigned int responseCount = 0;
unsigned int errorCount = 0;

void setUp() {
  mock = new MockInterface;
  optolink = new VS1(mock);
  response.clear();
  responseCount = 0;
  errorCount = 0;
  optolink->onResponse([](const uint8_t* data, uint8_t length, const Datapoint& request) {
    (void) request;
    response.assign(data, data + length);
    ++responseCount;
  });
  optolink->onError([](OptolinkResult error, const Datapoint& request) {
    (void) error;
    (void) request;
    ++errorCount;
  });
}

void tearDown() {
  delete optolink;
  optolink = nullptr;
  delete mock;
  mock = nullptr;
}

// Sync on ENQ and send the read request: 0x01 + F7 <addr hi> <addr lo> <len>
void sendRequest(uint16_t address, uint8_t length) {
  TEST_ASSERT_TRUE(optolink->begin());
  TEST_ASSERT_TRUE(optolink->read(Datapoint("dp", address, length, VitoWiFi::noconv)));
  mock->rx.push_back(0x05);  // ENQ from the boiler
  for (int i = 0; i < 10 && mock->tx.size() < 5; ++i) {
    optolink->loop();
  }
  const std::vector<uint8_t> expected = {
    0x01, 0xF7,
    static_cast<uint8_t>(address >> 8),
    static_cast<uint8_t>(address & 0xFF),
    length};
  TEST_ASSERT_EQUAL_UINT(expected.size(), mock->tx.size());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected.data(), mock->tx.data(), expected.size());
}

void test_receiveExactLength() {
  sendRequest(0x0800, 2);

  mock->rx.push_back(0x3D);
  mock->rx.push_back(0x00);
  optolink->loop();

  const uint8_t expected[] = {0x3D, 0x00};
  TEST_ASSERT_EQUAL_UINT(1, responseCount);
  TEST_ASSERT_EQUAL_UINT(0, errorCount);
  TEST_ASSERT_EQUAL_UINT(2, response.size());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, response.data(), 2);
  TEST_ASSERT_FALSE(optolink->isBusy());
}

void test_receiveSplitResponse() {
  sendRequest(0x08A7, 4);

  mock->rx.push_back(0x01);
  mock->rx.push_back(0x02);
  optolink->loop();
  TEST_ASSERT_EQUAL_UINT(0, responseCount);

  mock->rx.push_back(0x03);
  mock->rx.push_back(0x04);
  optolink->loop();

  const uint8_t expected[] = {0x01, 0x02, 0x03, 0x04};
  TEST_ASSERT_EQUAL_UINT(1, responseCount);
  TEST_ASSERT_EQUAL_UINT(4, response.size());
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, response.data(), 4);
}

// More bytes in the receive buffer than requested (e.g. optical echo of the
// request or line noise) must not be written past the response buffer.
void test_receiveMoreThanRequested() {
  sendRequest(0x551E, 1);

  mock->rx.push_back(0x01);  // response
  mock->rx.push_back(0xAA);  // surplus bytes
  mock->rx.push_back(0xBB);
  mock->rx.push_back(0xCC);
  mock->rx.push_back(0xDD);
  optolink->loop();

  TEST_ASSERT_EQUAL_UINT(1, responseCount);
  TEST_ASSERT_EQUAL_UINT(0, errorCount);
  TEST_ASSERT_EQUAL_UINT(1, response.size());
  TEST_ASSERT_EQUAL_HEX8(0x01, response[0]);
  // surplus bytes are left in the interface, not consumed into the buffer
  TEST_ASSERT_EQUAL_UINT(4, mock->available());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_receiveExactLength);
  RUN_TEST(test_receiveSplitResponse);
  RUN_TEST(test_receiveMoreThanRequested);
  return UNITY_END();
}
