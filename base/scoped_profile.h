// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <chrono>
#include <base/logging.h>

namespace base {
namespace cn = std::chrono;

struct ScopedProfile {
  cn::high_resolution_clock::time_point delta;
  const char* const name;

  ScopedProfile(const char* const name)
      : name(name), delta(cn::high_resolution_clock::now()) {}

  ~ScopedProfile() {
    auto now = std::chrono::high_resolution_clock::now();
    auto nanos =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - delta)
            .count();
    auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(now - delta).count();
    LOG_INFO("~ScopedTimer() : <{}> : {} ms ({} s)", name, nanos, seconds);
  }
};
}  // namespace base