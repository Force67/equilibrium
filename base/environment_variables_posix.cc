// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// getenv and setenv reach the same `environ` array this walks directly; going
// through it keeps <stdlib.h> out and costs nothing, since getenv is itself a
// linear scan. Writes still go through setenv/unsetenv, which own the storage
// they allocate for entries they added -- taking that over would mean
// duplicating an allocator libc already has.

#include <base/environment_variables.h>

#include <base/check.h>
#include <base/strings/char_algorithms.h>
#include <base/strings/string_compare.h>
#include <base/text/code_convert.h>

extern "C" {
extern char** environ;
int setenv(const char* name, const char* value, int overwrite);
int unsetenv(const char* name);
}

namespace base {
namespace {

// The entry for |name|, positioned just past its '=', or nullptr.
const char* FindInEnvironment(const char* name) noexcept {
  const mem_size length = CountStringLength(name);
  for (char** entry = environ; entry && *entry; ++entry) {
    if (Strncmp(*entry, name, length) == 0 && (*entry)[length] == '=')
      return *entry + length + 1;
  }
  return nullptr;
}

}  // namespace

bool GetEnvironmentVariable(const base::StringRefU8 variable_name, base::StringU8& out) {
  const char* value =
      FindInEnvironment(reinterpret_cast<const char*>(variable_name.c_str()));
  if (value) {
    out = base::StringU8(reinterpret_cast<const char8_t*>(value));
    return true;
  }
  return false;
}

bool SetEnvironmentVariable(const base::StringRefU8 name, const base::StringRefU8 value) {
  return ::setenv(reinterpret_cast<const char*>(name.c_str()),
                  reinterpret_cast<const char*>(value.c_str()), 1) == 0;
}

bool DeleteEnvironmentVariable(const base::StringRefU8 variable_name) {
  return ::unsetenv(reinterpret_cast<const char*>(variable_name.c_str())) == 0;
}
}  // namespace base
