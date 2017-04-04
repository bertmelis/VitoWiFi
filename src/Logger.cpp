#include "Logger.h"

Logger::Logger()
: _loggingEnabled(false)
, _printer(&Serial1) {
}

void Logger::setPrinter(Print* printer) {
  _printer = printer;
}

void Logger::setLogging(bool enable){
  _loggingEnabled = enable;
}

size_t Logger::write(uint8_t character) {
  if (_loggingEnabled) _printer->write(character);
}

size_t Logger::write(const uint8_t* buffer, size_t size) {
  if (_loggingEnabled) _printer->write(buffer, size);
}
