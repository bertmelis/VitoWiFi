/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include "VS2/VS2.h"
#include "VS1/VS1.h"
#include "GWG/GWG.h"

namespace VitoWiFi {

template<class PROTOCOLVERSION>
class VitoWiFi {
 public:
  template <class IFACE>
  explicit VitoWiFi(IFACE* interface)
  : _optolink(interface) {
    // empty
  }

  void onResponse(typename PROTOCOLVERSION::OnResponseCallback callback) {
    _optolink.onResponse(callback);
  }

  void onError(typename PROTOCOLVERSION::OnErrorCallback callback) {
    _optolink.onError(callback);
  }

  bool begin() {
    return _optolink.begin();
  }

  void end() {
    return _optolink.end();
  }

  void loop() {
    _optolink.loop();
  }

  bool read(Datapoint datapoint) {
    return _optolink.read(datapoint);
  }

  template <typename T>
  bool write(Datapoint datapoint, T value) {
    VariantValue v(value);
    return _optolink.write(datapoint, v);
  }

  bool write(Datapoint datapoint, const uint8_t* data, uint8_t length) {
    return _optolink.write(datapoint, data, length);
  }

  int getState() {
    return static_cast<int>(_optolink.getState());
  }

 private:
  PROTOCOLVERSION _optolink;
};

}  // end namespace VitoWiFi
