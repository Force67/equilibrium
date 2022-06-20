// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/filesystem/path.h>
#include <base/dynamic_library.h>

namespace {
TEST(DynamicLibrary, EmptyLoad) {
  const base::DynamicLibrary lib;
  ASSERT_FALSE(lib.loaded());
}

TEST(DynamicLibrary, Load) {
  const base::DynamicLibrary lib(
#if defined(OS_LINUX)
	  "/lib/x86_64-linux-gnu/libstdc++.so"
#elif defined(OS_WIN)
	  "iphlpapi.dll"
#endif
  );
  ASSERT_TRUE(lib.loaded());
}

TEST(DynamicLibrary, LoadExisting) {
  const base::Path kPath(
#if defined(OS_LINUX)
	  "libc.so"
#elif defined(OS_WIN)
      "kernel32.dll"
#endif
  );
  const base::DynamicLibrary lib(kPath);
  ASSERT_TRUE(lib.loaded());
}
}  // namespace