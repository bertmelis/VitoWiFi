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
 * @file OptolinkDP.h
 * @brief OptolinkDP Class definition
 *
 * This file contains the class definition OptolinkDP, an object
 * used as a queue item in Optolink.
 */

#pragma once

#include <stdint.h>
#include <cstring>  // memcpy

/**
 * @brief Class holding datapoint values. The Optolink queue stores this
 * struct.
 */
class OptolinkDP {
 public:
   /**
   * @brief Construct a new OptolinkDP object.
   * 
   * @param address Address of the datapoint (eg. 0x1234)
   * @param length Length in bytes of the datapoint. This is also the length
   *               of the value when writing.
   * @param write Bool indicating the datapoint is readonly (false) or
   *              read/write (true)
   * @param value Pointer to data to write (set to nullptr when reading). This 
   *              data will be copied so it is allowed to go out of scope
   *              after passing the this object.
   * @param arg Argument (const) to use for the callback (if not used, set to nullptr)
   */
  OptolinkDP(uint16_t address, uint8_t length, bool write, uint8_t* value, void* arg);
  /**
   * @brief Construct a new OptolinkDP object.
   * 
   * All members will be set to zero or nullptr.
   */
  OptolinkDP();

  /**
   * @brief Copy constructor for the OptolinkDP class.
   * 
   * @param obj Object to be copied.
   */
  OptolinkDP(const OptolinkDP& obj);

  /**
   * @brief Destroy the OptolinkDP object
   * 
   */
  ~OptolinkDP();
  uint16_t address;  //!< Address of the datapoint, 2 bytes
  uint8_t length;    //!< Length of the dataponit, 1 byte
  bool write;        //!< Mark the dataponit as writeable (true) or not (false)
  uint8_t* data;     //!< Pointer to the (raw) data to be written. Memory is allocated by this class
  void* arg;         //!< Argument to be used on the callback function
};
