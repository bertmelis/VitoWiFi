# VitoWifi
[![Build Status](https://travis-ci.org/bertmelis/VitoWifi.svg?branch=master)](https://travis-ci.org/bertmelis/VitoWifi)

Arduino Library for ESP8266 to communicate with Viessmann boilers using a (DIY) serial optolink.

Based on the fantastic work on http://openv.wikispaces.com. Check out this site for a simple hardware implementations

**WARNING: This code compiles and works**
**However, this is only a testing release**

**This library is asynchronous**
This means that the use of blocking code is not supported. On the upside, this library itself is non blocking.
Since we're talking serial @4800 baud, we would be wasting precious processing time while reading and writing to the optolink. Furthermore this library is useable in other async frameworks such as the marvellous <a href="https://github.com/marvinroger/homie-esp8266">Homie for ESP8266</a> framework.


## Prerequisites
- Platformio or Arduino for ESP8266
- a (DIY) serial optocoupler to connect your ESP8266 to your Viessmann boiler
- As this is an implementation of the P300 (aka VS2) protocol, your boiler must be compatible with this protocol.

## Usage
See [usage](https://github.com/bertmelis/VitoWifi/blob/master/USAGE.md) for more details or explore the examples.
A working example using Homie and my [WifiPrinter](https://github.com/bertmelis/WifiPrinter) can be found on my [Github page](https://github.com/bertmelis/homie-boiler).

## TO DO
- improve reliability
- add transformation functions for returned values
- ~~communication is really slow. After initializing, it is quite fast however.
Maybe I could reset the connection after every TX/RX.~~ Done!


## HISTORY
I was using the original openv/vcontrold code from wikispaces on a Raspberry Pi V1. However, this setup was a bit overkill: such a powerful device doing only serial communication at 4800 baud. So I wanted to optimise this situation.
An ESP8266 has built-in serial/UART communication and is obviously connectable via Wifi. The small form factor and power consumption were bonus points. Ever since it has been made compatible with the Arduino environment (or other way around), it is my Nr 1 choice for IoT applications.


## Credits go to
- Hex print: 2011, robtillaart @ Arduino.cc forum
- Logger: MIT 2015, marvinroger @ Github
- Serial Protocol @ openv.wikispaces.com
- and many others for code and inspiration
