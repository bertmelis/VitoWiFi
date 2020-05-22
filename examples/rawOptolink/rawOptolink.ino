/*

This example defines 3 datapoints in a struct array.
Every minute, the array is iterated to enqueue all defined datapoints.

On success, the read value is printed as HEX in Serial1.

*/

#include <OptolinkP300.h>

OptolinkP300 myOptolink(&Serial);
unsigned long lastMillis = 0;
bool getValues = false;

// Define struct to hold Viessmann datapoints
struct Datapoint {
  const char* name;
  uint16_t address;
  uint8_t length;
};
const Datapoint datapoints[] = {
  "outsidetemp", 0x5525, 2,
  "boilertemp", 0x0810, 2,
  "dhwtemp", 0x0812, 2
};
uint8_t numberOfDatapoints = sizeof(datapoints) / sizeof(*datapoints);

void printHex(uint8_t* data, uint8_t length) {
  char tmp[length * 2 + 1];
  byte first;
  int j = 0;
  for (uint8_t i = 0; i < length; ++i) {
    first = (data[i] >> 4) | 48;
    if (first > 57) tmp[j] = first + (byte)39;
    else tmp[j] = first ;
    ++j;

    first = (data[i] & 0x0F) | 48;
    if (first > 57) tmp[j] = first + (byte)39; 
    else tmp[j] = first;
    ++j;
  }
  tmp[length * 2] = 0;
  Serial1.println(tmp);
}

void onData(uint8_t* data, uint8_t len, void* index) {
  Serial1.print(datapoints[(size_t)index].name);
  Serial1.print(": 0x");
  printHex(data, len);
  Serial1.print("\n");
}

void onError(uint8_t error, void* arg) {
  Serial1.print(datapoints[(size_t)arg].name);
  Serial1.print(": error ");
  Serial1.println(error);
}

void setup() {
  Serial1.begin(115200);

  myOptolink.onData(onData);
  myOptolink.onError(onError);
  myOptolink.begin();
}

void loop() {
  if (millis() - lastMillis > 1 * 60 * 1000UL) {  // get values every 1 minute
    lastMillis = millis();
    getValues = true;
  }

  if (getValues) {
    getValues = false;
    for (uint8_t i = 0; i < numberOfDatapoints; ++i) {
      myOptolink.read(
        datapoints[i].address,
        datapoints[i].length,
        (void*)i);
    }
  }
  myOptolink.loop();
}
