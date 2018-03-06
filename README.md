# VitoWifi

[![Build Status](https://travis-ci.org/bertmelis/VitoWifi.svg?branch=master)](https://travis-ci.org/bertmelis/VitoWifi)

Arduino Library for ESP8266 to communicate with Viessmann boilers using a (DIY) serial optolink.

Based on the fantastic work on [openv](https://github.com/openv/openv/wiki). Check out this wiki for a simple hardware implementations

* usage: see [USAGE](USAGE.md)
* reference: see [REFERENCE](REFERENCE.md)

**This library is non-blocking**
This also means that the use of blocking code is not supported.
Since we're talking serial @4800 baud, we would be wasting precious processing time while reading and writing to the optolink. Furthermore this library is useable in async frameworks such as the marvellous [Homie for esp8266](https://github.com/marvinroger/homie-esp8266) framework.

## Prerequisites

* Platformio or Arduino for ESP8266
* a (DIY) serial optocoupler to connect your ESP8266 to your Viessmann boiler
* As this is an implementation of the P300 (aka VS2) and KW protocol, your boiler must be compatible with these protocols.

## Installation

* For Arduino IDE: see [the Arduino Guide](https://www.arduino.cc/en/Guide/Libraries#toc4)
* For Platformio: see the [Platfomio lib page for VitoWifi](http://platformio.org/lib/show/1939/VitoWifi)

## Usage

A working example can be found on my [Github page](https://github.com/bertmelis/esp-boiler). I'm using this at home on a Vitodens 200/Vitotronic 200.  
Look into the examples to explore the functionality.

See [USAGE](USAGE.md) for a more in detail explenation or look directly into the [REFERENCE](REFERENCE.md).

## TO DO

* improve/implement error handling
* integrate missing datapoint types and rearrange

## HISTORY

I was using the original openv/vcontrold code from wikispaces on a Raspberry Pi V1. However, this setup was a bit overkill: such a powerful device doing only serial communication at 4800 baud. So I wanted to optimise this situation.
An ESP8266 has built-in serial/UART communication and is obviously connectable via Wifi. The small form factor and power consumption were bonus points. Ever since it has been made compatible with the Arduino environment (or other way around), it is my Nr 1 choice for IoT applications.

## Credits go to

* Hex print: 2011, robtillaart @ Arduino.cc forum
* Logger/Blinker: MIT 2015, marvinroger @ Github
* Serial Protocol @ openv.wikispaces.com
* [tolw](https://github.com/tolw) for impleenting the writing
* [Empor-co](https://github.com/Empor-co) for testing the KW-protocol
* and many others for code and inspiration
