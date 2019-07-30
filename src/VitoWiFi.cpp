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

#include <VitoWiFi.h>

VitoWiFi::VitoWiFi(VitoWiFiProtocol protocol, HardwareSerial* serial) :
  _optolink(nullptr),
  _datapoints(),
  _onErrorCb(nullptr) {
    switch (protocol) {
    case P300:
      _optolink = new OptolinkP300(serial);
      break;
    case KW:
      // to be implemented
    default:
      abort();
    }
    if (_optolink) {
      _optolink->begin();
    }
}

VitoWiFi::~VitoWiFi() {
  delete _optolink;
}

void VitoWiFi::addDatapoint(Datapoint* datapoint) {
  if (datapoint) _datapoints.push_back(datapoint);
}

void VitoWiFi::onError(std::function<void(uint8_t, Datapoint*)> callback) {
  _onErrorCb = callback;
}

void VitoWiFi::begin() {
  _datapoints.shrink_to_fit();
  _optolink->onData(&VitoWiFi::_onData);
  _optolink->onError(&VitoWiFi::_onError);
  _optolink->begin();
}

void VitoWiFi::loop() {
  _optolink->loop();
}

void VitoWiFi::readAll() {
  for (Datapoint* dp : _datapoints) {
    read(*dp);
  }
}

bool VitoWiFi::read(Datapoint& datapoint) {  // NOLINT TODO(bertmelis): make const reference
  CbArg* arg = new CbArg(this, &datapoint);
  if (_optolink->read(datapoint.getAddress(), datapoint.getLength(), reinterpret_cast<void*>(arg))) {
    return true;
  } else {
    delete arg;
    return false;
  }
}

void VitoWiFi::_onData(uint8_t* data, uint8_t len, void* arg) {
  CbArg* cbArg = reinterpret_cast<CbArg*>(arg);
  cbArg->dp->decode(data, len, cbArg->dp);
  delete cbArg;
}

void VitoWiFi::_onError(uint8_t error, void* arg) {
  CbArg* cbArg = reinterpret_cast<CbArg*>(arg);
  if (cbArg->v->_onErrorCb) cbArg->v->_onErrorCb(error, cbArg->dp);
  delete cbArg;
}
