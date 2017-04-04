# VitoWifi
[![Build Status](https://travis-ci.org/bertmelis/VitoWifi.svg?branch=master)](https://travis-ci.org/bertmelis/VitoWifi)

Library to communicate with Viessmann boilers using a (DIY) optolink for ESP8266 (Arduino environment)

Based on the fantastic work on http://openv.wikispaces.com. Check out this site for a simple hardware implementations

**This library is asynchronous**
This means that the use of blocking code is not supported. On the upside, this library itself is non blocking. Since we're talking serial @4800, we would be wasting precious processing time while reading and writing to the optolink. Furthermore this library is useable in other async frameworks such as the marvellous <a href="https://github.com/marvinroger/homie-esp8266">Homie for ESP8266</a> framework.

**WARNING: This code compiles but is largely untested!**


## Dependencies
- Platformio or Arduino for ESP8266
- a (DIY) optocoupler to connect your ESP8266 to your Viessmann boiler
- As this is an implementation of the P300 (aka VS2) protocol, your boiler must be compatible with this protocol.


## TO DO
- test and debug
- add transformation functions for returned values


## HISTORY
I was using the original openv/vcontrold code from wikispaces on a Raspberry Pi V1. However, this setup was a bit overkill: such a powerful device doing only serial communication at 4800 baud. So I wanted to optimise this situation. An ESP8266 has built-in serial/UART communication and is obviously connectable via Wifi. The small form factor and power consumption were bonus points. And ever since it has been made compatible with the Arduino environment (or other way around), it is my Nr 1 choice for IoT applications.


## Credits go to
- robtillaart @ Arduino.cc forum
- Marvin ROGER for the logger code
and many others for code and inspiration
