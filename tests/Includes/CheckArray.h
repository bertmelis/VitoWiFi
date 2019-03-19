/* copyright 2019 Bert Melis */

#include <stddef.h>
#include <sstream>
#include <iomanip>
#include "catch.hpp"

class ByteArrayEqualsBegin : public Catch::MatcherBase<const uint8_t*> {
 public:
  ByteArrayEqualsBegin(const uint8_t* standard, size_t size) :
    _standard(standard),
    _size(size) {}

  // Performs the test for this matcher
  bool match(const uint8_t* const& test) const override {
    for (size_t i = 0; i < _size; ++i) {
      if (test[i] != _standard[i]) return false;
    }
    return true;
  }

  // Produces a string describing what this matcher does. It should
  // include any provided data (the begin/ end in this case) and
  // be written as if it were stating a fact (in the output it will be
  // preceded by the value under test).
  std::string describe() const override {
      std::ostringstream ss;
      ss << "starts with 0x";
      for (size_t i = 0; i < _size; ++i) {
        ss << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << unsigned(_standard[i]);
      }
      return ss.str();
  }

 private:
  const uint8_t* _standard;
  const size_t _size;
};

// The builder function
inline ByteArrayEqualsBegin ByteArrayEqual(const uint8_t* arr, const size_t size) {
    return ByteArrayEqualsBegin(arr, size);
}
