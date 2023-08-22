// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Time implementation for windows.

#include <base/check.h>
#include <base/time/time.h>
#include <ctime>
#include <sys/time.h>

namespace base {

i64 GetUnixTimeStamp() {
  // best solution since this also the underlying syscall...
  struct timeval tv;
  ::gettimeofday(&tv, nullptr);
  return tv.tv_sec;  // Return the number of seconds since January 1, 1970
}
} // namespace base