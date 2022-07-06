// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include <base/logging.h>

#include <base/memory/lazy_instance.h>

namespace {
struct ComplexObject {
  explicit ComplexObject(int a) : a_(a) {}
  int a_ = 0;
};

struct ComplexDefaultConstructedObject {
  ComplexDefaultConstructedObject() = default;
  int a_ = 0;
};

TEST(LazyInstance, ComplexInstance) {
  base::LazyInstance<ComplexObject> instance;
  EXPECT_FALSE(instance.constructed());

  instance.Make(1);
  EXPECT_TRUE(instance.constructed());
}

TEST(LazyInstance, ComplexDefaultConstructedInstance) {
  base::LazyInstance<ComplexDefaultConstructedObject> instance;
  EXPECT_FALSE(instance.constructed());

  instance.Make();
  EXPECT_TRUE(instance.constructed());
}
}  // namespace