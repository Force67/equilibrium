// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/expected.h>

#if 0
namespace {
TEST(Expected, AssignUnequalTypes) {
  base::Expected<bool /*value*/, int /*error_value*/> a(true);
  EXPECT_TRUE(!a.has_error());
  EXPECT_TRUE(a.value());
}

struct Complex {
 private:
  int a;
 public:
  Complex(int a) : a(a) {}

  int get_a() const { return a; }
};

static void ff(const Complex& c) {
  EXPECT_EQ(c.get_a(), 10);
}

// types are unequal, one ctor got disabled, now choose between two very similar ones.
TEST(Expected, AssignDerivedTypes) {
  ff(10);
 
  Complex a(10);
  base::Expected<Complex /*value*/, int/*error_value*/> exp(a);
  EXPECT_TRUE(!exp.has_error());
  EXPECT_EQ(exp.value().get_a(), 10);

  base::Expected<Complex /*value*/, int /*error_value*/> exp2(10);
  EXPECT_TRUE(exp2.has_error());
  EXPECT_EQ(exp2.error(), 10);
}

TEST(Expected, AssignEqualTypes) {
  base::Expected<int /*value*/, int /*error_value*/> a(1, false /*not an error*/);
  EXPECT_TRUE(!a.has_error());
  EXPECT_EQ(a.value(), 1);

  base::Expected<int /*value*/, int /*error_value*/> b(1, true/*an error*/);
  EXPECT_TRUE(b.has_error());
  EXPECT_EQ(b.error(), 1);
}
}  // namespace
#endif