// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <Windows.h>
#include <base/environment_variables.h>
#include <base/text/code_convert.h>

namespace base {
bool GetEnvironmentVariable(const base::StringRefU8 variable_name, base::StringU8& out) {
  auto wide_name = base::UTF8ToWide(variable_name);

  DWORD buffer_size = ::GetEnvironmentVariableW(wide_name.c_str(), nullptr, 0);
  if (buffer_size == 0)
    return false;

  base::StringW wide_buffer;
  wide_buffer.resize(buffer_size);
  buffer_size = ::GetEnvironmentVariableW(wide_name.c_str(), wide_buffer.data(),
                                          static_cast<DWORD>(wide_buffer.size()));

  // yes, this is needed else we end up with a character too much, this isn't
  // ideal but we should use a raw buffer anyway in the future
  wide_buffer.resize(buffer_size);

  if (buffer_size > wide_buffer.length() /*account for nterm*/)
    return false;

  out = base::WideToUTF8(wide_buffer);
  return true;
}

bool SetEnvironmentVariable(const base::StringRefU8 name, const base::StringRefU8 value) {
  auto wide_name = base::UTF8ToWide(name);
  auto wide_value = base::UTF8ToWide(value);

  return ::SetEnvironmentVariableW(wide_name.c_str(), wide_value.c_str()) == TRUE;
}

bool DeleteEnvironmentVariable(const base::StringRefU8 variable_name) {
  // https://stackoverflow.com/questions/3205197/remove-environmental-variable-programmatically
  auto wide_name = base::UTF8ToWide(variable_name);
  return ::SetEnvironmentVariableW(wide_name.c_str(), nullptr) == TRUE;
}
}  // namespace base