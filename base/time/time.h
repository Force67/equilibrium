// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Time utilities.
#pragma once

#include <base/arch.h>
#include <base/export.h>

namespace base {

using time_type = i64;

class BASE_EXPORT Time {
 public:
  // explicit Time(i64 us);

  // Returns the current time. Watch out, the system might adjust its clock
  // in which case time will actually go backwards. We don't guarantee that
  // times are increasing, or that two calls to Now() won't be the same.
  static Time Now();

  // Returns the current time. Same as Now() except that this function always
  // uses system time so that there are no discrepancies between the returned
  // time and system time even on virtual environments including our test bot.
  // For timing sensitive unittests, this function should be used.
  static Time NowFromSystemTime();

  // Comparison operators
  constexpr bool operator==(const Time& other) const { return us_ == other.us_; }
  constexpr bool operator!=(const Time& other) const { return us_ != other.us_; }
  constexpr bool operator<(const Time& other) const { return us_ < other.us_; }
  constexpr bool operator<=(const Time& other) const { return us_ <= other.us_; }
  constexpr bool operator>(const Time& other) const { return us_ > other.us_; }
  constexpr bool operator>=(const Time& other) const { return us_ >= other.us_; }

 private:
  // time in micro seconds
  i64 us_;
};

// Get the number of seconds since January 1, 1970 12:00am UTC
BASE_EXPORT i64 GetUnixTimeStamp();

// Wall-clock time in milliseconds since the Unix epoch. Subject to system
// clock adjustments — use TickClock for measuring elapsed durations.
BASE_EXPORT i64 GetUnixTimeMilliseconds();

// Monotonic, high-resolution tick source for measuring elapsed time.
// Not affected by wall-clock adjustments. The reference point is
// implementation-defined — only differences between two samples are
// meaningful.
class BASE_EXPORT TickClock {
 public:
  // Current tick value in nanoseconds since the reference point.
  static i64 NowNs();
};
}  // namespace base
