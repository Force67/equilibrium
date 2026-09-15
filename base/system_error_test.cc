// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <errno.h>
#include <string.h>

#include <base/system_error.h>

namespace {

TEST(SystemError, NamesTheCommonCodes) {
  EXPECT_STREQ(base::ErrnoName(ENOENT), "ENOENT");
  EXPECT_STREQ(base::ErrnoName(EACCES), "EACCES");
  EXPECT_STREQ(base::ErrnoName(EINVAL), "EINVAL");
  EXPECT_STREQ(base::ErrnoName(ENOMEM), "ENOMEM");
  EXPECT_STREQ(base::ErrnoName(EINTR), "EINTR");
}

// Unknown codes render numerically rather than returning null, because every
// caller drops the result straight into a log line.
TEST(SystemError, RendersUnknownCodesNumerically) {
  const char* name = base::ErrnoName(31337);
  ASSERT_NE(name, nullptr);
  EXPECT_STREQ(name, "E31337");

  EXPECT_STREQ(base::ErrnoName(0), "E0");
  EXPECT_STREQ(base::ErrnoName(-1), "E-1");
}

TEST(SystemError, NeverReturnsNull) {
  for (int code = -5; code < 200; code++)
    EXPECT_NE(base::ErrnoName(code), nullptr) << "code " << code;
}

// The name must outlive the call, since callers pass it on rather than copy.
TEST(SystemError, KnownNamesAreStable) {
  const char* first = base::ErrnoName(ENOENT);
  const char* second = base::ErrnoName(ENOENT);
  EXPECT_EQ(first, second);
  EXPECT_STREQ(first, "ENOENT");
}

}  // namespace
