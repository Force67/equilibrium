// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace base {

template <class TLock>
class ScopedLockGuard {
 public:
  inline ScopedLockGuard() { lock_.Acquire(); }
  inline ~ScopedLockGuard() { lock_.Release(); }
 private:
  TLock lock_;
};
}  // namespace base
