// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include "joaat.h"

using namespace base;

TEST(JoaatTest, EmptyString) {
  const byte data[] = "";
  EXPECT_EQ(Joaat(data, 0), 0);
}

TEST(JoaatTest, SingleCharacter) {
  const byte data[] = {'a'};
  EXPECT_EQ(Joaat(data, 1), 0xca2e9442);
}

TEST(JoaatTest, MultipleCharacters) {
  const byte data[] = {'a', 'b', 'c'};
  EXPECT_EQ(Joaat(data, 3), 0xed131f5b);
}

TEST(JoaatTest, CaseSensitivity) {
  const byte data1[] = {'a', 'B', 'c'};
  const byte data2[] = {'A', 'b', 'C'};
  EXPECT_NE(Joaat(data1, 3), Joaat(data2, 3));
}

TEST(JoaatTest, SameStringDifferentInstances) {
  const byte data1[] = {'a', 'b', 'c'};
  const byte data2[] = {'a', 'b', 'c'};
  EXPECT_EQ(Joaat(data1, 3), Joaat(data2, 3));
}
