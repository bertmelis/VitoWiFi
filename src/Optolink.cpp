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

#include "Optolink.h"

Optolink_DP::Optolink_DP(uint16_t address, uint8_t length, bool write, uint8_t* value, void* arg) :
    address(address),
    length(length),
    write(write),
    data(nullptr),
    arg(arg) {
        if (write) {
            data = new uint8_t[length];
            memcpy(data, value, length);
        }
    }

Optolink_DP::~Optolink_DP() {
    delete[] data;
}

Optolink::Optolink(HardwareSerial* serial) :
    _serial(serial),
    _queue(),
    _onData(nullptr),
    _onError(nullptr) {}

Optolink::~Optolink() {
    // TODO(bertmelis): anything to do?
}

void Optolink::onData(std::function<void(uint8_t* data, uint8_t len, void* arg)> callback) {
    _onData = callback;
}

void Optolink::onError(std::function<void(uint8_t error)> callback) {
    _onError = callback;
}

bool Optolink::read(uint16_t address, uint8_t length, void* arg) {
    if (_queue.size() < VITOWIFI_MAX_QUEUE_LENGTH) {
        _queue.emplace(address, length, false, nullptr, arg);
        return true;
    }
    return false;
}

bool Optolink::write(uint16_t address, uint8_t length, uint8_t* data, void* arg) {
    if (_queue.size() < VITOWIFI_MAX_QUEUE_LENGTH) {
        _queue.emplace(address, length, true, data, arg);
        return true;
    }
    return false;
}
