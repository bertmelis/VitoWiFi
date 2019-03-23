/*

This example defines 3 datapoints of type "TEMP" in a struct array.
Every minute, the array is iterated.

For each Datapoint, the read value is printed as HEX in Serial1

*/

#include <OptolinkP300.h>

OptolinkP300 myOptolink(&Serial);
uint32_t lastMillis = 0;
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
uint8_t numberOfDatapoints = sizeof(Datapoints) / sizeof(*Datapoints);

void onData(uint8_t* data, uint8_t len, void* index) {
  Serial1.print(datapoints[static_cast<size_t>(index)].name);
  Serial1.print(": 0x");
  for (uint8_t i = 0; i < len; ++i) Serial1.printf("%02x", data[i]);
  Serial1.print("\n");
}

void onData(uint8_t error, void* arg) {
  Serial1.printf("%s: error %u", datapoints[static_cast<size_t>(index)].name,
                                 error);
}

void setup() {
  Serial1.begin(115200);

  myOptolink.onData()
  myOptolink.begin();
}

void loop() {
  if (millis() - lastMillis > 1 * 60 * 1000UL) {  // get values every 1 minute
    lastMillis = millis();
    getValues = true;
  }

  if (getValues) {
    getValue = false;
    for (unit8_t i = 0; i < numberOfDatapoints; ++i) {
      myOptolink.read(
        datapoints[i].address,
        datapoints[i].length,
        (void*)i);
    }
  }
  myOptolink.loop();
}
