// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "registry_key.h"

#include <Windows.h>
#include <base/check.h>

extern "C" __declspec(dllimport) DWORD WINAPI ExpandEnvironmentStringsW(LPCWSTR lpSrc,
                                                                        LPWSTR lpDst,
                                                                        DWORD nSize);

namespace base::win {
namespace {
// RegEnumValue() reports the number of characters from the name that were
// written to the buffer, not how many there are. This constant is the maximum
// name size, such that a buffer with this size should read any name.
constexpr DWORD MAX_REGISTRY_NAME_SIZE = 16384;
// Registry values are read as BYTE* but can have wchar_t* data whose last
// wchar_t is truncated. This function converts the reported |byte_size| to
// a size in wchar_t that can store a truncated wchar_t if necessary.
inline DWORD to_wchar_size(DWORD byte_size) {
  return (byte_size + sizeof(wchar_t) - 1) / sizeof(wchar_t);
}

// Mask to pull WOW64 access flags out of REGSAM access.
constexpr REGSAM kWow64AccessMask = KEY_WOW64_32KEY | KEY_WOW64_64KEY;
constexpr DWORD kInvalidIterValue = static_cast<DWORD>(-1);

#define ERROR_CANTREAD 1012L
#define ERROR_DIR_NOT_EMPTY 145L
}  // namespace

RegistryKey::RegistryKey() = default;
RegistryKey::RegistryKey(HKEY key) : key_(key) {}

RegistryKey::RegistryKey(HKEY rootkey, const wchar_t* sub_key, REGSAM access) {
  if (rootkey) {
    if (access & (KEY_SET_VALUE | KEY_CREATE_SUB_KEY | KEY_CREATE_LINK))
      Create(rootkey, sub_key, access);
    else
      Open(rootkey, sub_key, access);
  } else {
    DCHECK(!sub_key);
    wow64access_ = access & kWow64AccessMask;
  }
}

RegistryKey::~RegistryKey() {
  Close();
}

LONG RegistryKey::Create(HKEY root_key, const wchar_t* sub_key, REGSAM access) {
  DWORD disposition_value;
  return CreateWithDisposition(root_key, sub_key, &disposition_value, access);
}

LONG RegistryKey::CreateWithDisposition(HKEY rootkey,
                                        const wchar_t* subkey,
                                        DWORD* disposition,
                                        REGSAM access) {
  DCHECK(rootkey && subkey && access && disposition);
  HKEY subhkey = nullptr;
  LONG result = ::RegCreateKeyExW(rootkey, subkey, 0, nullptr, REG_OPTION_NON_VOLATILE,
                                  access, nullptr, &subhkey, disposition);
  if (result == ERROR_SUCCESS) {
    Close();
    key_ = subhkey;
    wow64access_ = access & kWow64AccessMask;
  }

  return result;
}

LONG RegistryKey::CreateKey(const wchar_t* name, REGSAM access) {
  DCHECK(name && access);
  // After the application has accessed an alternate registry view using one of
  // the [KEY_WOW64_32KEY / KEY_WOW64_64KEY] flags, all subsequent operations
  // (create, delete, or open) on child registry keys must explicitly use the
  // same flag. Otherwise, there can be unexpected behavior.
  // http://msdn.microsoft.com/en-us/library/windows/desktop/aa384129.aspx.
  if ((access & kWow64AccessMask) != wow64access_) {
    IMPOSSIBLE;
    return 87L;
  }
  HKEY subkey = nullptr;
  LONG result = ::RegCreateKeyExW(key_, name, 0, nullptr, REG_OPTION_NON_VOLATILE, access,
                                  nullptr, &subkey, nullptr);
  if (result == ERROR_SUCCESS) {
    Close();
    key_ = subkey;
    wow64access_ = access & kWow64AccessMask;
  }

  return result;
}

LONG RegistryKey::Open(HKEY rootkey, const wchar_t* subkey, REGSAM access) {
  DCHECK(rootkey && subkey && access);
  HKEY subhkey = nullptr;

  LONG result = ::RegOpenKeyExW(rootkey, subkey, 0, access, &subhkey);
  if (result == ERROR_SUCCESS) {
    Close();
    key_ = subhkey;
    wow64access_ = access & kWow64AccessMask;
  }

  return result;
}

LONG RegistryKey::OpenKey(const wchar_t* relative_key_name, REGSAM access) {
  DCHECK(relative_key_name && access);
  // After the application has accessed an alternate registry view using one of
  // the [KEY_WOW64_32KEY / KEY_WOW64_64KEY] flags, all subsequent operations
  // (create, delete, or open) on child registry keys must explicitly use the
  // same flag. Otherwise, there can be unexpected behavior.
  // http://msdn.microsoft.com/en-us/library/windows/desktop/aa384129.aspx.
  if ((access & kWow64AccessMask) != wow64access_) {
    IMPOSSIBLE;
    return 87L;
  }
  HKEY subkey = nullptr;
  LONG result = ::RegOpenKeyExW(key_, relative_key_name, 0, access, &subkey);

  // We have to close the current opened key before replacing it with the new
  // one.
  if (result == ERROR_SUCCESS) {
    Close();
    key_ = subkey;
    wow64access_ = access & kWow64AccessMask;
  }
  return result;
}

void RegistryKey::Close() {
  if (key_) {
    ::RegCloseKey(key_);
    key_ = nullptr;
    wow64access_ = 0;
  }
}

bool RegistryKey::HasValue(const wchar_t* name) const {
  return ::RegQueryValueExW(key_, name, nullptr, nullptr, nullptr, nullptr) ==
         ERROR_SUCCESS;
}

DWORD RegistryKey::GetValueCount() const {
  DWORD count = 0;
  LONG result = ::RegQueryInfoKeyW(key_, nullptr, nullptr, nullptr, nullptr, nullptr,
                                   nullptr, &count, nullptr, nullptr, nullptr, nullptr);
  return (result == ERROR_SUCCESS) ? count : 0;
}

FILETIME RegistryKey::GetLastWriteTime() const {
  FILETIME last_write_time;
  LONG result =
      ::RegQueryInfoKeyW(key_, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                         nullptr, nullptr, nullptr, nullptr, &last_write_time);
  return (result == ERROR_SUCCESS) ? last_write_time : FILETIME{};
}

LONG RegistryKey::GetValueNameAt(DWORD index, base::StringW& name) const {
  wchar_t buf[256];
  DWORD bufsize = sizeof(buf) / sizeof(wchar_t);
  LONG r =
      ::RegEnumValueW(key_, index, buf, &bufsize, nullptr, nullptr, nullptr, nullptr);
  if (r == ERROR_SUCCESS)
    name.assign(buf, bufsize);

  return r;
}

LONG RegistryKey::DeleteKey(const wchar_t* name) {
  DCHECK(key_);
  DCHECK(name);
  HKEY subkey = nullptr;

  // Verify the key exists before attempting delete to replicate previous
  // behavior.
  LONG result = ::RegOpenKeyExW(key_, name, 0, READ_CONTROL | wow64access_, &subkey);
  if (result != ERROR_SUCCESS)
    return result;
  RegCloseKey(subkey);

  return RegDelRecurse(key_, name, wow64access_);
}

LONG RegistryKey::DeleteEmptyKey(const wchar_t* name) {
  DCHECK(key_);
  DCHECK(name);

  HKEY target_key = nullptr;
  LONG result = ::RegOpenKeyExW(key_, name, 0, KEY_READ | wow64access_, &target_key);

  if (result != ERROR_SUCCESS)
    return result;

  DWORD count = 0;
  result = ::RegQueryInfoKeyW(target_key, nullptr, nullptr, nullptr, nullptr, nullptr,
                              nullptr, &count, nullptr, nullptr, nullptr, nullptr);

  ::RegCloseKey(target_key);

  if (result != ERROR_SUCCESS)
    return result;

  if (count == 0)
    return ::RegDeleteKeyExW(key_, name, wow64access_, 0);

  return ERROR_DIR_NOT_EMPTY;
}

LONG RegistryKey::DeleteValue(const wchar_t* value_name) {
  DCHECK(key_);
  LONG result = ::RegDeleteValueW(key_, value_name);
  return result;
}

LONG RegistryKey::ReadValueDW(const wchar_t* name, DWORD& out_value) const {
  DWORD type = REG_DWORD;
  DWORD size = sizeof(DWORD);
  DWORD local_value = 0;
  LONG result = ReadValue(name, &local_value, size, type);
  if (result == ERROR_SUCCESS) {
    if ((type == REG_DWORD || type == REG_BINARY) && size == sizeof(DWORD))
      out_value = local_value;
    else
      result = ERROR_CANTREAD;
  }

  return result;
}

LONG RegistryKey::ReadI64(const wchar_t* name, i64& out_value) const {
  DWORD type = REG_QWORD;
  i64 local_value = 0;
  DWORD size = sizeof(local_value);
  LONG result = ReadValue(name, &local_value, size, type);
  if (result == ERROR_SUCCESS) {
    if ((type == REG_QWORD || type == REG_BINARY) && size == sizeof(local_value))
      out_value = local_value;
    else
      result = ERROR_CANTREAD;
  }

  return result;
}

LONG RegistryKey::ReadValue(const wchar_t* name, base::StringW& out_value) const {
  const size_t kMaxStringLength = 1024;  // This is after expansion.
  // Use the one of the other forms of ReadValue if 1024 is too small for you.
  wchar_t raw_value[kMaxStringLength];
  DWORD type = REG_SZ, size = sizeof(raw_value);
  LONG result = ReadValue(name, raw_value, size, type);
  if (result == ERROR_SUCCESS) {
    if (type == REG_SZ) {
      out_value = raw_value;
    } else if (type == REG_EXPAND_SZ) {
      wchar_t expanded[kMaxStringLength];
      size = ExpandEnvironmentStringsW(raw_value, expanded, kMaxStringLength);
      // Success: returns the number of wchar_t's copied
      // Fail: buffer too small, returns the size required
      // Fail: other, returns 0
      if (size == 0 || size > kMaxStringLength) {
        result = ERROR_MORE_DATA;
      } else {
        out_value = expanded;
      }
    } else {
      // Not a string. Oops.
      result = ERROR_CANTREAD;
    }
  }

  return result;
}

LONG RegistryKey::ReadValue(const wchar_t* name,
                            void* data,
                            DWORD& dsize,
                            DWORD& dtype) const {
  LONG result = ::RegQueryValueExW(key_, name, nullptr, &dtype,
                                   reinterpret_cast<LPBYTE>(data), &dsize);
  return result;
}

LONG RegistryKey::WriteValue(const wchar_t* name, const DWORD in_value) {
  return WriteValue(name, &in_value, static_cast<DWORD>(sizeof(in_value)), REG_DWORD);
}

LONG RegistryKey::WriteValue(const wchar_t* name, const base::StringRefW string_ref) {
  return WriteValue(name, string_ref.c_str(),
                    static_cast<DWORD>(sizeof(wchar_t) * (string_ref.length() + 1)),
                    REG_SZ);
}

LONG RegistryKey::WriteValue(const wchar_t* name,
                             const void* data,
                             DWORD dsize,
                             DWORD dtype) {
  DCHECK(data || !dsize);

  LONG result = ::RegSetValueExW(
      key_, name, 0, dtype, reinterpret_cast<LPBYTE>(const_cast<void*>(data)), dsize);
  return result;
}

// static
LONG RegistryKey::RegDelRecurse(HKEY root_key, const wchar_t* name, REGSAM access) {
  // First, see if the key can be deleted without having to recurse.
  LONG result = RegDeleteKeyExW(root_key, name, access, 0);
  if (result == ERROR_SUCCESS)
    return result;

  HKEY target_key = nullptr;
  result =
      ::RegOpenKeyExW(root_key, name, 0, KEY_ENUMERATE_SUB_KEYS | access, &target_key);

  if (result == ERROR_FILE_NOT_FOUND)
    return ERROR_SUCCESS;
  if (result != ERROR_SUCCESS)
    return result;

  base::StringW subkey_name(name);

  // Check for an ending slash and add one if it is missing.
  if (!subkey_name.empty() && subkey_name.back() != '\\')
    subkey_name.push_back('\\');

  // Enumerate the keys
  result = ERROR_SUCCESS;
  const DWORD kMaxKeyNameLength = MAX_PATH;
  const size_t base_key_length = subkey_name.length();

  base::StringW key_name;
  while (result == ERROR_SUCCESS) {
    DWORD key_size = kMaxKeyNameLength;

    key_name.reserve(kMaxKeyNameLength);
    key_name.resize(kMaxKeyNameLength - 1);

    result = ::RegEnumKeyExW(target_key, 0, key_name.data(), &key_size, nullptr, nullptr,
                             nullptr, nullptr);

    if (result != ERROR_SUCCESS)
      break;

    key_name.resize(key_size);
    subkey_name.resize(base_key_length);
    subkey_name += key_name;

    if (RegDelRecurse(root_key, subkey_name.c_str(), access) != ERROR_SUCCESS)
      break;
  }

  ::RegCloseKey(target_key);

  // Try again to delete the key.
  result = ::RegDeleteKeyExW(root_key, name, access, 0);

  return result;
}
}  // namespace base::win