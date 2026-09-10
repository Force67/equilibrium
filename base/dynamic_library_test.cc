// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/filesystem/path.h>
#include <base/dynamic_library.h>

namespace {

// A module every platform already has mapped by the time the tests run, and a
// symbol it exports. Built on demand rather than at namespace scope, to keep a
// Path out of static initialization.
base::Path ExistingModule() {
  return base::Path(
#if defined(OS_LINUX)
      "libc.so"
#elif defined(OS_WIN)
      "kernel32.dll"
#elif defined(OS_MAC)
      "/usr/lib/libSystem.B.dylib"
#endif
  );
}

constexpr const char* kExportedSymbol =
#if defined(OS_WIN)
    "VirtualAlloc";
#else
    "malloc";
#endif

TEST(DynamicLibrary, EmptyLoad) {
  const base::DynamicLibrary lib;
  ASSERT_FALSE(lib.loaded());
}

TEST(DynamicLibrary, Load) {
  const base::DynamicLibrary lib(
#if defined(OS_LINUX)
      "libstdc++.so"
#elif defined(OS_WIN)
      "iphlpapi.dll"
#elif defined(OS_MAC)
      "/usr/lib/libSystem.B.dylib"
#endif
  );
  ASSERT_TRUE(lib.loaded());
}

TEST(DynamicLibrary, LoadExisting) {
  const base::DynamicLibrary lib(ExistingModule());
  ASSERT_TRUE(lib.loaded());
}

TEST(DynamicLibrary, LoadExistingResolvesSymbols) {
  // The Linux path used to store dl_phdr_info::dlpi_addr -- a load bias, not a
  // handle -- and then hand it to dlsym and dlclose. A symbol lookup is what
  // separates a real handle from a link-map address; loaded() alone proved
  // nothing.
  base::DynamicLibrary lib;
  ASSERT_TRUE(lib.LoadExisting(ExistingModule()));
  EXPECT_NE(lib.FindSymbolPointer(kExportedSymbol), nullptr);
}

TEST(DynamicLibrary, LoadExistingLeavesTheModuleBehind) {
  {
    base::DynamicLibrary lib;
    ASSERT_TRUE(lib.LoadExisting(ExistingModule()));
  }
  // Destruction returns at most the reference this object took, never one that
  // belongs to whoever mapped the module.
  base::DynamicLibrary again;
  ASSERT_TRUE(again.LoadExisting(ExistingModule()));
  EXPECT_NE(again.FindSymbolPointer(kExportedSymbol), nullptr);
}

TEST(DynamicLibrary, FreeInvalidatesTheHandleItReturns) {
  base::DynamicLibrary lib;
  ASSERT_TRUE(lib.Load(ExistingModule(), /*should_free=*/true));
  ASSERT_TRUE(lib.loaded());
  EXPECT_TRUE(lib.Free());

  // The handle is gone, so no symbol can be looked up through it and a second
  // Free has nothing left to hand back.
  EXPECT_FALSE(lib.loaded());
  EXPECT_EQ(lib.FindSymbolPointer(kExportedSymbol), nullptr);
  EXPECT_FALSE(lib.Free());
}

TEST(DynamicLibrary, FreeRefusesAHandleItDoesNotOwn) {
  base::DynamicLibrary lib;
  ASSERT_TRUE(lib.Load(ExistingModule(), /*should_free=*/false));
  ASSERT_TRUE(lib.loaded());
  // Nothing was taken, so there is nothing to give back. Windows in particular
  // must not reach FreeLibrary here: GetModuleHandleW carries no reference, and
  // releasing it would decrement one that belongs to someone else.
  EXPECT_FALSE(lib.Free());
  EXPECT_TRUE(lib.loaded());
  EXPECT_NE(lib.FindSymbolPointer(kExportedSymbol), nullptr);
}

}  // namespace
