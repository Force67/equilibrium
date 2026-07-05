// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Time utilities.
#pragma once

#include <base/arch.h>
#include <base/export.h>

namespace base {

using time_type = i64;

// A signed duration with microsecond precision. Create through the
// Microseconds()/Milliseconds()/Seconds()/Minutes() factories.
class TimeDelta {
 public:
  constexpr TimeDelta() : us_(0) {}
  explicit constexpr TimeDelta(i64 us) : us_(us) {}

  constexpr i64 InMicroseconds() const { return us_; }
  constexpr i64 InMilliseconds() const { return us_ / 1000; }
  constexpr i64 InSeconds() const { return us_ / 1000000; }
  constexpr f64 InSecondsF() const { return static_cast<f64>(us_) / 1000000.0; }

  constexpr bool is_zero() const { return us_ == 0; }

  constexpr TimeDelta operator+(const TimeDelta& other) const {
    return TimeDelta(us_ + other.us_);
  }
  constexpr TimeDelta operator-(const TimeDelta& other) const {
    return TimeDelta(us_ - other.us_);
  }
  constexpr TimeDelta& operator+=(const TimeDelta& other) {
    us_ += other.us_;
    return *this;
  }
  constexpr TimeDelta& operator-=(const TimeDelta& other) {
    us_ -= other.us_;
    return *this;
  }
  constexpr TimeDelta operator-() const { return TimeDelta(-us_); }

  constexpr bool operator==(const TimeDelta& other) const { return us_ == other.us_; }
  constexpr bool operator!=(const TimeDelta& other) const { return us_ != other.us_; }
  constexpr bool operator<(const TimeDelta& other) const { return us_ < other.us_; }
  constexpr bool operator<=(const TimeDelta& other) const { return us_ <= other.us_; }
  constexpr bool operator>(const TimeDelta& other) const { return us_ > other.us_; }
  constexpr bool operator>=(const TimeDelta& other) const { return us_ >= other.us_; }

 private:
  // duration in micro seconds
  i64 us_;
};

constexpr TimeDelta Microseconds(i64 us) {
  return TimeDelta(us);
}
constexpr TimeDelta Milliseconds(i64 ms) {
  return TimeDelta(ms * 1000);
}
constexpr TimeDelta Seconds(i64 s) {
  return TimeDelta(s * 1000000);
}
constexpr TimeDelta Minutes(i64 m) {
  return TimeDelta(m * 60000000);
}

// A wall clock reading, stored as microseconds since the Unix epoch
// (January 1, 1970 12:00am UTC). Default constructed Time is the epoch.
class BASE_EXPORT Time {
 public:
  constexpr Time() : us_(0) {}

  // Returns the current time. Watch out, the system might adjust its clock
  // in which case time will actually go backwards. We don't guarantee that
  // times are increasing, or that two calls to Now() won't be the same.
  // Use TimeTicks to measure elapsed time.
  static Time Now();

  // Returns the current time. Same as Now() except that this function always
  // uses system time so that there are no discrepancies between the returned
  // time and system time even on virtual environments including our test bot.
  // For timing sensitive unittests, this function should be used.
  static Time NowFromSystemTime();

  constexpr TimeDelta operator-(const Time& other) const {
    return TimeDelta(us_ - other.us_);
  }
  constexpr Time operator+(const TimeDelta& delta) const {
    return Time(us_ + delta.InMicroseconds());
  }
  constexpr Time operator-(const TimeDelta& delta) const {
    return Time(us_ - delta.InMicroseconds());
  }

  // Comparison operators
  constexpr bool operator==(const Time& other) const { return us_ == other.us_; }
  constexpr bool operator!=(const Time& other) const { return us_ != other.us_; }
  constexpr bool operator<(const Time& other) const { return us_ < other.us_; }
  constexpr bool operator<=(const Time& other) const { return us_ <= other.us_; }
  constexpr bool operator>(const Time& other) const { return us_ > other.us_; }
  constexpr bool operator>=(const Time& other) const { return us_ >= other.us_; }

 private:
  explicit constexpr Time(i64 us) : us_(us) {}

  // time in micro seconds
  i64 us_;
};

// A monotonic clock reading. Unlike Time it never goes backwards, but it has
// no relation to wall clock time. Only meaningful when compared against other
// TimeTicks from the same process; use it for measuring elapsed time and for
// timeouts.
class BASE_EXPORT TimeTicks {
 public:
  constexpr TimeTicks() : us_(0) {}

  // Returns the current monotonic tick count.
  static TimeTicks Now();

  constexpr bool is_null() const { return us_ == 0; }

  constexpr TimeDelta operator-(const TimeTicks& other) const {
    return TimeDelta(us_ - other.us_);
  }
  constexpr TimeTicks operator+(const TimeDelta& delta) const {
    return TimeTicks(us_ + delta.InMicroseconds());
  }
  constexpr TimeTicks operator-(const TimeDelta& delta) const {
    return TimeTicks(us_ - delta.InMicroseconds());
  }

  // Comparison operators
  constexpr bool operator==(const TimeTicks& other) const { return us_ == other.us_; }
  constexpr bool operator!=(const TimeTicks& other) const { return us_ != other.us_; }
  constexpr bool operator<(const TimeTicks& other) const { return us_ < other.us_; }
  constexpr bool operator<=(const TimeTicks& other) const { return us_ <= other.us_; }
  constexpr bool operator>(const TimeTicks& other) const { return us_ > other.us_; }
  constexpr bool operator>=(const TimeTicks& other) const { return us_ >= other.us_; }

 private:
  explicit constexpr TimeTicks(i64 us) : us_(us) {}

  // ticks in micro seconds
  i64 us_;
};

// Seconds since the Unix epoch. Moves with wall-clock adjustments.
BASE_EXPORT i64 GetUnixTimeStamp();

// Milliseconds since the Unix epoch. Same caveat as GetUnixTimeStamp.
BASE_EXPORT i64 GetUnixTimeMilliseconds();

// Monotonic tick source. The reference point is implementation-defined,
// only differences between samples are meaningful.
class BASE_EXPORT TickClock {
 public:
  static i64 NowNs();
};
}  // namespace base
