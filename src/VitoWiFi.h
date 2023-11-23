/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include <cassert>

#ifdef ARDUINO
#include <HardwareSerial.h>
#include <SoftwareSerial.h>
#endif

#include "Constants.h"
#include "Logging.h"
#include "Helpers.h"

#include "Datapoint/Datapoint.h"
#include "VS2/VS2.h"

namespace VitoWiFi {

template<class PROTOCOLVERSION>
class VitoWiFi {
 public:
  #if defined(ARDUINO)
  template <class IFACE>
  explicit VitoWiFi(IFACE* interface)
  : _optolink(interface)
  , _onResponse(nullptr)
  , _onError(nullptr) {
    // empty
  }
  #endif

  void onResponse(typename PROTOCOLVERSION::OnResponseCallback callback) {
    _onResponse = callback;
  }

  void onError(typename PROTOCOLVERSION::OnErrorCallback callback) {
    _onError = callback;
  }

  bool begin() {
    return _optolink.begin();
  }

  void loop() {
    OptolinkResult result = _optolink.loop();
    if (result == OptolinkResult::CONTINUE) {
      // do nothing
    } else if (result == OptolinkResult::PACKET) {
      if (_onResponse) _onResponse(_optolink.response(), _optolink.datapoint());
    } else {
      // if not CONTINUE or PACKET, it must be an error
      if (_onError) _onError(result, _optolink.datapoint());
    }
  }

  bool read(Datapoint datapoint) {
    return _optolink.read(datapoint);
  }

  template <typename T>
  bool write(Datapoint datapoint, T value) {
    VariantValue v(value);
    return _optolink.write(datapoint, v);
  }

 private:
  PROTOCOLVERSION _optolink;
  typename PROTOCOLVERSION::OnResponseCallback _onResponse;
  typename PROTOCOLVERSION::OnErrorCallback _onError;
};

}  // end namespace VitoWiFi
