// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <cstdlib>
#include <base/environment_variables.h>
#include <base/text/code_convert.h>

#include <base/check.h>

namespace base {
bool GetEnvironmentVariable(const base::StringRefU8 variable_name,
                            base::StringU8& out) {
  const char* value = getenv(reinterpret_cast<const char*>(variable_name.c_str()));
  if (value) {
    out = base::StringU8(reinterpret_cast<const char8_t*>(value));
    return true;
  }
  return false;
}

bool SetEnvironmentVariable(const base::StringRefU8 name,
                            const base::StringRefU8 value) {
  return setenv(reinterpret_cast<const char*>(name.c_str()), reinterpret_cast<const char*>(value.c_str()), 1) == 0;
}

bool DeleteEnvironmentVariable(const base::StringRefU8 variable_name) {
  return unsetenv(reinterpret_cast<const char*>(variable_name.c_str())) == 0;
}
}  // namespace base