// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/containers/span.h>
#include <base/containers/vector.h>

namespace {
TEST(SpanTest, ConstructFrom) {
  base::Vector<i32> vec;
  for (i32 i = 0; i < 10; i++) {
    vec.push_back(i);
  }

  base::Span a = base::MakeSpan(vec);
  EXPECT_EQ(a.size(), 10);

  base::Span<i32> b(vec);
  EXPECT_EQ(b.size(), 10);
}
}  // namespace
