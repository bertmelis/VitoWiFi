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
 * @file VitoWiFi.h
 * @brief API definitions for VitoWiFi
 *
 * This library is made to use with the Arduino framework for 
 * esp8266 and esp32.
 */

#pragma once

#if defined ARDUINO_ARCH_ESP8266 || ARDUINO_ARCH_ESP32

#include <vector>

#include "OptolinkP300.h"

#include "Datapoints/DPTemp.h"
#include "Datapoints/DPStat.h"
#include "Datapoints/DPCountS.h"

/**
 * @brief Protocols implemented in VitoWiFi
 * 
 * Possible values are
 * - P300: the newer protocol with connection management and message integrity 
 *         checks (checksum). Units are backward compatible with KW.
 * - KW: older protocol, based on the early GWG but with support for 
 *       datapoints > 2 bytes.
 * 
 * When trying VitoWiFi you should test with P300. If your heater 
 * doesn't respond, switch to KW.
 * 
 */
enum VitoWiFiProtocol {
  P300,
  KW
};

/**
 * @brief VitoWiFi manages the datapoints and optolink to your Viessmann device.
 * 
 */
class VitoWiFi {
 public:
 /**
  * @brief Construct a new VitoWiFi object
  * 
  * @param protocol Protocol to be used by the optolink (`P300` or `KW`)
  * @param serial Hardwareserial port to be used by the optolink
  */
  VitoWiFi(VitoWiFiProtocol protocol, HardwareSerial* serial);
  /**
   * @brief Destroy the VitoWiFi object
   * 
   */
  ~VitoWiFi();

  /**
   * @brief Add a datapoint to your VitoWiFi object.
   * 
   * Adding a datapoint is not neccecary but `readAll` only reads the
   * datapoints that were registered by this method.
   * 
   * @param datapoint Pointer to the datapoint.
   */
  void addDatapoint(Datapoint* datapoint);

  /**
   * @brief Registers the callback to be used on data.
   * 
   * The onData callback is called when there is no dedicated callback registered
   * on the active datapoint
   * 
   * @param callback Function to call on data
   */
  void onData(std::function<void(const uint8_t* data, uint8_t length, Datapoint* dp)> callback);

  /**
   * @brief Registers the callback to be used on error.
   * 
   * The onError callback is used by VitoWiFi so it is shared by all 
   * the registered datapoints.
   * 
   * @param callback Function to call on error
   */
  void onError(std::function<void(uint8_t, Datapoint*)> callback);
  /**
   * @brief Start VitoWiFi.
   * 
   * This should be called after all setup for VitoWiFi is done.
   * By calling `begin()`, the Optolink will be started.
   * 
   */
  void begin();

  /**
   * @brief Keeps VitoWiFi running.
   * 
   * This method keeps the optolink running. You should call this often,
   * in the Arduino `loop()` for example.
   * 
   */
  void loop();

  /**
   * @brief Queues all registered dataponits for reading.
   * 
   * All registered datapoints are queued to be read. This includes the
   * datapoints that are marked as writeable. This method does not return
   * any failure code so you don't know whether the enqueuing was
   * successful.
   * 
   */
  void readAll();

  /**
   * @brief enqueue a datapoint for reading.
   * 
   * @param datapoint Datapoint to be read, passed by reference.
   * @return true Enqueueing was successful
   * @return false Enqueueing failed (eg. queue full)
   */
  bool read(Datapoint& datapoint);  // NOLINT todo: make it a const ref or pointer?

  /**
   * @brief Enqueue a datapoint for writing.
   * 
   * The onData callback will be launched on success.
   * 
   * @tparam D Type of datapoint (inherited from class `Datapoint`)
   * @tparam T Type of the value to be written
   * @param datapoint Datapoint to be read, passed by reference.
   * @param value Value to be written
   * @return true Enqueueing was successful
   * @return false Enqueueing failed (eg. queue full)
   */
  template<class D, typename T>
  bool write(D& datapoint, T value);  // NOLINT todo: make it a const ref or pointer?

 private:
  struct CbArg {
    CbArg(VitoWiFi* vw, Datapoint* d) :
      v(vw),
      dp(d) {}
    VitoWiFi* v;
    Datapoint* dp;
  };
  static void _onData(uint8_t* data, uint8_t len, void* arg);
  static void _onError(uint8_t error, void* arg);
  Optolink* _optolink;
  std::vector<Datapoint*> _datapoints;
  std::function<void(uint8_t, Datapoint*)> _onErrorCb;
};


// implementation
template<class D, typename T>
bool VitoWiFi::write(D& datapoint, T value) {  // NOLINT TODO(bertmelis): make const reference
  uint8_t* raw = new uint8_t[datapoint.getLength()];  // temporary variable to hold encoded value, will be copied by optolink
  datapoint.encode(raw, datapoint.getLength(), value);
  CbArg* arg = new CbArg(this, &datapoint);
  if (_optolink->write(datapoint.getAddress(), datapoint.getLength(), raw, reinterpret_cast<void*>(arg))) {
    delete[] raw;
    return true;
  } else {
    delete[] raw;
    return false;
  }
}


#elif defined VITOWIFI_TEST

// no tests here

#else

// unsuitable platform

#endif
