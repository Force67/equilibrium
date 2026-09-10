// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <Windows.h>
#include <base/strings/xstring.h>
#include <base/win/registry_key.h>

namespace base::win {
namespace {

// The string overload of ReadValue reads into a 1024 wchar_t buffer.
constexpr DWORD kReadBufferLength = 1024;
constexpr const wchar_t* kTestKey = L"Software\\Equilibrium\\RegistryKeyTest";

class RegistryKeyStringTest : public ::testing::Test {
 protected:
  RegistryKey key;

  void SetUp() override {
    ASSERT_EQ(key.Create(HKEY_CURRENT_USER, kTestKey, KEY_READ | KEY_WRITE),
              ERROR_SUCCESS);
  }

  void TearDown() override {
    key.Close();
    RegistryKey parent(HKEY_CURRENT_USER, L"Software\\Equilibrium",
                       KEY_READ | KEY_WRITE);
    if (parent.good())
      parent.DeleteKey(L"RegistryKeyTest");
  }

  // Writes |name| as a REG_SZ of exactly |length| characters, with no
  // terminator stored -- which is what RegQueryValueExW is allowed to hand
  // back without one.
  void WriteUnterminated(const wchar_t* name, DWORD length, wchar_t fill) {
    base::StringW value(length, fill);
    ASSERT_EQ(key.WriteValue(name, value.data(),
                             static_cast<DWORD>(length * sizeof(wchar_t)), REG_SZ),
              ERROR_SUCCESS);
  }
};

TEST_F(RegistryKeyStringTest, ReadsAShortTerminatedString) {
  ASSERT_EQ(key.WriteValue(L"short", base::StringRefW(L"hello")), ERROR_SUCCESS);
  base::StringW read;
  ASSERT_EQ(key.ReadValue(L"short", read), ERROR_SUCCESS);
  EXPECT_EQ(read.size(), 5u);
  EXPECT_EQ(read, base::StringW(L"hello"));
}

TEST_F(RegistryKeyStringTest, ReadsAnEmptyString) {
  // A zero-length REG_SZ leaves the stack buffer untouched. Deriving the
  // length from the returned byte count is what keeps this from walking
  // uninitialized stack looking for a terminator.
  ASSERT_EQ(key.WriteValue(L"empty", nullptr, 0, REG_SZ), ERROR_SUCCESS);
  base::StringW read;
  ASSERT_EQ(key.ReadValue(L"empty", read), ERROR_SUCCESS);
  EXPECT_TRUE(read.empty());
}

TEST_F(RegistryKeyStringTest, ReadsAValueThatExactlyFillsTheBuffer) {
  // The regression: a REG_SZ of exactly the buffer length comes back with no
  // terminator, and constructing a string from the raw array then read past
  // the end of the stack buffer.
  ASSERT_NO_FATAL_FAILURE(WriteUnterminated(L"exact", kReadBufferLength, L'a'));

  base::StringW read;
  ASSERT_EQ(key.ReadValue(L"exact", read), ERROR_SUCCESS);
  EXPECT_EQ(read.size(), kReadBufferLength);
  for (mem_size i = 0; i < read.size(); ++i)
    ASSERT_EQ(read[i], L'a') << "at " << i;
}

TEST_F(RegistryKeyStringTest, TruncatesAValueLongerThanTheBuffer) {
  ASSERT_NO_FATAL_FAILURE(
      WriteUnterminated(L"toolong", kReadBufferLength + 64, L'b'));

  base::StringW read;
  // ERROR_MORE_DATA is what RegQueryValueExW reports; either way nothing may
  // be read past the buffer.
  const LONG result = key.ReadValue(L"toolong", read);
  if (result == ERROR_SUCCESS) {
    EXPECT_LE(read.size(), kReadBufferLength);
    for (mem_size i = 0; i < read.size(); ++i)
      ASSERT_EQ(read[i], L'b') << "at " << i;
  } else {
    EXPECT_EQ(result, ERROR_MORE_DATA);
  }
}

TEST_F(RegistryKeyStringTest, ReadsAnUnterminatedExpandString) {
  // The same buffer feeds ExpandEnvironmentStringsW, which needs a terminated
  // source string.
  base::StringW value(kReadBufferLength, L'c');
  ASSERT_EQ(key.WriteValue(L"expand", value.data(),
                           static_cast<DWORD>(kReadBufferLength * sizeof(wchar_t)),
                           REG_EXPAND_SZ),
            ERROR_SUCCESS);

  base::StringW read;
  const LONG result = key.ReadValue(L"expand", read);
  // Expansion of 1024 characters needs a terminator slot beyond them, so this
  // legitimately reports ERROR_MORE_DATA. What matters is that it reports
  // rather than reading past the buffer.
  if (result == ERROR_SUCCESS) {
    EXPECT_EQ(read.size(), kReadBufferLength);
  } else {
    EXPECT_EQ(result, ERROR_MORE_DATA);
  }
}

TEST_F(RegistryKeyStringTest, RejectsANonStringValue) {
  ASSERT_EQ(key.WriteValue(L"number", DWORD(42)), ERROR_SUCCESS);
  base::StringW read;
  EXPECT_NE(key.ReadValue(L"number", read), ERROR_SUCCESS);
}

}  // namespace
}  // namespace base::win
