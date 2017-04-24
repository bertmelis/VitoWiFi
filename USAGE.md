### VitoWiFi Manual

## Hardware implementation

A detailed description of my setup is work in progress. Meanwhile you can check out http://openv.wikispaces.com/Bauanleitung+ESP8266 as this is a simple but effective implementation. I'm using this and I haven't run into any problems which I can trace back to hardware limitations.

Simply put: I made the described circuit on a perfboard and connected this to a Wemos D1 mini (but any ESP board should do).

Some remarks:
- when programming the ESP8266 via serial (or USB), do not connect the optocoupler.
- SoftwareSerial is not (yet?) compatible so other pins for UART are not supported. SoftWareSerial does not support 8E2 mode so is not capable of communication with the Vitotronic interface.

## Library usage

Below is a breakdown of the simple-read example. Keep in mind that this example is ginving no output at all because the serial interface is connected to the Vitotronic. You could monitor the interface with a terminal program on your computer but it wont give you human readable information. For a more real world example, you can look into this: https://github.com/bertmelis/homie-boiler.

For installing this library, I refer to the Platformio ([global](http://docs.platformio.org/en/latest/userguide/lib/cmd_install.html) or [project-specific](http://docs.platformio.org/en/latest/projectconf.html)) or Arduino manuals ([guide](https://www.arduino.cc/en/Guide/Libraries)).

### simple-read.ino
#### General section
```C++
#include <VitoWifi.h>

VitoWifi myVitoWifi;
```
Start with including the header file and create a VitoWifi instance.

```C++
//char[15+1] - READ/WRITE - uint16_t - uint8_t - type
const Datapoint DP = { "OutsideTemp", READ, 0x5525, 16, TEMP};
char value[8] = {0};
```
The library gives you access to a new `Datapoint` variable. This variable holds the properties of the Vitotronic datapoint:
- `name`: a name of the datapoint (max 15 chars long)
- `rw`: hether it is a DP to `READ`or to `WRITE`
- `address`: the memory address of the DP
- `length`: the length of the DP in bytes
- `type`: the type of the value returned
The type can be one of the following:
```
TEMP,   //temperatures
H,      //hours
C,      //counter
E_OM1,  //enum operating mode (stanby/only dhw/heating+dhw/red/std)
E_OM2,  //enum operating mode heating (standby/reduced/norm1/norm2)
E_S,    //enum status
E_SF,   //enum status flame (B2)
E_M,    //enum mode (B1)
E_PM,   //enum party mode (B7)
E_FW,   //enum frost warning (B7)
TXT,    //text
E_SV,   //switch valve (undef/heating/middle/dhw)
NONE    //no transformation
```

The variable is declared `const` as there are no properties of the DP to be changed during execution.
The variable `value` will be used to store the returned values. 8 Characters is the minimum size.

Do a [Google](http://www.google.com)-search for the appropriate datapoints for your Viessmann device or look into the [openv](http://openv.wikispaces.com)-website.

#### Setup()
```C++
myVitoWifi.begin(&Serial);
```
In the Arduino setup()-function only 1 things has to be done minimally:
Tell VitoWifi to use the Serial interface. Pass the `Serial` object by reference by using the `&`. (Should you forget, the library takes care.)
There is no need to call `Serial.begin()`yourself, the library does this for you. Keep this in mind and don't call `Serial.begin()` further down your code.

#### Loop()
In the Arduino loop()-function:
```C++
myVitoWifi.loop();
```
Keep this at the begin or end of your loop to keep VitoWifi running. In this loop, the library does all it's work hence it should be called frequently (eg. don't use any blocking code or lengthy routines).

The principle of communicating is as follows:

1. **evaluate the DP:**
```C++
myVitoWifi.sendDP(DP);
```
the `sendDP(Datapoint& DP)` method is used to hand over your datapoint to VitoWifi. VitoWifi then executes the `READ` or `WRITE` command on the specified memory addrss.
2. **wait for response**
```C++
myVitoWifi.available();
```
Check whether VitoWifi is ready to return the read `READ` value or, in case of a `WRITE`, return the written value.
3. **reading the value**
 ```C++
myVitoWifi.read(value, sizeof(value));
```
`read(char value[], length)` puts the written value into you char array as declared above. The length is 8 characters minimally.
You could also use `read()` -without arguments- which returnes a `float` value.

##### A remark:
Create your sketch in such a way that this routine is executed in this particular order. Not adhering will generate an error or gives undefined results.


### Advanced: multiple-read.ino
You can also create a array of Datapoints as is the multiple-read example. This example uses `PROGMEM` to store the datapoints to save precious RAM on the ESP8266.
The library includes handy template functions to read the DPs and to get the number of DPs declared.

Create your sketch in such a way that the loop never gets blocked. Intead of creating a `while` loop, use flag variables and `if` structures. Explore [multiple-read](https://github.com/bertmelis/VitoWifi/blob/master/examples/multiple-read/multiple-read.ino) for an example.
