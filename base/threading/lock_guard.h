// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace base {

template <class T>
class ScopedLockGuard {
 public:
  ScopedLockGuard(T& t) : lock_(t) {}

  inline ScopedLockGuard() { lock_.Acquire(); }
  inline ~ScopedLockGuard() { lock_.Release(); }

 private:
  T lock_;
};

template <class T>
class NonOwningScopedLockGuard {
 public:
  inline NonOwningScopedLockGuard(T& l) : lock_(l) { l.Acquire(); }
  inline ~NonOwningScopedLockGuard() { lock_.Release(); }

 private:
  T& lock_;
};
}  // namespace base
