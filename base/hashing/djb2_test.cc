// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "gtest/gtest.h"
#include "djb2.h"

namespace {
struct {
  const char* key;
  const u64 expected;
} kTestValues[] = {{"", 0},
                   {"a", 733599527},
                   {"ab", 24209185930},
                   {"abc", 798903541326},
                   {"abcd", 26363817273491},
                   {"abcde", 870005970439033},
                   {"abcdef", 28710197024906016},
                   {"abcdefg", 947436501822320552},
                   {"abcdefgh", 12818660486427452721}};

TEST(Djb2, Hash64) {
  for (const auto& pair : kTestValues) {
    const auto actual_hash = base::Djb2Hash64(pair.key);
    EXPECT_EQ(pair.expected, actual_hash) << "For input string: " << pair.key;
  }
}

struct {
  const char* key;
  const u32 expected;
} kTestValues32[] = {{"", 0},
                     {"a", 177670},
                     {"ab", 5863208},
                     {"abc", 193485963},
                     {"abcd", 2090069583},
                     {"abcde", 252819604},
                     {"abcdef", 4048079738},
                     {"abcdefg", 442645281},
                     {"abcdefgh", 1722392489}};

TEST(Djb2, Hash32) {
  for (const auto& pair : kTestValues32) {
    const auto actual_hash = base::Djb2Hash32(pair.key);
    EXPECT_EQ(pair.expected, actual_hash) << "For input string: " << pair.key;
  }
}

}  // namespace