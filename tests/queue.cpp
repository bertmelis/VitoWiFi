/* Copyright 2019 Bert Melis */

#include <Helpers/SimpleQueue.h>

#include <assert.h>
#include <iostream>
using std::cout;

#define BUFFER_SIZE 5

bool succes = true;
bool testResult = true;

int main() {
  // Create queue
  SimpleQueue<uint8_t>* queue = new SimpleQueue<uint8_t>(BUFFER_SIZE);

  cout << "TEST adding to queue\n";
  testResult = true;
  for (uint8_t i = 0; i < BUFFER_SIZE; ++i) {
    testResult = queue->add(i);
  }
  assert(testResult && "FAIL adding\n");

  cout << "TEST size after adding\n";
  assert(queue->size() == BUFFER_SIZE && "FAIL size (adding)\n");

  cout << "TEST adding when full\n";
  assert(!queue->add(6) && "FAIL overfilling\n");

  cout << "TEST size after overfill\n";
  assert(queue->size() == BUFFER_SIZE && "FAIL size (overfilling)\n");

  cout << "TEST add after rollover\n";
  queue->remove();
  assert(queue->add(5) && "FAIL rollover\n");

  cout << "TEST reading/removing\n";
  testResult = true;
  for (uint8_t i = 0; i < BUFFER_SIZE; ++i) {
    if (i + 1 == *queue->first()) {
      queue->remove();
    } else {
      testResult = false;
    }
  }
  assert(testResult && "FAIL removing\n");

  cout << "TEST size after removing\n";
  assert(queue->size() == 0 && "FAIL size (removing)");

  cout << "TEST empty queue\n";
  assert(queue->first() && "FAIL empty queue");

  cout << "TEST deleting queue\n";
  delete queue;

  return 0;
}
