# VitoWiFi

[![Build Status](https://travis-ci.org/bertmelis/VitoWiFi.svg?branch=master)](https://travis-ci.org/bertmelis/VitoWiFi) 
[![Gitter chat](https://badges.gitter.im/gitterHQ/gitter.png)](https://gitter.im/VitoWiFi/VitoWiFi)

Arduino Library for ESP8266 to communicate with Viessmann boilers using a (DIY) serial optolink.

Based on the fantastic work on [openv](https://github.com/openv/openv/wiki). Check out this wiki for a simple hardware implementations

**This library is non-blocking**
This also means that the use of blocking code is not supported.
Since we're talking serial @4800 baud, we would be wasting precious processing time while reading and writing to the optolink. Furthermore this library is useable in async or non blocking frameworks such as the marvellous [Homie for esp8266](https://github.com/marvinroger/homie-esp8266) framework or [Blynk](https://www.blynk.cc).

## Important notice

Version 2 uses a different approach to enable the raw optolink to be used in "regular" Arduino boards. To do this, the API is different and not compatible with version 1.

Second, version 2 is partially untested. I currently don't own a compatible heater anymore so I can not test this library. Without your feedback, I don't know for sure that this library works. Please use the Github issues system or the Gitter channel to raise any questions or signal errors.

Lastly, ESP32 might work but is not supported (mainly due to multithreading behaviour). I'm willing to update the code, but again, I cannot test myself.

Should you prefer version 1, switch branches and look for V1.

## Table of contents

- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [How to use](#how-to-use)
- [Datapoints](#datapoints)
- [DPValue](#dpvalue)
- [Optolink](#optolink)
- [End note](#end-note)

## Prerequisites

* Platformio or Arduino for ESP8266
* a (DIY) serial optocoupler to connect your ESP8266 to your Viessmann boiler
* As this is an implementation of the P300 (aka VS2) and KW protocol, your boiler must be compatible with these protocols.

## Installation

* For Arduino IDE: see [the Arduino Guide](https://www.arduino.cc/en/Guide/Libraries#toc4)
* For Platformio: see the [Platfomio lib page for VitoWifi](http://platformio.org/lib/show/1939/VitoWiFi)

## How to use

Implementing vitoWiFi takes a few steps:
1. define your VitoWiFi object and specify the protocol and interface
2. define all needed datapoints
3. create callback for when data (or errors) is returned (you can also use lambda's)
4. in `void setup()`
  - attach the datapoints to your VitoWiFi object
  - attach the callbacks to the datapoints
  - attach general callback to the VitoWiFi object
  - start VitoWiFi
5. in `void loop()`:
  - call `loop()` regularly. It keeps VitoWiFi running. the `loop()` method is not blocking, you shouldn't block either

These steps are done in the example below

```C++
#include <VitoWiFi.h>  // don't forget to include this lib

// first define your heater
// first argument is the protocol (P300 or KW), second is the serial interface on which you connect the optolink
VitoWiFi vitodens200(P300, &Serial);

// next, define some datapoints, see the "datapoints" section for more info
DPTemp outsideTemp("outsideTemp", 0x5525);
DPTemp boilerTemp("boilertemp", 0x0810);
DPStat pumpStat("pump", 0x2906);

// create a callback function to do something useful with the data
// this one is dedicated for temperature values
void tempCallback(const char* name, float value) {
  float fahrenheit = 0;
  fahrenheit = (5.0 / 9) * (value + 32);
  Serial1.print(name);
  Serial1.print(": ");
  Serial1.println(fahrenheit, 1);  // print with 1 decimal
}

// create a general callback for datapoints that don't get a dedicated one
void dataCallback(const uint8_t* data, uint8_t length, Datapoint* dp) {
  Serial1.print(dp->getName());
  Serial1.print(" is 0x");
  for (uint8_t i = 0; i < length; ++i) Serial1.printf("%02x", data[i]);
  Serial1.print("\n");
}

// create a callback to read error messages
void errorCallback(uint8_t error, Datapoint* dp) {
  Serial1.printf("%s error %u\n", dp->getName(), error);
}

void setup() {
  // attach the "temp" callback to the two temperature datapoints
  outsideTemp.onData([](float value) {
    tempCallback(outsideTemp.getName(), value);
  });
  boilerTemp.onData([](float value) {
    tempCallback(outsideTemp.getName(), value);
  });
  // when attaching callback not on specific datapoints, use your VitoWiFi
  // object instead:
  vitodens200.onData(dataCallback);
  // attach the errorCallback
  vitodens200.onError(errorCallback);
  
  delay(5);

  // setup is done, start VitoWiFi
  vitodens200.begin();
  Serial1.begin(115200);
  Serial1.println(F("Setup finished..."));
}

void loop() {
  static unsigned long lastMillis = 0;
  if (millis() - lastMillis > 60 * 1000UL) {  // read all values every 60 seconds
    lastMillis = millis();
    vitodens200.readAll();
  }
  vitodens200.loop();
}
```

### More examples

Next to the examples in this repo, you can find the firmware of the setup I'm using in this repo: [esp8266-heating](https://github.com/bertmelis/esp8266-heating).

## Datapoints

A datapoint is a 

## History

I was using the original openv/vcontrold code from wikispaces on a Raspberry Pi V1. However, this setup was a bit overkill: such a powerful device doing only serial communication at 4800 baud. So I wanted to optimise this situation.
An ESP8266 has built-in serial/UART communication and is obviously connectable via Wifi. The small form factor and power consumption were bonus points. Ever since it has been made compatible with the Arduino environment (or other way around), it is my Nr 1 choice for IoT applications.

## License

Copyright (c) 2017 Bert Melis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Credits go to

* Hex print: 2011, robtillaart @ Arduino.cc forum
* Logger/Blinker: MIT 2015, marvinroger @ Github
* Serial Protocol @ openv.wikispaces.com
* [tolw](https://github.com/tolw) for implementing the writing
* [Empor-co](https://github.com/Empor-co) for testing the KW-protocol
* and many others for code and inspiration
