/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "Helpers.h"

namespace VitoWiFi {

void reverse(uint8_t* arr, uint8_t len) {
  uint8_t start = 0;
  while (0 < len) {
    uint8_t temp = arr[start];
    arr[start] = arr[len];
    arr[len] = temp;
    start++;
    len--;
  }
}

}  // end namespace VitoWiFi
