// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/memory/unique_pointer.h>

namespace {
struct ComplexObject {
  explicit ComplexObject(int a) : a_(a) {}
  int a_ = 0;
};

struct ComplexDefaultConstructedObject {
  ComplexDefaultConstructedObject() = default;
  int a_ = 0;
};

TEST(UniquePointer, CreateIm) {
  auto pointer = base::MakeUnique<ComplexObject>(1337);
  EXPECT_TRUE(pointer);  // operator bool
  EXPECT_EQ(pointer->a_, 1337);
}

TEST(UniquePointer, CreateDelayed) {
  base::UniquePointer<ComplexObject> pointer;
  EXPECT_FALSE(pointer);  // operator bool

  pointer = base::MakeUnique<ComplexObject>(1337);
  EXPECT_TRUE(pointer);
  EXPECT_EQ(pointer->a_, 1337);
}

TEST(UniquePointer, Move) {
  auto a = base::MakeUnique<ComplexObject>(1337);
  EXPECT_TRUE(a);  // operator bool
  EXPECT_EQ(a->a_, 1337);

  base::UniquePointer<ComplexObject> b = base::move(a);
  EXPECT_TRUE(b);  // operator bool
  EXPECT_FALSE(a);
}

TEST(UniquePointer, MoveConstruct) {
  auto a = base::MakeUnique<ComplexObject>(1337);
  EXPECT_TRUE(a);  // operator bool
  EXPECT_EQ(a->a_, 1337);

  base::UniquePointer<ComplexObject> b(base::move(a));
  EXPECT_TRUE(b);  // operator bool
  EXPECT_FALSE(a);
}

// TODO: this should have a leak detector check
TEST(UniquePointer, ArrayAccess) {
  auto a = base::MakeUnique<u8[]>(4);
  EXPECT_TRUE(a);  // operator bool

  a[3] = 4;
  EXPECT_EQ(a[3], 4);
}
}  // namespace