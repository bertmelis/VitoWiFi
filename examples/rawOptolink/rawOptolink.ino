/*

This example defines 3 datapoints of type "TEMP" in a struct array.
Every minute, the array is iterated.

For each Datapoint, the read value is printed as HEX in Serial1

*/

#include <OptolinkP300.hpp>

OptolinkP300 myOptolink;
uint32_t lastMillis = 0;
bool getValues = false;
uint8_t currentIndex = 0;

// Define struct to hold Viessmann datapoints
struct Datapoint {
  uint16_t address;
  uint8_t length;
};
const Datapoint Datapoints[] = {
    0x5525, 2,  // outside temp
    0x0810, 2,  // boiler temp
    0x0812, 2   // DHW temp
};
uint8_t numberOfDatapoints = sizeof(Datapoints) / sizeof(*Datapoints);

void setup() {
  Serial1.begin(115200);

  // Start Viessmann communication on Serial (aka UART0)
  myOptolink.begin(&Serial);
  myOptolink.setLogger(&Serial1);
}

void loop() {
  // put loop() function inside main loop
  myOptolink.loop();

  if (millis() - lastMillis > 1 * 60 * 1000UL) {  // get values every 1 minute
    lastMillis = millis();
    getValues = true;
    currentIndex = 0;
  }

  if (getValues) {
    if (!myOptolink.isBusy()) {
      myOptolink.readFromDP(Datapoints[currentIndex].address, Datapoints[currentIndex].length);
    }
  }

  if (myOptolink.available() > 0) {
    uint8_t value[4] = {0};
    myOptolink.read(value);
    Serial1.print(F("Address "));
    Serial1.print(Datapoints[currentIndex].address, HEX);
    Serial1.print(F(" has value "));
    for (uint8_t i = 0; i <= 4; ++i) {
      Serial1.print(value[i], HEX);
    }
    Serial1.println();
    ++currentIndex;
  }
  if (myOptolink.available() < 0) {
    Serial1.println(myOptolink.readError());
    ++currentIndex;
  }
  if (currentIndex >= numberOfDatapoints) getValues = false;
}
