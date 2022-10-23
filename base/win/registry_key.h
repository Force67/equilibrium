// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

// adapted from:
// https://source.chromium.org/chromium/chromium/src/+/main:base/win/registry.h

#include <base/arch.h>
#include <base/export.h>
#include <base/win/minwin.h>
#include <base/memory/move.h>
#include <base/strings/string_ref.h>

namespace base::win {

class BASE_EXPORT RegistryKey {
 public:
  RegistryKey();
  explicit RegistryKey(HKEY key);

  // open immedeatly
  RegistryKey(HKEY rootkey, const wchar_t* subkey, REGSAM access);

  BASE_NOCOPYMOVE(RegistryKey);

  ~RegistryKey();

  // key getter
  HKEY hkey() const noexcept { return key_; }

  bool good() const noexcept { return key_ != nullptr; }
  operator bool() const noexcept { return key_ != nullptr; }

  LONG Create(HKEY root_key, const wchar_t* sub_key, REGSAM access);
  LONG CreateWithDisposition(HKEY root_key,
                             const wchar_t* sub_key,
                             DWORD* disposition,
                             REGSAM access);

  // Creates a subkey or open it if it already exists.
  LONG CreateKey(const wchar_t* name, REGSAM access);

  // Opens an existing reg key.
  LONG Open(HKEY rootkey, const wchar_t* sub_key, REGSAM access);

  // Opens an existing reg key, given the relative key name.
  LONG OpenKey(const wchar_t* relative_key_name, REGSAM access);

  // Closes this reg key.
  void Close();

  // Returns false if this key does not have the specified value, or if an error
  // occurrs while attempting to access it.
  bool HasValue(const wchar_t* value_name) const;

  // Returns the number of values for this key, or 0 if the number cannot be
  // determined.
  DWORD GetValueCount() const;

  // Returns the last write time or 0 on failure.
  FILETIME GetLastWriteTime() const;

  // Determines the nth value's name.
  LONG GetValueNameAt(DWORD index, base::StringW& name) const;

  // Kills a key and everything that lives below it; please be careful when
  // using it.
  LONG DeleteKey(const wchar_t* name);

  // Deletes an empty subkey.  If the subkey has subkeys or values then this
  // will fail.
  LONG DeleteEmptyKey(const wchar_t* name);

  // Deletes a single value within the key.
  LONG DeleteValue(const wchar_t* name);

  LONG ReadValueDW(const wchar_t* name, DWORD& out_value) const;
  LONG ReadI64(const wchar_t* name, i64& out_value) const;
  LONG ReadValue(const wchar_t* name, base::StringW& out_value) const;
  LONG ReadValue(const wchar_t* name, void* data, DWORD& dsize, DWORD& dtype) const;

  LONG WriteValue(const wchar_t* name, const DWORD value);
  LONG WriteValue(const wchar_t* name, const base::StringRefW string_ref);
  LONG WriteValue(const wchar_t* name, const void* data, DWORD dsize, DWORD dtype);

 private:
  class Watcher;

  // Recursively deletes a key and all of its subkeys.
  static LONG RegDelRecurse(HKEY root_key, const wchar_t* name, REGSAM access);

  HKEY key_ = nullptr;  // The registry key being iterated.
  REGSAM wow64access_ = 0;
};
}  // namespace base::win