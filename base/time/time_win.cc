// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Time implementation for windows.

#include <base/check.h>
#include <base/time/time.h>
#include <base/win/minwin.h>

namespace wintypes {
union LARGE_INTEGER {
  struct {
    DWORD LowPart;
    LONG HighPart;
  } DUMMYSTRUCTNAME;
  struct {
    DWORD LowPart;
    LONG HighPart;
  } u;
  LONGLONG QuadPart;
};

struct FILETIME {
  DWORD dwLowDateTime;
  DWORD dwHighDateTime;
};

extern "C" __declspec(dllimport) void GetSystemTimeAsFileTime(wintypes::FILETIME*);
}  // namespace wintypes

namespace base {
#if 0
namespace {
i64 initial_time = 0;
constexpr TimeDelta kMaxTimeToAvoidDrift = Seconds(60);

void InitializeClock() {
  g_initial_ticks = subtle::TimeTicksNowIgnoringOverride();
  initial_time = CurrentWallclockMicroseconds();
}

}  // namespace

Time TimeNowIgnoringOverride() {
  if (initial_time == 0)
    InitializeClock();

  // We implement time using the high-resolution timers so that we can get
  // timeouts which are smaller than 10-15ms.  If we just used
  // CurrentWallclockMicroseconds(), we'd have the less-granular timer.
  //
  // To make this work, we initialize the clock (g_initial_time) and the
  // counter (initial_ctr).  To compute the initial time, we can check
  // the number of ticks that have elapsed, and compute the delta.
  //
  // To avoid any drift, we periodically resync the counters to the system
  // clock.
  while (true) {
    TimeTicks ticks = TimeTicksNowIgnoringOverride();

    // Calculate the time elapsed since we started our timer
    TimeDelta elapsed = ticks - g_initial_ticks;

    // Check if enough time has elapsed that we need to resync the clock.
    if (elapsed > kMaxTimeToAvoidDrift) {
      InitializeClock();
      continue;
    }

    return Time() + elapsed + Microseconds(g_initial_time);
  }
}

Time TimeNowFromSystemTimeIgnoringOverride() {
  // Force resync.
  InitializeClock();
  return Time() + Microseconds(g_initial_time);
}
#endif

i64 GetUnixTimeStamp() {
  const i64 kUnixTimeBase =
      0x019DB1DED53E8000;  // January 1, 1970 (start of Unix epoch) in "ticks"
  const i64 kTicksPerSecond = 10000000;  // a tick is 100ns

  wintypes::FILETIME ft;
  GetSystemTimeAsFileTime(&ft);

  // Copy the low and high parts of FILETIME into a LARGE_INTEGER
  // This is so we can access the full 64-bits as an i64 without causing an
  // alignment fault
  wintypes::LARGE_INTEGER li;
  li.u.LowPart = ft.dwLowDateTime;
  li.u.HighPart = ft.dwHighDateTime;

  // Convert ticks since 1/1/1970 into seconds
  return (li.QuadPart - kUnixTimeBase) / kTicksPerSecond;
}

Time Time::Now() {
  IMPOSSIBLE;
  return {};
}

Time Time::NowFromSystemTime() {
  IMPOSSIBLE;
  return {};
}

}  // namespace base
