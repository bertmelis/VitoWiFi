# VitoWiFi

[![Build Status](https://travis-ci.org/bertmelis/VitoWiFi.svg?branch=master)](https://travis-ci.org/bertmelis/VitoWiFi)

Arduino Library for ESP8266 to communicate with Viessmann boilers using a (DIY) serial optolink.

Based on the fantastic work on [openv](https://github.com/openv/openv/wiki). Check out this wiki for a simple hardware implementations

**This library is non-blocking**
This also means that the use of blocking code is not supported.
Since we're talking serial @4800 baud, we would be wasting precious processing time while reading and writing to the optolink. Furthermore this library is useable in async or non blocking frameworks such as the marvellous [Homie for esp8266](https://github.com/marvinroger/homie-esp8266) framework or [Blynk](https://www.blynk.cc).

## Table of contents

- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Steps to get going](#steps-to-get-going)
  - [define the protocol](#define-the-protocol)
  - [Define your datapoints](#define-your-datapoints)
  - [Setup VitoWiFi](#setup-vitowifi)
    - [In void setup()](#in-void-setup)
    - [In void loop()](#in-void-loop)
  - [Read/write datapoints](#readwrite-datapoints)
- [Datapoints](#datapoints)
  - [My datapoint type isn’t listed?](#my-datapoint-type-isn%E2%80%99t-listed)
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

## Steps to get going

### Define the protocol

```C++
VitoWiFi_setProtocol(protocol);  // set protocol to KW or P300
```

There’s no typo! This line calls a macro that initialises the class with the protocol parameter: VitoWiFi is a templated class where the template parameter is the protocol. The template design avoids a virtual function call on every interaction with the optolink interface.  
Also, don't use quotes, protocol is an enum.

### Define your datapoints

```C++
DPTemp myTempDP(const char name, const char group, uint16_t address)
```

There are a number of predefined datapoint types:

- DPTemp: 2 byte, factor 10, returning float
- DPTempS: 1 byte, no factor, returning uint8_t
- DPStat: 1 byte, no factor, returning bool
- DPCount: 4 byte, no factor, returning uint32_t
- DPCountS: 2 byte, no factor, returning uint16_t
- DPMode: 1 byte, node mode, return uint8_t (same as DPTempS)
- DPHours: 4 byte, factor 3600, returning float
- DPCoP: 1 byte, factor 10, returning float
- DPRaw (see [datapoints](#datapoints))

Each datapoint is defined by a name, a group and an address. The addresses notation is mostly in a 2-byte hex eg. 0x5525.

Read on about datapoints in the separate chapter about [datapoints](#datapoints).

### Setup VitoWiFi

#### In void setup()

```C++
VitoWiFi.setup(&Serial);
```

Assign the serial interface connected to the optolink. Pass by reference. For ESP32 you also have to give the two pin numbers: setup(&Serial, 21, 22).
The serial interface will be set to 4800 baud, 8 bits, even parity and 2 stop bits by vitoWiFi.

```C++
VitoWiFi.setGlobalCallback(Callback);
```

Set the function to execute when data is received when there is no callback defined for the currently processed datapoint.
The callback has the following signature: `void function(IDatapoint*, DPValue)`

Set additional properties of the individual datapoints:

- `IDatapoint.setWriteable(bool)` to make to datapoint R/W instead or RO by default
- `IDatapoint.setCallback(Callback)` to assign a custom, per datapoint, callback handler. The signature is the same.

The modifiers can be chained, they return a reference to the datapoint.

Optionally:

- `VitoWiFi.setLogger(Printer*)` so vitoWiFi can print messages for debugging purposes.
- `VitoWiFi.enableLogger()` or `.disableLogger()` enables or disables logging

#### In void loop()

Call `VitoWiFi.loop();` in the Arduino `void loop()`.
Avoid blocking code. Minimize lengthy calculations etc. VitoWiFi will probably still work (the amount of data coming from the optolink is limited) but vitoWiFi works by polling so try to poll as much as possible.

### Read/write datapoints

```C++
VitoWiFi.readDatapoint(IDatapoint&);
```

You pass the datapoints you created in step 1. The datapoint is passed by reference.  
Do not call this repeatedly but use a timed call. On every call, the action is placed in a queue. This queue is limited in size.  
Mind that it takes some time (although less than 10ms) to process an action so calling too often will fill the queue.

```C++
VitoWiFi.writeDatapoint(IDatapoint&, DPValue);
```

The same as readDatapoint but with an extra DPValue parameter holding the value to be written. Make sure to initialize the value correctly. More on this in the chapter [DPValue](#dpvalue).

Alternative ways to read datapoints is by calling

-	`VitoWiFi.readAll();`
-	`VitoWiFi.readGroup(const char*);`

You cannot write to multiple datapoints in one command.

## Datapoints

Some insights...  
The datapoints architecture is as follows: there is an "IDatapoint" class which is the superclass from which all datapoint types inherit. The is done to have a base type (pointer) for universal storage of the datapoints in a std::vector.  
A templated "Datapoint" class inherits from "IDatapoint". "Datapoint" contains a class member, which is the template parameter, that handles the encoding and decoding.  
For easier coding, the builtin specialized classed have a typedef.

An overview of the builtin datapoint types:

| Type     | Underlying type         | Length | Conversion factor | Value type |
|----------|-------------------------|--------|-------------------|------------|
| DPTemp   | Datapoint<conv2_10_F>   | 2      | / 10              | float      |
| DPTempS  | Datapoint<conv1_1_US>   | 1      | 1                 | uint8_t    |
| DPStat   | Datapoint<conv1_1_B>    | 1      | 1                 | uint8_t    |
| DPCount  | Datapoint<conv4_1_UL>   | 4      | 1                 | uint32_t   |
| DPCountS | Datapoint<conv2_1_UL>   | 2      | 1                 | uint16_t   |
| DPMode   | Datapoint<conv1_1_US>   | 1      | 1                 | uint8_t    |
| DPHours  | Datapoint<conv4_3600_F> | 4      | / 3600            | float      |
| DPCoP    | Datapoint<conv1_10_F>   | 1      | / 10              | float      |
| DPRaw    | Datapoint<convRaw>      | -(1)   | -                 | uint8_t*   |

(1) the length of DPRaw has to be explicitly set using `setLength()`

### My datapoint type isn’t listed?

Then you can build it yourself! (and make a PR afterwards) But how do I do this?

Below is an example that creates a type called "DPSeconds" for a 4 bytes datapoint that holds a seconds counter. The datapoint converts this to a float with the number of minutes. You can place this directly in your code or place it in a seperate file that you include. If you use a seperate file, make sure you include "Datapoint.hpp" in that one.

```C++
// first create a converter class, and inherit from DPType
class conv4_60_F : public DPType {
 public:
  void encode(uint8_t* out, DPValue in) {  // encode can be left empty when the DP is only used for reading.
  int32_t tmp = floor((in.getFloat() * 60) + 0.5);
  out[3] = tmp >> 24;
  out[2] = tmp >> 16;
  out[1] = tmp >> 8;
  out[0] = tmp & 0xFF;
  }
  DPValue decode(const uint8_t* in) {
    int32_t tmp = in[3] << 24 | in[2] << 16 | in[1] << 8 | in[0];  // keep endianess in mind! input is LSB first
    DPValue out(tmp / 60.0f);
    return out;
  }
  const size_t getLength() const {
    return 4;
  }
};
// the typedef is optional. If you should consider making a PR,
// please do make a typedef for consistency.
typedef Datapoint<conv4_1_UL> DPMinutes;

// now you can use the newly created type:
DPMinutes myDatapoint("name", "group", 0x1234);
```

The example above is also in the [examples](https://github.com/bertmelis/VitoWiFi/tree/master/examples) folder.

## DPValue

DPValue is a structure that is used as a universal data holder. It can hold a bool, a unsigned integers (8, 16 and 32 bit) or a float. It cannot convert between types although a `getString()` method is available to print our it’s value.
An example:

```C++
uint16_t myInt = 234;
DPValue value(234);  // value holds a uint16_t with value 234
value1 = value.getFloat();  // value1 = 0 as value didn’t contain a float
value1 = value.getU16()  // value1 = 234
char str[4];
value.getString(str, sizeof(str));  // str contains “234” (+ extra null-termination)
```

The possible types are:

| type     | getter             | prints            |
|----------|--------------------|-------------------|
| uint8_t  | `getU8()`          | %u                |
| uint16_t | `getU16()`         | %u                |
| uint32_t | `getU32()`         | %u                |
| bool     | `getBool()`        | "true" or "false" |
| float    | `getFloat()`       | %.1f              |
| raw      | `getRaw(uint8_t*)` | %02x              |

## Optolink

The optolink is the core of this library. It is rather unpolished and requires some insights to use on it’s own. Please refer to the example for more details.
There are 2 versions: one for the KW protocol and one for the P300 protocol. Both work as a finite state machine with sequential connection management, waiting state, polling for a response and returning to idle.

You should first try if the P300 protocol works and if it doesn’t, try KW. If that also doesn’t work, you have a hardware problem or an unsupported boiler.
When making bug reports, make sure to include hardware details.

## End note

### History

I was using the original openv/vcontrold code from wikispaces on a Raspberry Pi V1. However, this setup was a bit overkill: such a powerful device doing only serial communication at 4800 baud. So I wanted to optimise this situation.
An ESP8266 has built-in serial/UART communication and is obviously connectable via Wifi. The small form factor and power consumption were bonus points. Ever since it has been made compatible with the Arduino environment (or other way around), it is my Nr 1 choice for IoT applications.

### License

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

### Credits go to

* Hex print: 2011, robtillaart @ Arduino.cc forum
* Logger/Blinker: MIT 2015, marvinroger @ Github
* Serial Protocol @ openv.wikispaces.com
* [tolw](https://github.com/tolw) for implementing the writing
* [Empor-co](https://github.com/Empor-co) for testing the KW-protocol
* and many others for code and inspiration
