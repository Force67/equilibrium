// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#include <gtest/gtest.h>

#include <type_traits>

#include "fixed_string.h"

namespace {

using FS16 = base::BasicFixedString<16>;
using FS8 = base::BasicFixedString<8>;

static_assert(std::is_trivially_copyable_v<FS16>,
              "FixedString must be trivially copyable");
static_assert(std::is_standard_layout_v<FS16>,
              "FixedString must be standard layout");

TEST(FixedStringTest, DefaultIsEmpty) {
  FS16 s;
  EXPECT_EQ(s.size(), 0u);
  EXPECT_TRUE(s.empty());
  EXPECT_STREQ(s.c_str(), "");
  EXPECT_EQ(FS16::capacity(), 16u);
}

TEST(FixedStringTest, ConstructFromCStr) {
  FS16 s("hello");
  EXPECT_EQ(s.size(), 5u);
  EXPECT_STREQ(s.c_str(), "hello");
}

TEST(FixedStringTest, ExactCapacityFits) {
  FS8 s("12345678");
  EXPECT_EQ(s.size(), 8u);
  EXPECT_STREQ(s.c_str(), "12345678");
}

TEST(FixedStringTest, OverflowAsserts) {
  EXPECT_DEATH({ FS8 s("123456789"); }, "");
}

TEST(FixedStringTest, AppendFits) {
  FS16 s("hello");
  s += " world";
  EXPECT_STREQ(s.c_str(), "hello world");
  EXPECT_EQ(s.size(), 11u);
}

TEST(FixedStringTest, AppendOverflowAsserts) {
  FS8 s("1234");
  EXPECT_DEATH({ s.append("567890"); }, "");
}

TEST(FixedStringTest, PushBackOverflowAsserts) {
  FS8 s("12345678");
  EXPECT_DEATH({ s.push_back('x'); }, "");
}

TEST(FixedStringTest, AssignTruncatingTruncates) {
  FS8 s;
  bool fit = s.assign_truncating("this string is way too long");
  EXPECT_FALSE(fit);
  EXPECT_EQ(s.size(), 8u);
  EXPECT_STREQ(s.c_str(), "this str");
}

TEST(FixedStringTest, AssignTruncatingFits) {
  FS8 s;
  bool fit = s.assign_truncating("hi");
  EXPECT_TRUE(fit);
  EXPECT_EQ(s.size(), 2u);
  EXPECT_STREQ(s.c_str(), "hi");
}

TEST(FixedStringTest, MemcpyableSemantics) {
  // Trivially copyable: round-trip via raw memory should be valid.
  FS16 a("hello");
  FS16 b;
  memcpy(&b, &a, sizeof(a));
  EXPECT_EQ(a, b);
  EXPECT_STREQ(b.c_str(), "hello");
}

TEST(FixedStringTest, EqualityOperators) {
  FS16 a("foo");
  FS16 b("foo");
  FS16 c("bar");
  EXPECT_EQ(a, b);
  EXPECT_NE(a, c);
  EXPECT_EQ(a, "foo");
}

TEST(FixedStringTest, Clear) {
  FS16 s("hello");
  s.clear();
  EXPECT_TRUE(s.empty());
  EXPECT_STREQ(s.c_str(), "");
}

TEST(FixedStringTest, FindChar) {
  FS16 s("path/to/file");
  EXPECT_EQ(s.find('/'), 4u);
  EXPECT_EQ(s.find('z'), FS16::npos);
}

TEST(FixedStringTest, ResizeShrinks) {
  FS16 s("hello world");
  s.resize(5);
  EXPECT_EQ(s.size(), 5u);
  EXPECT_STREQ(s.c_str(), "hello");
}

TEST(FixedStringTest, ResizeGrowsZeroFill) {
  FS16 s("hi");
  s.resize(5);
  EXPECT_EQ(s.size(), 5u);
  EXPECT_EQ(s[0], 'h');
  EXPECT_EQ(s[1], 'i');
  EXPECT_EQ(s[2], '\0');
}

}  // namespace
