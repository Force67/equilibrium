// Copyright (C) 2024 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// STL-free number-to-string conversions.
#pragma once

#include <base/arch.h>
#include <base/strings/base_string.h>
#include <cstdio>

namespace base {

inline String ToString(int value) {
  char buf[32];
  int len = snprintf(buf, sizeof(buf), "%d", value);
  return String(buf, static_cast<mem_size>(len));
}

inline String ToString(unsigned int value) {
  char buf[32];
  int len = snprintf(buf, sizeof(buf), "%u", value);
  return String(buf, static_cast<mem_size>(len));
}

inline String ToString(long value) {
  char buf[32];
  int len = snprintf(buf, sizeof(buf), "%ld", value);
  return String(buf, static_cast<mem_size>(len));
}

inline String ToString(unsigned long value) {
  char buf[32];
  int len = snprintf(buf, sizeof(buf), "%lu", value);
  return String(buf, static_cast<mem_size>(len));
}

inline String ToString(long long value) {
  char buf[32];
  int len = snprintf(buf, sizeof(buf), "%lld", value);
  return String(buf, static_cast<mem_size>(len));
}

inline String ToString(unsigned long long value) {
  char buf[32];
  int len = snprintf(buf, sizeof(buf), "%llu", value);
  return String(buf, static_cast<mem_size>(len));
}

inline String ToString(float value) {
  char buf[64];
  int len = snprintf(buf, sizeof(buf), "%.1f", value);
  return String(buf, static_cast<mem_size>(len));
}

inline String ToString(double value) {
  char buf[64];
  int len = snprintf(buf, sizeof(buf), "%.2f", value);
  return String(buf, static_cast<mem_size>(len));
}

}  // namespace base
