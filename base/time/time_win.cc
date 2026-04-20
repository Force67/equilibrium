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

extern "C" __declspec(dllimport) void __stdcall GetSystemTimeAsFileTime(wintypes::FILETIME*);
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

namespace {
// 100-ns ticks since Unix epoch, read from FILETIME.
i64 FileTimeTicksSinceUnix() {
  const i64 kUnixTimeBase =
      0x019DB1DED53E8000;  // January 1, 1970 in 100-ns FILETIME ticks

  wintypes::FILETIME ft;
  wintypes::GetSystemTimeAsFileTime(&ft);

  wintypes::LARGE_INTEGER li;
  li.u.LowPart = ft.dwLowDateTime;
  li.u.HighPart = ft.dwHighDateTime;

  return li.QuadPart - kUnixTimeBase;
}
}  // namespace

i64 GetUnixTimeStamp() {
  return FileTimeTicksSinceUnix() / 10'000'000LL;  // 100ns ticks -> seconds
}

i64 GetUnixTimeMilliseconds() {
  return FileTimeTicksSinceUnix() / 10'000LL;     // 100ns ticks -> ms
}

extern "C" __declspec(dllimport) int __stdcall QueryPerformanceCounter(
    wintypes::LARGE_INTEGER*);
extern "C" __declspec(dllimport) int __stdcall QueryPerformanceFrequency(
    wintypes::LARGE_INTEGER*);

i64 TickClock::NowNs() {
  static i64 freq = [] {
    wintypes::LARGE_INTEGER f;
    QueryPerformanceFrequency(&f);
    return f.QuadPart;
  }();
  wintypes::LARGE_INTEGER now;
  QueryPerformanceCounter(&now);
  // ns = ticks * 1e9 / freq, staggered to avoid overflow on large counters.
  i64 seconds = now.QuadPart / freq;
  i64 remainder = now.QuadPart % freq;
  return seconds * 1'000'000'000LL + (remainder * 1'000'000'000LL) / freq;
}

Time Time::Now() {
  BASE_IMPOSSIBLE;
  return {};
}

Time Time::NowFromSystemTime() {
  BASE_IMPOSSIBLE;
  return {};
}

}  // namespace base
