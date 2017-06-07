# VitoWifi
[![Build Status](https://travis-ci.org/bertmelis/VitoWifi.svg?branch=master)](https://travis-ci.org/bertmelis/VitoWifi)

Arduino Library for ESP8266 to communicate with Viessmann boilers using a (DIY) serial optolink.

Based on the fantastic work on http://openv.wikispaces.com. Check out this site for a simple hardware implementations

**WARNING: This code compiles and works**
**However, this is only a testing release**
**Writing to Vitotronic is not tested**

**This library is non-blocking**
This also means that the use of blocking code is not supported.
Since we're talking serial @4800 baud, we would be wasting precious processing time while reading and writing to the optolink. Furthermore this library is useable in async frameworks such as the marvellous <a href="https://github.com/marvinroger/homie-esp8266">Homie for ESP8266</a> framework.


## Prerequisites
- Platformio or Arduino for ESP8266
- a (DIY) serial optocoupler to connect your ESP8266 to your Viessmann boiler
- As this is an implementation of the P300 (aka VS2) protocol, your boiler must be compatible with this protocol.

## Usage
~~See [usage](https://github.com/bertmelis/VitoWifi/blob/master/USAGE.md) for more details or explore the examples.~~  
A working example using Homie and my [WifiPrinter](https://github.com/bertmelis/WifiPrinter) can be found on my [Github page](https://github.com/bertmelis/homie-boiler).  
In the meantime: look into the examples to explore the functionality, documentation is on it's way!

## TO DO
- improve/implement error handling
- integrate missing datapoint types
- update USAGE.md


## HISTORY
I was using the original openv/vcontrold code from wikispaces on a Raspberry Pi V1. However, this setup was a bit overkill: such a powerful device doing only serial communication at 4800 baud. So I wanted to optimise this situation.
An ESP8266 has built-in serial/UART communication and is obviously connectable via Wifi. The small form factor and power consumption were bonus points. Ever since it has been made compatible with the Arduino environment (or other way around), it is my Nr 1 choice for IoT applications.


## Credits go to
- Hex print: 2011, robtillaart @ Arduino.cc forum
- Logger/Blinker: MIT 2015, marvinroger @ Github
- Serial Protocol @ openv.wikispaces.com
- and many others for code and inspiration
