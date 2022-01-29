/**
 * @file include/capstone2llvmir/address.h
 * @brief Address, address pair and other derived class representation.
 * @copyright (c) 2019 Avast Software, licensed under the MIT license
 */

#ifndef RETDEC_COMMON_ADDRESS_H
#define RETDEC_COMMON_ADDRESS_H

#include <base/check.h>
#include <cstddef>
#include <ostream>
#include <set>
#include <sstream>

namespace retdec {
namespace common {

class Address {
 public:
  Address() = default;
  Address(uint64_t a) : address(a) {}
  operator uint64_t() const { return address; }
  explicit operator bool() const { return isDefined() && address; }

  Address& operator++() {
    if (isDefined())
      address++;

    return *this;
  }
  Address operator++(int) {
    if (isDefined())
      address++;

    return *this;
  }

  Address& operator--() {
    if (isDefined())
      address--;

    return *this;
  }
  Address operator--(int) {
    if (isDefined())
      address--;

    return *this;
  }

  Address& operator+=(const Address& rhs) {
    address += rhs;
    return *this;
  }
  Address& operator-=(const Address& rhs) {
    address -= rhs;
    return *this;
  }
  Address& operator|=(const Address& rhs) {
    address |= rhs;
    return *this;
  }

  bool isUndefined() const { return address == Undefined; }

  bool isDefined() const { return !isUndefined(); }

  uint64_t getValue() const {
    BUGCHECK(isDefined());
    return address;
  }

 public:
  static const uint64_t Undefined = ULLONG_MAX;

 private:
  uint64_t address;
};

}  // namespace common
}  // namespace retdec

#endif