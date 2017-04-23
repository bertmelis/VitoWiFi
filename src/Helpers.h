#pragma once
#include <Arduino.h>

enum DPType: bool {
  READ,
  WRITE
};


enum ValueType: uint8_t {
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
};


//DataPoint = FriendlyName - R/W - VS-Address - Length
struct Datapoint {
  char name[15+1];
  DPType rw;
  uint16_t address; //0x5525
  uint8_t length; //length of read or written value in bytes
  ValueType type;
};


//read from PROGMEM
template <typename T> void PROGMEM_readAnything (const T * sce, T& dest){
  memcpy_P (&dest, sce, sizeof (T));
}
template <typename T> T PROGMEM_getAnything (const T * sce){
  static T temp;
  memcpy_P (&temp, sce, sizeof (T));
  return temp;
}


// number of items in an array
template< typename T, size_t N > size_t ArraySize (T (&) [N]){ return N; }


/*

to initiate connection:
1. in KW mode Vitotronic sends periodically 0x05 (ENQ)
2. You have to answer this immediately by 0x16 0x00 0x00 (SYN)
3. Vitotronic answers back  0x06 (ACK)
4. Vitotronic stops sending 0x05 periodically
5. reset to KW by sending 0x04 (EOT)

to communicate:
When data is read:
# 41: Message start byte
# 05: Length of user data (number of bytes between the telegram start byte (0x41) and checksum)
# 00: 00 = request, response = 01, 03 = Error
# 01: 01 = Read Data, Write Data = 02, 07 = Function Call
# XX XX: 2 byte address of the data or procedure
# XX: number of bytes to read (= expected in the response)
# XX: CRC=sum total of the values ​​from the 2 bytes (excluding 41)

If data is to be written:
# 41: Message start byte
# 06: Length of user data (number of bytes between the telegram start byte (0x41) and checksum)
# 00: 00 = request, response = 01, 03 = Error
# 02: 01 = Read Data, Write Data = 02, 07 = Function Call
# XXXX: 2 bytes address of the data or procedure
# XX: number of bytes to be written
# XX: content to be written
# XX: CRC=sum total of the values ​​from the 2 bytes (excluding 41) modulo 256
The answer from a write request does not contain the written value.
It doesn't contain any value at all.

*/
