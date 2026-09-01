// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Time implementation for windows.

#include <base/time/time.h>
#include <base/win/minwin.h>

#if defined(__MINGW32__) || defined(__MINGW64__)
// minwin.h defers to the real <windows.h> on mingw, so re-declaring these types
// and entry points would conflict with the ones already in scope. Adopt them.
namespace wintypes {
using ::FILETIME;
using ::LARGE_INTEGER;
using ::GetSystemTimeAsFileTime;
using ::QueryPerformanceCounter;
using ::QueryPerformanceFrequency;
}  // namespace wintypes
#else
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
extern "C" __declspec(dllimport) int __stdcall QueryPerformanceCounter(wintypes::LARGE_INTEGER*);
extern "C" __declspec(dllimport) int __stdcall QueryPerformanceFrequency(wintypes::LARGE_INTEGER*);
}  // namespace wintypes
#endif

namespace base {
namespace {
// 100-ns ticks since Unix epoch, read from FILETIME.
i64 FileTimeTicksSinceUnix() {
  const i64 kUnixTimeBase =
      0x019DB1DED53E8000;  // January 1, 1970 in 100-ns FILETIME ticks

  wintypes::FILETIME ft;
  wintypes::GetSystemTimeAsFileTime(&ft);

  // Copy the low and high parts of FILETIME into a LARGE_INTEGER
  // This is so we can access the full 64-bits as an i64 without causing an
  // alignment fault
  wintypes::LARGE_INTEGER li;
  li.u.LowPart = ft.dwLowDateTime;
  li.u.HighPart = static_cast<LONG>(ft.dwHighDateTime);

  return li.QuadPart - kUnixTimeBase;
}

i64 QpcTicksPerSecond() {
  static i64 freq = [] {
    wintypes::LARGE_INTEGER f;
    wintypes::QueryPerformanceFrequency(&f);
    return f.QuadPart;
  }();
  return freq;
}
}  // namespace

i64 GetUnixTimeStamp() {
  return FileTimeTicksSinceUnix() / 10'000'000LL;  // 100ns ticks -> seconds
}

i64 GetUnixTimeMilliseconds() {
  return FileTimeTicksSinceUnix() / 10'000LL;  // 100ns ticks -> ms
}

i64 TickClock::NowNs() {
  const i64 freq = QpcTicksPerSecond();
  wintypes::LARGE_INTEGER now;
  wintypes::QueryPerformanceCounter(&now);
  // ns = ticks * 1e9 / freq, staggered to avoid overflow on large counters.
  i64 seconds = now.QuadPart / freq;
  i64 remainder = now.QuadPart % freq;
  return seconds * 1'000'000'000LL + (remainder * 1'000'000'000LL) / freq;
}

Time Time::Now() {
  return Time(FileTimeTicksSinceUnix() / 10);  // 100ns ticks -> microseconds
}

Time Time::NowFromSystemTime() {
  return Now();
}

TimeTicks TimeTicks::Now() {
  const i64 freq = QpcTicksPerSecond();
  wintypes::LARGE_INTEGER now;
  wintypes::QueryPerformanceCounter(&now);
  // us = ticks * 1e6 / freq, staggered to avoid overflow on large counters.
  i64 seconds = now.QuadPart / freq;
  i64 remainder = now.QuadPart % freq;
  return TimeTicks(seconds * 1'000'000LL + (remainder * 1'000'000LL) / freq);
}
}  // namespace base
