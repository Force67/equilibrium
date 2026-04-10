// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#include <gtest/gtest.h>

#include "small_string.h"

namespace {

using SS32 = base::BasicSmallString<char, 32>;
using SS8 = base::BasicSmallString<char, 8>;

// u32 size_type pin: catches accidental regressions to size_t.
static_assert(std::is_same_v<SS32::size_type, arch_types::u32>);
// SmallString<32> on a 64-bit system: LargeRep is 16 bytes, SmallRep is 40
// (33 chars + 3 pad + 4 size), bool tail + padding rounds the whole thing
// to the next multiple of alignof(char*) = 8.
static_assert(sizeof(SS32) == 48,
              "SmallString<32> size regression — check the size_type savings");

TEST(SmallStringTest, DefaultIsEmptyAndInline) {
  SS32 s;
  EXPECT_EQ(s.size(), 0u);
  EXPECT_TRUE(s.empty());
  EXPECT_TRUE(s.is_inline());
  EXPECT_STREQ(s.c_str(), "");
  EXPECT_EQ(s.capacity(), 32u);
}

TEST(SmallStringTest, ConstructFromCStrInline) {
  SS32 s("hello");
  EXPECT_EQ(s.size(), 5u);
  EXPECT_TRUE(s.is_inline());
  EXPECT_STREQ(s.c_str(), "hello");
}

TEST(SmallStringTest, ConstructFromCStrSpillsToHeap) {
  // 33 chars > 32 inline capacity → must spill.
  const char* kLong = "abcdefghijklmnopqrstuvwxyz0123456";
  SS32 s(kLong);
  EXPECT_EQ(s.size(), 33u);
  EXPECT_FALSE(s.is_inline());
  EXPECT_STREQ(s.c_str(), kLong);
  EXPECT_GE(s.capacity(), 33u);
}

TEST(SmallStringTest, ExactCapacityFitsInline) {
  // Exactly 32 chars must still be inline.
  SS32 s("12345678901234567890123456789012");
  EXPECT_EQ(s.size(), 32u);
  EXPECT_TRUE(s.is_inline());
}

TEST(SmallStringTest, AppendCrossesInlineBoundary) {
  SS8 s("1234");
  EXPECT_TRUE(s.is_inline());
  s += "56789ABCDEF";  // total 15 > 8
  EXPECT_FALSE(s.is_inline());
  EXPECT_STREQ(s.c_str(), "123456789ABCDEF");
  EXPECT_EQ(s.size(), 15u);
}

TEST(SmallStringTest, PushBackGrowsCorrectly) {
  SS8 s;
  for (int i = 0; i < 20; ++i) s.push_back('a' + (i % 26));
  EXPECT_EQ(s.size(), 20u);
  EXPECT_FALSE(s.is_inline());
  EXPECT_EQ(s[0], 'a');
  EXPECT_EQ(s[19], 't');
}

TEST(SmallStringTest, CopyCtorPreservesContent) {
  SS32 a("the quick brown fox jumps");
  SS32 b(a);
  EXPECT_EQ(a, b);
  EXPECT_NE(a.data(), b.data());  // independent storage
}

TEST(SmallStringTest, CopyCtorOfHeapString) {
  SS8 a("this string is definitely too long for inline");
  EXPECT_FALSE(a.is_inline());
  SS8 b(a);
  EXPECT_FALSE(b.is_inline());
  EXPECT_EQ(a, b);
  EXPECT_NE(a.data(), b.data());
}

TEST(SmallStringTest, MoveCtorStealsHeap) {
  SS8 a("this string is definitely too long for inline");
  const char* old_ptr = a.data();
  EXPECT_FALSE(a.is_inline());

  SS8 b(std::move(a));
  EXPECT_EQ(b.data(), old_ptr);    // pointer stolen
  EXPECT_TRUE(a.is_inline());      // moved-from is empty inline
  EXPECT_EQ(a.size(), 0u);
}

TEST(SmallStringTest, MoveCtorOfInlineCopies) {
  SS32 a("hello");
  SS32 b(std::move(a));
  EXPECT_STREQ(b.c_str(), "hello");
  EXPECT_EQ(a.size(), 0u);
}

TEST(SmallStringTest, AssignmentReplacesContent) {
  SS32 s("foo");
  s = "bar baz";
  EXPECT_STREQ(s.c_str(), "bar baz");
  EXPECT_EQ(s.size(), 7u);
}

TEST(SmallStringTest, ShrinkToFitMovesHeapBackInline) {
  SS8 s;
  s.append("0123456789");  // 10 chars, on heap
  EXPECT_FALSE(s.is_inline());
  s.erase(8);  // size becomes 8
  EXPECT_EQ(s.size(), 8u);
  s.shrink_to_fit();
  EXPECT_TRUE(s.is_inline());
  EXPECT_STREQ(s.c_str(), "01234567");
}

TEST(SmallStringTest, ClearKeepsCapacity) {
  SS8 s("0123456789ABCDEF");  // heap
  EXPECT_FALSE(s.is_inline());
  const auto cap = s.capacity();
  s.clear();
  EXPECT_EQ(s.size(), 0u);
  EXPECT_EQ(s.capacity(), cap);  // doesn't drop heap allocation
  EXPECT_STREQ(s.c_str(), "");
}

TEST(SmallStringTest, FindChar) {
  SS32 s("path/to/file.txt");
  EXPECT_EQ(s.find('/'), 4u);
  EXPECT_EQ(s.find('.'), 12u);
  EXPECT_EQ(s.find('z'), SS32::npos);
}

TEST(SmallStringTest, FindSubstring) {
  SS32 s("the quick brown fox");
  EXPECT_EQ(s.find("quick"), 4u);
  EXPECT_EQ(s.find("slow"), SS32::npos);
}

TEST(SmallStringTest, Substr) {
  SS32 s("hello world");
  auto sub = s.substr(6, 5);
  EXPECT_STREQ(sub.c_str(), "world");
}

TEST(SmallStringTest, EqualityOperators) {
  SS32 a("foo");
  SS32 b("foo");
  SS32 c("bar");
  EXPECT_EQ(a, b);
  EXPECT_NE(a, c);
  EXPECT_EQ(a, "foo");
  EXPECT_NE(a, "fo");
}

TEST(SmallStringTest, OrderingOperators) {
  SS32 a("apple");
  SS32 b("banana");
  EXPECT_LT(a, b);
  EXPECT_LE(a, b);
  EXPECT_GT(b, a);
  EXPECT_GE(b, a);
}

TEST(SmallStringTest, ReserveTriggersHeap) {
  SS8 s("hi");
  EXPECT_TRUE(s.is_inline());
  s.reserve(100);
  EXPECT_FALSE(s.is_inline());
  EXPECT_GE(s.capacity(), 100u);
  EXPECT_STREQ(s.c_str(), "hi");
}

}  // namespace
