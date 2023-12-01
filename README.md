# VitoWiFi

Library for ESP8266 to communicate with Viessmann systems using a (DIY) serial optolink.

Based on the fantastic work on [openv](https://github.com/openv/openv/wiki).

[![PlatformIO Registry](https://badges.registry.platformio.org/packages/bertmelis/library/VitoWiFi.svg)](https://registry.platformio.org/libraries/bertmelis/VitoWiFi)
[![Build with Platformio](https://github.com/bertmelis/VitoWiFi/actions/workflows/build_platformio.yml/badge.svg)](https://github.com/bertmelis/VitoWiFi/actions/workflows/build_platformio.yml)
[![Build with Arduino IDE](https://github.com/bertmelis/VitoWiFi/actions/workflows/build_arduino_ide.yml/badge.svg)](https://github.com/bertmelis/VitoWiFi/actions/workflows/build_arduino_ide.yml)

## Features

- VS1 (KW) and VS2 (P300) support. Older systems using the GWG protocol are not supported
- Non-blocking API calls
- For the Arduino framework and POSIX systems (Linux, tested on a Raspberry Pi 1B)
- Possible to use `SoftwareSerial`

## Contents

- [Installation](#installation)
- [Hardware](#hardware)
- [Usage](#usage)
- [Datapoints](#datapoints)
- [API reference](#api-reference)
- [Bugs and feature requests](#bugs-and-feature-requests)
- [License](#license)

## Installation

* For Arduino IDE: see [the Arduino Guide](https://www.arduino.cc/en/Guide/Libraries#toc4)
* For Platformio: see the [Platfomio registry page for VitoWifi](https://registry.platformio.org/libraries/bertmelis/VitoWiFi)

## Hardware

The optolink hardware for ESP8266 and ESP32 is simple. Using the circuit below you can build your own optolink.
Please also check the [openv wiki, in German](https://github.com/openv/openv/wiki/Die-Optolink-Schnittstelle) for more implementations.

```
                3.3V
                 O
                 |
           +-----+-----+
           |           |
          ---         ---
          | |         | |
          | | 180Ohm  | | 10kOhm
          | |         | |
          ---         ---
           |           |
          ---          |
SFH487-2  \ / ->       |
           V  ->       |
          ---          |
           |           |
TX O-------+           |
RX O-------------------+
                       |
                      |/ c
                   -> |     SFH309FA
                   -> |> e
                      |
                    -----
                     ---
                      -
```

## Usage

The canonical way to use this library is simple and straightforward. A few steps are involved:

1. define your VitoWiFi object and specify the protocol and interface
2. define all needed datapoints
3. create callback for when data or errors are returned (std::function supported)
4. in `void setup()`
    - attach the callbacks
    - start VitoWiFi
5. in `void loop()`:
    - call `loop()` regularly. It keeps VitoWiFi running.

A simple program for ESP32 to test and query your devicetype looks like this:

```cpp
#include <Arduino.h>
#include <VitoWiFi.h>

VitoWiFi::VitoWiFi<VitoWiFi::VS2> myHeater(&Serial1);
VitoWiFi::Datapoint deviceId("device id", 0x00F8, 2, VitoWiFi::noconv);

void onResponse(const VitoWiFi::PacketVS2& response, const VitoWiFi::Datapoint& request) {
  Serial.print("Raw data received:");
  const uint8_t* data = response.data();
  for (uint8_t i = 0; i < response.dataLength(); ++i) {
    Serial.printf(" %02x", data[i]);
  }
  Serial.print("\n");
}

void onError(VitoWiFi::OptolinkResult error, const VitoWiFi::Datapoint& request) {
  Serial.printf("Datapoint \"%s\" error: ", request.name());
  if (error == VitoWiFi::OptolinkResult::TIMEOUT) {
    Serial.print("timeout\n");
  } else if (error == VitoWiFi::OptolinkResult::LENGTH) {
    Serial.print("length\n");
  } else if (error == VitoWiFi::OptolinkResult::NACK) {
    Serial.print("nack\n");
  } else if (error == VitoWiFi::OptolinkResult::CRC) {
    Serial.print("crc\n");
  } else if (error == VitoWiFi::OptolinkResult::ERROR) {
    Serial.print("error\n");
  }
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.print("Setting up vitoWiFi\n");
  myHeater.onResponse(onResponse);
  myHeater.onError(onError);
  myHeater.begin();
  Serial.print("Setup finished\n");
}

void loop() {
  static uint32_t lastReadTime = 0;
  if (millis() - lastReadTime > 30000) {
    lastReadTime = millis();
    if (myHeater.read(deviceId)) {
      Serial.printf("reading \"%s\"\n", deviceId.name());
    } else {
      Serial.printf("error reading \"%s\"\n", deviceId.name());
    }
  }
  myHeater.loop();
}

```

Most users will have a collection of datapoints thay want to read. A possible technique to query a large number of datapoints is by simply iterating over them:

```cpp
// create a collection (array) of datapoints:
VitoWiFi::Datapoint datapoints[] = {
  VitoWiFi::Datapoint("outside temp", 0x5525, 2, VitoWiFi::div10),
  VitoWiFi::Datapoint("boiler temp", 0x0810, 2, VitoWiFi::div10),
  VitoWiFi::Datapoint("pump status", 0x2906, 1, VitoWiFi::noconv)
};
int numberDatapoints = 3;
int currentIndex = -1;

// to start reading, set currentIndex to 0
if (currentIndex > 0) {
  // reading will return `true` when successful.
  // as long as VitoWiFi is busy it will return `false`
  if (myVitoWiFi.read(datapoints[currentIndex])) {
    ++currentIndex;
    if (currentIndex == numberDatapoints) currentIndex = -1;
  }
}
```

### More examples

you can find more examples in the `examples` directory in this repo.

## Datapoints

When defining your datapoints, you need to specify the name, address, length and converion type. Datapoints in C++ looks like this:

```cpp
VitoWiFi::Datapoint datapoint1("outside temp", 0x5525, 2, VitoWiFi::div10);
VitoWiFi::Datapoint datapoint2("boiler temp", 0x0810, 2, VitoWiFi::div10);
VitoWiFi::Datapoint datapoint3("pump status", 0x2906, 1, VitoWiFi::noconv);
```

It is not possible for me to give you a list of available datapoints for your device. Please consult the [openv wiki](https://github.com/openv/openv/wiki/Adressen) or the [InsideViessmannVitosoft repo](https://github.com/sarnau/InsideViessmannVitosoft).

While name, address and length are self-explanatory, conversion type is a bit more complicated.

### Conversion types

Data is stored in binary and often needs a conversion function to transform into a more readable type. This is specified by the conversion type, the last argument in the datapoint definition.

Since C++ is a strongly typed programming language so using the right type is important (read: mandatory). Each conversion type corresponds with a certain type. Reading or writing has to be done using this specific type and failure to do so will not work or will lead to undefined results.

In the table below you can find how to define your datapoints:

|name|size|converter|return type|remarks|
|---|---|---|---|---|
|Temperature|2|div10|float||
|Temperature short|1|noconv|uint8_t|equivalent to Mode|
|Power|1|div2|float|also used for temperature in GWG|
|Status|1|noconv|bool|this is the same as 'Temperature short' and 'Mode'. The  `uint8_t` value will be implicitely converted to bool.|
|Hours|4|div3600|float|this is in fact a `Count` datapoint (seconds) converted to hours.|
|Count|4|noconv|uint32_t||
|Count short|2|noconv|uint16_t||
|Mode|1|noconv|uint8_t|possibly castable to ENUM|
|CoP|1|div10|float|Also used for heating curve slope|

## Bugs and feature requests

Please use Github's facilities to get in touch. While the issue template is not mandatory to use, please use it at least as a starting point to supply the needed info for bughunting.

## API reference

Below is an overview of all commonly used methods. For extra functions you can consult the source code.

### Datapoint

##### `Datapoint(const char* name, uint16_t address, uint8_t length, const Converter& converter)`

Constructor for datapoints.

##### `const char* name() const`
##### `uint16_t address() const`
##### `uint8_t length() const`

Self-explanatory

##### `const Converter& converter() const`

Returns the associated converter class. Can be used to select code flow in the callbacks.

##### `VariantValue decode(const PacketVS2& packet) const`

Decodes the data in `packet` using the converter class attached.
Returns `VariantValue` which is implicitely castable to the correct datatype. Consult the table above.

##### `VariantValue decode(const uint8_t* data, uint8_t length) const`

Decodes the data in the supplied `data`-buffer using the converter class attached.
Returns `VariantValue` which is implicitely castable to the correct datatype. Consult the table above.

##### `void encode(uint8_t* buf, uint8_t len, const VariantValue& value) const`

Encodes `value` into the supplied `buf` with maximum size `len`. The size should obviously be at least the length of the datapoint.

`VariantValue` is a type to implicitely convert datatypes for use in VitoWiFi. Make sure to use the type that matches your Converter type.

### `PacketVS2`

Only used in VS2. This type is used in the onResponse callback and contains the returned data.
Most users will only use the following two methods and only if they want to access the raw data. Otherwise, the data can be decoded using the corresponding `Datapoint`.

##### `uint8_t dataLength() const`

Returns the number of bytes in the payload.

##### `const uint8_t* data() const`

Returns a pointer to the payload.

### VitoWiFi

##### `VitoWiFi<PROTOCOL_VERSION>(IFACE* interface)`

Constructor of the VitoWiFi class. `PROTOCOL_VERSION` can be  `GWG`, `VS1` or `VS2`. If your Viessmann device is somewhat modern, you should use `VS2`.
`interface` can be any of the `HardwareSerial` interfaces (`Serial`, `Serial1`...), `SoftwareSerial` or if you are on Linux, pass the c-string depicting your device (for example `"/dev/ttyUSB0"`).

##### `void onResponse(typename PROTOCOLVERSION::OnResponseCallback callback)`

Attach an onResponse callback. You can only attack one and will overwrite the previously attached callback.
The callback has the following signature:
`VS1`: `void (const uint8_t*, uint8_t, const VitoWiFi::Datapoint&)`
`VS2`: `void (const VitoWiFi::PacketVS2&, const VitoWiFi::Datapoint&)`

##### `void onError(typename PROTOCOLVERSION::OnErrorCallback callback)`

Attach an onError callback. You can only attack one and will overwrite the previously attached callback.
The callback has the following signature:
`void (VitoWiFi::OptolinkResult, const VitoWiFi::Datapoint&)`

##### `bool begin()`

Start the optolink serial interface. Returns bool on success.

##### `void end()`

Stop the optolink serial interface

##### `void loop()`

Worker function, must be called regularly. This is the method that polls the serial interface. Callbacks are dispatched from this method so they will run in the same thread/task.

##### `bool read(Datapoint datapoint)`

Read `datapoint`. Returns `true` on success.

##### `bool write(Datapoint datapoint, T value)`

Write `value` with type `T` to `datapoint`. Make sure to use the correct type. consult the table with types in the "Datapoints" section.

##### `write(Datapoint datapoint, const uint8_t* data, uint8_t length)`

Write the raw `data` with `length` to `datapoint`. Returns `true` on success. `length` has to match the length of the datapoint.

### Enums

##### `VitoWiFi::OptolinkResult`

Used in the onError callback. Possible returned values are:
- TIMEOUT
- LENGTH
- NACK
- CRC
- ERROR

### Compile time configuration

##### `VW_START_PAYLOAD_LENGTH`

This macro sets the initial payload (data) length VitoWiFi allocates for incoming packets. If you know beforehand the maximum data length you are going to request, you can set this to that value to prevent reallocation of dynamic memory. The default is 10 bytes.

## Bugs and feature requests

Please use Github's facilities, issues and discussions, to get in touch.
When creating a bug report, please use the provided template. In any case, better to include too much info than too little.

## License

Copyright (c) 2017, 2023 Bert Melis

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

* Hex print: 2011, robtillaart @ Arduino.cc forum (not used in v3 and above)
* Logger/Blinker: MIT 2015, marvinroger @ Github (not used in v3 and above)
* Serial Protocol @ ~~http://openv.wikispaces.com~~https://github.com/openv/openv/wiki
* [@tolw](https://github.com/tolw) for implementing the writing
* [@Empor-co](https://github.com/Empor-co) for testing the KW-protocol
* and many others for code and inspiration
