#pragma once
// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

namespace base {

consteval size_t MakeIntegralSemVer(size_t major,
                                    size_t minor,
                                    size_t patch,
                                    size_t pp) {
  return major * 1000000 + minor * 10000 + patch * 100 + pp;
}

void ExtractIntegralSemVer(size_t& major, size_t& minor, size_t& patch, size_t& pp) {

}
}  // namespace base
