// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Time implementation for windows.

#include <base/check.h>
#include <base/time/time.h>
#include <ctime>
#include <sys/time.h>

namespace base {

i64 GetUnixTimeStamp() {
  struct timeval tv;
  ::gettimeofday(&tv, nullptr);
  return tv.tv_sec;
}

i64 GetUnixTimeMilliseconds() {
  struct timespec ts;
  ::clock_gettime(CLOCK_REALTIME, &ts);
  return static_cast<i64>(ts.tv_sec) * 1000 +
         static_cast<i64>(ts.tv_nsec) / 1000000;
}

i64 TickClock::NowNs() {
  struct timespec ts;
  ::clock_gettime(CLOCK_MONOTONIC, &ts);
  return static_cast<i64>(ts.tv_sec) * 1'000'000'000LL +
         static_cast<i64>(ts.tv_nsec);
}
}  // namespace base