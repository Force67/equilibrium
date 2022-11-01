// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/meta/source_location.h>
#include <gtest/gtest.h>

namespace {
TEST(SourceLocation, Crop) {
  MAKE_SOURCE_LOC("Mylongpath\\blah\\source_location_test.cc", __FILE__, __LINE__);
  ASSERT_STRCASEEQ(kSourceLoc.file, "source_location_test.cc");
}
}  // namespace