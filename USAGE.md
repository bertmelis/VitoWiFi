# Usage

## Hardware implementation
For detailed descriptions about various hardware implementations you can head over to [openv](https://github.com/openv/openv/wiki/Die-Optolink-Schnittstelle), the main source for information about the Viessmann communication. The simplest circuit is described below.

### Components:

* an **esp8266** or **esp32**<br />
The most expensive but easiest method is to use a development board like an Wemos D1 mini, NodeMCU or others. The benefit is that you have an onboard USB-connection so flashing is made very simple. The more experienced ones can use the cheaper esp8266-modules but may require additional tools for flashing or a bit soldering. I recommend a flash size of a least 1MB. Firmware with Wifi and communication clients will be about 400kB and you need the double of this to enable OTA updating.
* a **180 Ohm** and a **10kOhm** resistor
* a **SFH309FA** transistor (3mm, black housing)
* a **SFH487-2** infrared LED (3mm violet housing)

### Circuit

The circuitry for the link is very simple and can be made on a perfboard.

* circuit<br />
![Optolink circuit](/DOCS/circuit.png?raw=true "Optolink circuit")
* front<br />
![Optolink front](/DOCS/front.png?raw=true "Optolink front")
* back<br />
![Optolink back](/DOCS/circuit.png?raw=true "Optolink back")
* installation<br />
![Optolink installation](/DOCS/circuit.png?raw=true "Optolink installation")

Not on the pictures:  
* The board is placed into a housing with a bit of foam to avoid "optical shortcuts". A hole is drilled in the housing at the led and at the transistor. Keep in mind that, when installed, the led (violet) is at the left and the transistor (black) is at the right side.
* Connect the wires to the RX, TX, 3.3V and ground of your ESP.

### Firmware

#### General remarks

* VitoWifi polls the Optolink-driver and launches a callback when ready. Hence is it not [*truly asynchronous*](https://stackoverflow.com/questions/2625493) but it is non-blocking. Using blocking code (like long `delay()`s) *could* work but is not supported. This library is compatible with asynchronous frameworks like [Homie (for esp8266)](https://github.com/marvinroger/homie-esp8266).
* The Optolink-driver uses the hardware UART of the ESP8266. This means you cannot use this to read the messages back to your computer (via USB for instance). You could use a [telnet-like interface](https://github.com/bertmelis/WifiPrinter) aa *printer* to read debug messages. On the ESP32 there is more than 1 hardware UART so choose an appropriate one.
* Do not connect the optolink to the Viessmann system when flashing via serial as the communication will interfere with the Viessmann signals and results in errors.

#### Installation

On Arduino IDE: link

On Platformio: the library is registered in the library manager

#### Minimal sketch

```Arduino
#include <Arduino.h>
#include <VitoWifi.h>

VitoWifi_setProtocol(P300);  //P300 or KW, mind the underscore!

unsigned long lastMillis = 0;

void handleValue(const char* name, const char* group, const char* value) {
  /*
  /* this function can do something with
  //   name
  //   group
  //   value
  // like: yourPrinter.print("Value for ");
  //       yourPrinter.print(name);
  //       yourPrinter.print(" is ");
  //       yourPrinter.println(value);
  // This prints "Value for boilertemp is 48" on yourPrinter.
  */
  */
}

void setup() {
  VitoWifi.addDatapoint("boilertemp",  "boiler", 0x0810, TEMP);
  VitoWifi.setGlobalCallback(handleValue);
  VitoWifi.setup(&Serial);  //pass by reference
}

void loop() {
  if (millis() - lastMillis > 30000) {  //read values every 30 seconds
    lastMillis = millis();
    VitoWifi.readAll();
  }
  VitoWifi.loop();
}
```
This sketch sets the VitoWifi protocol to `P300` and defines 1 datapoint, boilertemp in group boiler with address 0810. The type of the datapoint is TEMP. `VitoWifi.loop()` polls the optolink and executes `handleValue` as soon as a new value is received.

The sketch is usable on the ESP8266. An ESP32 sketch can be found in the examples.

For the available datapoints for your Viessmann system, please head over to the [openv-wiki](https://github.com/openv/openv/wiki). The maximum number of datapoints is limited only by the amount of RAM your ESP has. Likely your Viessmann device runs faster out of useful datapoints then your ESP runs out of RAM.

A more useful example is my [Homie-boiler](https://github.com/bertmelis/homie-boiler).

#### Advanced usage
The complete overview of available methods and properties can be found [here](ADVANCED.md).


## Limitations and bugs

If you found a bug or want extra features, please use the appropriate github systems. Feel free to fork, adapt, improve. But please share your code.

Known limitations:
* the heating system will not start up when the optolink with VitoWifi firmware is connected
