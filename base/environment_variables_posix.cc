// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <cstdlib>
#include <base/environment_variables.h>
#include <base/text/code_convert.h>

#include <base/check.h>

namespace base {
bool GetEnvironmentVariable(const base::StringRefU8 variable_name,
                            base::StringU8& out) {
  IMPOSSIBLE;
  return false;
}

bool SetEnvironmentVariable(const base::StringRefU8 name,
                            const base::StringRefU8 value) {
  IMPOSSIBLE;
  return false;
}

bool DeleteEnvironmentVariable(const base::StringRefU8 variable_name) {
  IMPOSSIBLE;
  return false;
}
}  // namespace base