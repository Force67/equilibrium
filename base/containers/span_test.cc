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

TEST(SpanTest, DefaultIsEmpty) {
  base::Span<const i32> s;
  EXPECT_TRUE(s.empty());
  EXPECT_EQ(s.data(), nullptr);
  EXPECT_EQ(s.begin(), s.end());
}

TEST(SpanTest, WritesThroughNonConstElement) {
  i32 arr[3] = {1, 2, 3};
  base::Span<i32> s(arr);
  s[1] = 20;
  s.front() = 10;
  s.back() = 30;
  s.data()[0] += 1;
  EXPECT_EQ(arr[0], 11);
  EXPECT_EQ(arr[1], 20);
  EXPECT_EQ(arr[2], 30);
}

TEST(SpanTest, Slices) {
  const i32 arr[5] = {0, 1, 2, 3, 4};
  base::Span<const i32> s(arr);
  EXPECT_EQ(s.size_bytes(), 5 * sizeof(i32));

  auto head = s.first(2);
  ASSERT_EQ(head.size(), 2u);
  EXPECT_EQ(head[1], 1);

  auto tail = s.last(2);
  ASSERT_EQ(tail.size(), 2u);
  EXPECT_EQ(tail[0], 3);

  auto mid = s.subspan(1, 3);
  ASSERT_EQ(mid.size(), 3u);
  EXPECT_EQ(mid.front(), 1);
  EXPECT_EQ(mid.back(), 3);

  auto rest = s.subspan(2);
  ASSERT_EQ(rest.size(), 3u);
  EXPECT_EQ(rest[0], 2);

  EXPECT_TRUE(s.subspan(5).empty());
  EXPECT_TRUE(s.first(0).empty());
}

TEST(SpanTest, ConstSpanFromMutableSpan) {
  i32 arr[2] = {7, 8};
  base::Span<i32> writable(arr);
  base::Span<const i32> view(writable);
  EXPECT_EQ(view.size(), 2u);
  EXPECT_EQ(view[1], 8);
}

TEST(SpanTest, ConstSpanFromTemporarySpan) {
  i32 arr[3] = {1, 2, 3};
  auto sum = [](base::Span<const i32> s) {
    i32 total = 0;
    for (i32 v : s) total += v;
    return total;
  };
  EXPECT_EQ(sum(base::Span<i32>(arr, 3)), 6);
  EXPECT_EQ(sum(base::Span(&arr[1], 1)), 2);
}

TEST(SpanTest, DeducesFromContainer) {
  base::Vector<i32> vec;
  vec.push_back(4);
  vec.push_back(5);
  auto s = base::Span(vec);
  static_assert(base::is_same_v<decltype(s), base::Span<i32>>);
  EXPECT_EQ(s.first(1)[0], 4);

  const base::Vector<i32>& cvec = vec;
  auto cs = base::Span(cvec);
  static_assert(base::is_same_v<decltype(cs), base::Span<const i32>>);
  EXPECT_EQ(cs.size(), 2u);
}
}  // namespace
