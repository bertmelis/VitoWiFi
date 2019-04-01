/* copyright 2019 Bert Melis */

#include <memory>

#include <Helpers/SimpleQueue.h>

#include "Includes/catch.hpp"

#define BUFFER_SIZE 5

TEST_CASE("Simple queue with int", "[SimpleQueue]") {
  SimpleQueue<uint8_t>* queue = new SimpleQueue<uint8_t>(BUFFER_SIZE);
  bool result = false;

  REQUIRE(queue->front() == nullptr);

  for (uint8_t i = 0; i < BUFFER_SIZE; ++i) {
    result = queue->push(i);
    if (!result) break;
  }

  REQUIRE(result == true);
  REQUIRE(queue->size() == BUFFER_SIZE);

  result = false;
  result = queue->push(6);
  REQUIRE(result == false);

  result = false;
  queue->pop();
  result = queue->push(5);
  REQUIRE(result == true);

  result = true;
  for (uint8_t i = 0; i < BUFFER_SIZE; ++i) {
    if (i + 1 == *queue->front()) {
      queue->pop();
    } else {
      result = false;
      break;
    }
  }

  REQUIRE(result == true);
  REQUIRE(queue->size() == 0);
  REQUIRE(queue->front() == nullptr);

  delete queue;
}

TEST_CASE("Simple queue with pointer", "[SimpleQueue]") {
  SimpleQueue<uint64_t*>* queue = new SimpleQueue<uint64_t*>(BUFFER_SIZE);
  bool result = false;
  uint64_t* ptr = nullptr;

  REQUIRE(queue->front() == nullptr);

  result = false;  // should (always) return true
  for (uint8_t i = 0; i < BUFFER_SIZE; ++i) {
    ptr = new uint64_t(i);
    result = queue->push(ptr);
    if (!result) break;
  }

  REQUIRE(result == true);
  REQUIRE(queue->size() == BUFFER_SIZE);

  result = true;  // should return false
  ptr = new uint64_t(6);
  result = queue->push(ptr);
  REQUIRE(result == false);
  delete ptr;

  result = false;  // should return true
  delete queue->front();
  queue->pop();
  ptr = new uint64_t(5);
  result = queue->push(ptr);
  REQUIRE(result == true);

  result = true;  // should (always) keep true
  for (uint8_t i = 0; i < BUFFER_SIZE; ++i) {
    if (i + 1 == **queue->front()) {
      delete queue->front();
      queue->pop();
    } else {
      result = false;
      break;
    }
  }

  REQUIRE(result == true);
  REQUIRE(queue->size() == 0);
  REQUIRE(queue->front() == nullptr);
  delete queue;
}
