// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/memory/distinct_pointer_experimental.h>

namespace {
struct ComplexObject {
  explicit ComplexObject(int a) : a_(a) {}
  int a_ = 0;
};

struct ComplexObjectNoCtor {
  int a_ = 0;
};

#if 1
TEST(DistinctPointer, SimpleNoAssign) {
  base::DistinctPointer<char, base::MakeLater> var;
  EXPECT_TRUE(var.empty());
}

TEST(DistinctPointer, AssignSimple) {
  base::DistinctPointer<char> var('a');
  EXPECT_FALSE(var.empty());
}

TEST(DistinctPointer, AssignSimpleComplex) {
  base::DistinctPointer<ComplexObjectNoCtor> var;
  EXPECT_FALSE(var.empty());
}

TEST(DistinctPointer, AssignLaterSimple) {
  base::DistinctPointer<char, base::MakeLater> var;
  EXPECT_TRUE(var.empty());
  var.Make('a');
  EXPECT_FALSE(var.empty());
}

TEST(DistinctPointer, AssignComplex) {
  base::DistinctPointer<ComplexObject> var(1);
  EXPECT_FALSE(var.empty());
}

TEST(DistinctPointer, AssignLaterComplex) {
  base::DistinctPointer<ComplexObject, base::MakeLater> var;
  EXPECT_TRUE(var.empty());
  var.Make(1);
  EXPECT_FALSE(var.empty());
}

TEST(DistinctPointer, AssignComplexNoConstruct) {
  base::DistinctPointer<ComplexObjectNoCtor, base::MakeLater> var;
  EXPECT_TRUE(var.empty());
}
#endif

// below should never compile
#if 0
TEST(DistinctPointer, AssignComplexNoConstruct2) {
  base::DistinctPointer<ComplexObjectNoCtor> var();
  EXPECT_TRUE(var.empty());
}
#endif
}  // namespace