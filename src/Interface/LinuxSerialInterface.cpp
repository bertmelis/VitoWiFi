/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#if defined(__linux__)

#include "LinuxSerialInterface.h"

namespace VitoWiFiInternals {

LinuxSerialInterface::LinuxSerialInterface(const char* interface)
: _interfaceName(interface)
, _fd(0)
, _tty() {
  assert(interface);
}

bool LinuxSerialInterface::begin() {
  _fd = open(_interfaceName, O_RDWR);
  if (_fd < 0) {
    vw_log_e("Error %i from open: %s\n", errno, strerror(errno));
    return false;
  }
  if (tcgetattr(_fd, &_tty) != 0) {
    vw_log_e("Error %i from tcgetattr: %s\n", errno, strerror(errno));
    return false;
  }

  _tty.c_cflag |= PARENB;  // parity enable
  _tty.c_cflag |= CSTOPB;  // 2 stop bits
  _tty.c_cflag |= CS8;  // 8 bits per byte
  _tty.c_cflag &= ~CRTSCTS;  // Disable RTS/CTS hardware flow control
  _tty.c_cflag |= CREAD | CLOCAL;  // Turn on READ & ignore ctrl lines (CLOCAL = 1)

  _tty.c_lflag &= ~ICANON;
  _tty.c_lflag &= ~ECHO;  // Disable echo
  _tty.c_lflag &= ~ECHOE;  // Disable erasure
  _tty.c_lflag &= ~ECHONL;  // Disable new-line echo
  _tty.c_lflag &= ~ISIG;  // Disable interpretation of INTR, QUIT and SUSP
  _tty.c_iflag &= ~(IXON | IXOFF | IXANY);  // Turn off s/w flow ctrl
  _tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);  // Disable any special handling of received bytes

  _tty.c_oflag &= ~OPOST;  // Prevent special interpretation of output bytes (e.g. newline chars)
  _tty.c_oflag &= ~ONLCR;  // Prevent conversion of newline to carriage return/line feed
  // _tty.c_oflag &= ~OXTABS;  // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
  // _tty.c_oflag &= ~ONOEOT;  // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

  _tty.c_cc[VTIME] = 10;  // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
  _tty.c_cc[VMIN] = 0;

  // Set in/out baud rate to be 9600
  cfsetispeed(&_tty, B4800);
  cfsetospeed(&_tty, B4800);

  // Save tty settings, also checking for error
  if (tcsetattr(_fd, TCSANOW, &_tty) != 0) {
    vw_log_e("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    return false;
  }

  return true;
}

void LinuxSerialInterface::end() {
  ::close(_fd);
}

std::size_t LinuxSerialInterface::write(const uint8_t* data, uint8_t length) {
  ssize_t retVal = ::write(_fd, data, length);
  if (retVal < 0) {
    vw_log_w("Error writing serial port");
    return 0;
  }
  std::cout << "tx (" << unsigned(length) << "): 0x";
  for (uint8_t i = 0; i < retVal; ++i) {
    std::cout << std::setfill('0') << std::setw(2) << std::hex << unsigned(data[i]);
  }
  std::cout << std::endl;
  return retVal;
}

uint8_t LinuxSerialInterface::read() {
  uint8_t buf;
  ssize_t retVal = ::read(_fd, &buf, 1);
  if (retVal < 0) {
    vw_log_e("Error reading serial port");
    return 0;
  }
  return buf;
}

size_t LinuxSerialInterface::available() {
  int bytesAvailable;
  ioctl(_fd, FIONREAD, &bytesAvailable);
  return bytesAvailable;
}

}  // end namespace VitoWiFiInternals

#endif
