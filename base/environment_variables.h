// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/strings/string_ref.h>

#if defined(SetEnvironmentVariable)
#undef SetEnvironmentVariable
#endif

#if defined(GetEnvironmentVariable)
#undef GetEnvironmentVariable
#endif

namespace base {

bool GetEnvironmentVariable(const base::StringRefU8 variable_name, base::StringU8& out);
bool SetEnvironmentVariable(const base::StringRefU8 name, const base::StringRefU8 value);

bool DeleteEnvironmentVariable(const base::StringRefU8 variable_name);
}  // namespace base