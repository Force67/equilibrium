// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <random>
#include "network_host.h"

namespace network {

constexpr uint32_t kReservedSeedSpace = 10;

std::string NetworkHost::LastError() const {
  return sock_.last_error_str();
}

uint32_t NetworkHost::GetSeed() {
  auto mersenneRandom = std::mt19937(std::random_device{}());
  // give us a random value between 0 and limit
  // some of those upper values are reserved for the system.
  std::uniform_int_distribution<uint32_t> limits(
            0, UINT_MAX - kReservedSeedSpace);

  return limits(mersenneRandom);
}
}