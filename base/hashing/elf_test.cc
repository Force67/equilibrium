// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "gtest/gtest.h"
#include "elf.h"

namespace {
struct {
  const char* key;
  const u32 expected;
} kTestValues[] = {{"", 0},
                   {"a", 0x61},
                   {"ab", 0x672},
                   {"abc", 0x6783},
                   {"abcd", 0x67894},
                   {"abcde", 0x6789a5},
                   {"abcdef", 0x6789ab6},
                   {"abcdefg", 0x6789abc7},
                   {"abcdefgh", 0x789abcd8}};

TEST(ElfHash, Hash) {
  for (const auto& pair : kTestValues) {
    const auto actual_hash = base::ElfHash(pair.key);
    EXPECT_EQ(pair.expected, actual_hash) << "For input string: " << pair.key;
  }
}
}  // namespace