/* VitoWiFi

Copyright 2019 Bert Melis

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

/**
 * @file Optolink.h
 * @brief Optolink API definitions
 *
 * This file contains all method definitions for the Optolink. 
 * The optolink object is the high level class and contains the interface 
 * and implements the queue system. Protocol details are implemented in the 
 * inherited classes.
 * 
 * The optolink hardware implementation could be as follows:
 * 
 *                          3.3V
 *                           O
 *                           |
 *                   +-------+-------+
 *                   |               |
 *                   \               \
 *              180R /               / 10kR
 *                   \               \
 *                   /               /
 *                   |               |
 *                  ---              |
 *         SFH487-2 \ / -->          |
 *          (880nm)  V  -->          |
 *                 -----             |
 *                   |               |
 *     TX -----------+               |
 *     RX ---------------------------+
 *                                   |
 *                                 |/
 *                             --> |   SFH309FA
 *                             --> |>
 *                                   |
 *                                  ---
 *                                  GND
 * 
 */

#pragma once

#if defined ARDUINO

#ifndef VITOWIFI_MAX_QUEUE_LENGTH
  /** @brief Maximum number of datapoints the Optolink queue can hold
   */
  #define VITOWIFI_MAX_QUEUE_LENGTH 20
#endif
#ifndef MAX_DP_LENGTH
  /** @brief Maximum size in bytes of a datapoint
   */
  #define MAX_DP_LENGTH 9
#endif

#include <HardwareSerial.h>
#include <string.h>  // for memcpy

#include "Helpers/SimpleQueue.h"
#include "OptolinkDP.h"

/**
 * @brief Errors the optolink can encounter
 */
enum OptolinkError : uint8_t {
  TIMEOUT,    ///< Timeout for ack or answer
  LENGTH,     ///< Received message length differs from expected length
  NACK,       ///< Message was nacked by Vitotronic
  CRC,        ///< Checksum failed (only for P300)
  VITO_ERROR  ///< General error
};

typedef void (*OnDataArgCallback)(uint8_t* data, uint8_t len, void* arg);
typedef void (*OnErrorArgCallback)(uint8_t error, void* arg);

/**
 * @brief Base class for the Optolink.
 * 
 * This class is a pure virtual class. Only the Optolink implemented in the 
 * different protocol classes are to be used. This class defines the public
 * API.
 */
class Optolink {
 public:
  /**
   * @brief Construct the Optolink object.
   * 
   * @param serial Hardwareserial object to be used. Pass by reference.
   */
  explicit Optolink(HardwareSerial* serial);
  virtual ~Optolink();

  /**
   * @brief Attach a callback for succesfull requests.
   * 
   * @param callback Function to be called when data is received.
   */
  void onData(void (*callback)(uint8_t* data, uint8_t len));

  /**
   * @brief Attach a callback with an argument for succesfull requests.
   * 
   * @param callback Function to be called when data is received.
   */
  void onData(OnDataArgCallback callback);

  /**
   * @brief Attach the callback for erroneous requests.
   * 
   * @param callback Function to be called when en error is encountered.
   */
  void onError(void (*callback)(uint8_t error));

  /**
   * @brief Attach the callback with an argument for erroneous requests.
   * 
   * @param callback Function to be called when en error is encountered.
   */
  void onError(OnErrorArgCallback callback);

  /**
   * @brief Read a datapoint with specified properties
   * 
   * Read (length) bytes from (address). On success, the data will be returned
   * by the onData handler; On error, the onError handler will be called.
   * 
   * @param address Address of the datapoint (eg. 0x1234).
   * @param length Length in bytes of the datapoint. This is also the length
   *        of the value when writing.
   * @param arg Argument to use for the callback. Defaults to nullptr.
   * @return true Request was queued succesfully.
   * @return false Request could not be added to the queue (queue full?).
   */
  bool read(uint16_t address, uint8_t length, void* arg = nullptr);

  /**
   * @brief Write to a datapoint with specified properties
   * 
   * Write (length) bytes to (address). On success, the written data is
   * returned by the onData handler. On failure, the onError will be called.
   * 
   * @param address Address of the datapoint (eg. 0x1234).
   * @param length Length in bytes of the datapoint. This is also the length
   *        of the value when writing.
   * @param data Pointer to data to write (set to nullptr when reading). This 
   *        data will be copied so it is allowed to go out of scope after
   *        passing the this object.
   * @param arg Argument to use for the callback. Defaults to nullptr.
   * @return true Request was queued succesfully.
   * @return false Request could not be added to the queue (queue full?).
   */
  bool write(uint16_t address, uint8_t length, uint8_t* data, void* arg = nullptr);

  /**
   * @brief Pure virtual method to start the Optolink (implemented in protocol 
   *        classes).
   */
  virtual void begin() = 0;

  /**
   * @brief Pure virtual method to keep the Optolink running (implemented in
   *        protocol classes). Call repeatedly eg. in the Arduino loop.
   */
  virtual void loop() = 0;


 protected:
  void _tryOnData(uint8_t* data, uint8_t len);
  void _tryOnError(uint8_t error);
  HardwareSerial* _serial;
  SimpleQueue<OptolinkDP> _queue;  // TODO(bertmelis): add semaphore to ESP32 version to guard access to queue
  OnDataArgCallback _onData;
  OnErrorArgCallback _onError;
};

#elif defined VITOWIFI_TEST

#else

#pragma message "no suitable platform"

#endif
