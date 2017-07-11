# Changelog

## 2017-07-11
- Implemented CountLong Datapoint
- Helper inclusion deleted
- Moved callback typedefs to Datapoint.h

## 2017-06-16
- Implemented SoftwareSerial support (use version with 8E2-support)
- improvements on error handling
- added example for raw optolink usage

## 2017-06-15
- Serial can only be passed by reference (to avoid confusion)
- reworked Optolink error handling (not finished!)
- added isBusy-check to Optolink
- added "action" to Optolink FSM
- removed debugging printer from Datapoint class
- removed wifi from samples (to check ESP32 compatability)
- add direct use of Optolink example
- add esp32 compatability

## 2017-06-07
- BREAKING: library interface is using callbacks
- BREAKING: datapoints are added to VitoWifi and stored inside the lib.

## 2017-04-25
- FSM reworked (ACK-states added)
- Temp RX-buffers removed
- Returntypes in progress: TEMP and E_S working

## 2017-04-23
- reworked state machine
- renamed returntype in datapoint struct
- numerous bugfixes
TODO: rework returning of read values

## 2017-04-10 - Working version
- tested and working, but not reliably

## 2017-03-16 - Initial version
- untested code
- no transformation functions
