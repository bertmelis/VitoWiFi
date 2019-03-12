/* Copyright 2019 Bert Melis */

#include <iostream>
using std::cout;

#include <Helpers/SimpleQueue.h>

#define BUFFER_SIZE 5

bool succes = true;
bool testResult = true;

int main() {
  // Create queue
  SimpleQueue<uint8_t>* queue = new SimpleQueue<uint8_t>(BUFFER_SIZE);

  // TEST adding --> buffer = 0, 1, 2, 3, 4
  testResult = true;
  for (uint8_t i = 0; i < BUFFER_SIZE; ++i) {
    testResult = queue->add(i);
  }
  if (testResult) {
    cout << "TEST add OK\n";
  } else {
    cout << "TEST add FAIL\n";
    succes = false;
  }

  // TEST size after adding
  testResult = queue->size() == BUFFER_SIZE;
  if (testResult) {
    cout << "TEST size after add OK\n";
  } else {
    cout << "TEST size after add FAIL\n";
    succes = false;
  }

  // TEST adding when full
  testResult = true;
  testResult = !queue->add(6);
  if (testResult) {
    cout << "TEST add when full OK\n";
  } else {
    cout << "TEST add FAIL\n";
    succes = false;
  }

  // TEST size after overfill
  testResult = queue->size() == BUFFER_SIZE;
  if (testResult) {
    cout << "TEST size after overfill OK\n";
  } else {
    cout << "TEST size after overfill FAIL\n";
    succes = false;
  }

  // TEST add after rollover  --> buffer = 1, 2, 3, 4, 5
  testResult = true;
  queue->remove();
  testResult = queue->add(5);
  if (testResult) {
    cout << "TEST add after rollover OK\n";
  } else {
    cout << "TEST add after rollover FAIL\n";
    succes = false;
  }

  // TEST reading/removing
  testResult = true;
  for (uint8_t i = 0; i < BUFFER_SIZE; ++i) {
    if (i + 1 == *queue->first()) {
      queue->remove();
    } else {
      testResult = false;
    }
  }
  if (testResult) {
    cout << "TEST removing OK\n";
  } else {
    cout << "TEST removing FAIL\n";
    succes = false;
  }

  // TEST size after removing
  testResult = queue->size() == 0;
  if (testResult) {
    cout << "TEST size after removing OK\n";
  } else {
    cout << "TEST size after removing FAIL\n";
    succes = false;
  }

  // TEST empty queue
  testResult = queue->first() == nullptr;
  if (testResult) {
    cout << "TEST empty queue OK\n";
  } else {
    cout << "TEST empty queue FAIL\n";
    succes = false;
  }

  // TEST deleting queue
  delete queue;

  /* end */
  if (succes) {
    cout << "\nAll tests passed!\n\n";
    return 0;
  }
  cout << "\nTest failed!\n\n";
  return 1;
}
