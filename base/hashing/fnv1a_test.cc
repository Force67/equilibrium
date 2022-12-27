// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "fnv1a.h"
#include "gtest/gtest.h"

namespace {
struct {
  const char* name;
  const u64 expected_hash;
} kHashResultMap64[] = {
    {"", 0},
    {"a", 12638187200555641996ull},
    {"ab", 620445648566982762ull},
    {"abc", 16654208175385433931ull},
    {"abcd", 18165163011005162717ull},
    {"abcde", 7154184807124264104ull},
    {"abcdef", 15567776504244095498ull},
    {"abcdefg", 4642726675185563447ull},
    {"abcdefgh", 2727646559950394989ull},
    {"abcdefghi", 18100548702444955852ull},
    {"abcdefghij", 13383510251204481554ull},
};

struct {
  const char* name;
  const u32 expected_hash;
} kHashResultMap32[] = {
    {"", 0},
    {"a", 0xe40c292c},
    {"ab", 0x4d2505ca},
    {"abc", 0x1a47e90b},
    {"abcd", 0xce3479bd},
    {"abcde", 0x749bcf08},
    {"abcdef", 0xff478a2a},
    {"abcdefg", 0x2a9eb737},
    {"abcdefgh", 0x76daaa8d},
    {"abcdefghi", 0xfe3b04ec},
    {"abcdefghij", 0xbce81ef2},
};

TEST(FNV1a, Hash64) {
  for (auto& it : kHashResultMap64) {
    const auto actual_hash = base::FNV1a64(it.name);
    EXPECT_EQ(it.expected_hash, actual_hash) << "For input string: " << it.name;
  }
}

TEST(FNV1a, Hash32) {
  for (auto& it : kHashResultMap32) {
    const auto actual_hash = base::FNV1a32(it.name);
    EXPECT_EQ(it.expected_hash, actual_hash) << "For input string: " << it.name;
  }
}
}  // namespace