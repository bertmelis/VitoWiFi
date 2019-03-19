/* copyright 2019 Bert Melis */

#include <Helpers/SimpleQueue.h>

#include "Includes/catch.hpp"

#define BUFFER_SIZE 5

TEST_CASE("Simple queue", "[SimpleQueue]") {
  SimpleQueue<uint8_t>* queue = new SimpleQueue<uint8_t>(BUFFER_SIZE);
  bool result = false;

  REQUIRE(queue->front() == nullptr);

  for (uint8_t i = 0; i < BUFFER_SIZE; ++i) {
    result = queue->push(i);
    if (!result) break;
  }

  REQUIRE(result == true);
  REQUIRE(queue->size() == BUFFER_SIZE);

  result = queue->push(6);
  REQUIRE(result == false);

  queue->pop();
  result = queue->push(5);
  REQUIRE(result == true);

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
}

