/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.

Serial interface code by @gbmhunter:
https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/
https://github.com/gbmhunter/CppLinuxSerial

(consulted 11/2023)

*/

#pragma once

#if defined(__linux__)

#include <cassert>

// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h>  // Contains file controls like O_RDWR
#include <errno.h>  // Error integer and strerror() function
#include <termios.h>  // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()
#include <sys/ioctl.h>

#include "SerialInterface.h"
#include "../Logging.h"

namespace VitoWiFiInternals {

class LinuxSerialInterface : public SerialInterface {
 public:
  explicit LinuxSerialInterface(const char* interface);
  bool begin() override;
  void end() override;
  std::size_t write(const uint8_t* data, uint8_t length) override;
  uint8_t read() override;
  size_t available() override;

 private:
  const char* _interfaceName;
  int _fd;
  struct termios _tty;
};

}  // end namespace VitoWiFiInternals

#endif
