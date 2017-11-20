# Advanced

This is an overview of all (useful) available methods to configure and use the VitoWifi library.
The purpose and the use is explained on this page but you can find most of them also in the examples.

## VitoWifi methods

`VitoWifi_setProtocol(protocol)`
* required
* sets the protocol
* `protocol` can be `P300` or `KW`
  
  
`void setup(HardwareSerial* serial)`
* required, only for ESP8266
* defines the UART-interface to be used by the optolink


`setup(HardwareSerial* serial, int8_t rxPin, int8_t txPin)`
* required, only for ESP32
* defines the UART-interface to be used by the optolink and sets the pins on which it is connected


`void loop()`
* required
* keeps VitoWifi running


`void setGlobalCallback(GlobalCallbackFunction globalCallback)`
* optional, as long as every every datapoint has it's own specific callback
* function with format `void (*GlobalCallbackFunction)(const char*, const char*, const char*)` to be called when new values are available.
* calling the method multiple times overwrites the previous global callback
* to be called after the definitiion of at least 1 datapoint


`Datapoint& addDatapoint(const char* name, const char* group, const uint16_t address, const DPType type)`
* required, at least 1
* returns a reference to the newly added datapoint
* adds a datapoint with a name (must be unique!), a group, an address and a type
* the type can be:
  * `TEMP` (2-byte temperature)
  * `STAT` (1-bit status (on/off))
  * `COUNT` (4-byte counter value)
  * `COUNTS` (2-byte counter value)
  * `MODE` (1-byte mode)
  
  
`Datapoint& addDatapoint(const char* name, const char* group, const uint16_t address, const DPType type, bool isWriteable)`
* see previous, with an extra parameter `isWriteable`  which defines a writeable dataponit when set to `true`


`void readAll()`
* optional
* reads all defined datapoints (also writeable ones). After reading the specific or global callback is called per datapoint.


`void readGroup(const char* group)`
* optional
* reads all defined datapoints of a specific group.


`void readDatapoint(const char* name)`
* optional
* reads a specific datapoint. In the event you didn't made every datapoint's name unique, only the first found will be read.


`void writeDatapoint(const char* name, TArg arg)`
* optional
* writes the specified value to the specific datapoint. Any type of argument can be used, as long as it is compatible with the datapoint to be written to (eg. don't write a float value to a MODE).


`void setLogger(Print* printer)`
* optional
* set the *printer* to which debug messages have to be written to.


`void enableLogger()`
* optional
* enables the `Logger`.


`void disableLogger()`
* optional
* disables the `Logger`


## Datapoint methods

`Datapoint& setCallback(callbackfunction)`
* optional, if globalCallback has been set
* returns a reference to the datapoint
* sets a specific callback for this datapoint. When set, the global callback will not be called.
* callbackfunction has to comply with the definition based on the datapoint type:
  * `void (*TempCallbackFunction)(const char*, const char*, float)` for TEMP datapoints
  * `void (*StatCallbackFunction)(const char*, const char*, bool)` for STAT datapoints
  * `void (*CountCallbackFunction)(const char*, const char*, uint32_t)` for COUNT datapoints
  * `void (*CountSCallbackFunction)(const char*, const char*, uint16_t)` for COUNTS datapoints
  * `void (*ModeCallbackFunction)(const char*, const char*, uint8_t)` for MODE datapoints

  
`void setWriteable()`
* optional
* returns a reference to the datapoint
* set the datapoint to writeable.

