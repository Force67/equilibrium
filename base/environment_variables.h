// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/export.h>
#include <base/strings/string_ref.h>

#if defined(SetEnvironmentVariable)
#undef SetEnvironmentVariable
#endif

#if defined(GetEnvironmentVariable)
#undef GetEnvironmentVariable
#endif

namespace base {

BASE_EXPORT bool GetEnvironmentVariable(const base::StringRefU8 variable_name,
                                        base::StringU8& out);
BASE_EXPORT bool SetEnvironmentVariable(const base::StringRefU8 name,
                                        const base::StringRefU8 value);

BASE_EXPORT bool DeleteEnvironmentVariable(const base::StringRefU8 variable_name);
}  // namespace base