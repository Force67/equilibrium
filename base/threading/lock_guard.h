// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace base {

// --- Acquire/Release-style guards (for base::SpinningMutex) ---------------

template <class T>
class ScopedLockGuard {
 public:
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

// --- lock/unlock-style guards (for base::Mutex / base::SharedMutex) -------
// API-compatible with std::lock_guard / std::unique_lock / std::shared_lock.

struct try_to_lock_t {};
inline constexpr try_to_lock_t try_to_lock{};

template <class M>
class LockGuard {
 public:
  explicit LockGuard(M& m) : m_(m) { m_.lock(); }
  ~LockGuard() { m_.unlock(); }

  LockGuard(const LockGuard&) = delete;
  LockGuard& operator=(const LockGuard&) = delete;
  LockGuard(LockGuard&&) = delete;
  LockGuard& operator=(LockGuard&&) = delete;

 private:
  M& m_;
};

template <class M>
class UniqueLock {
 public:
  UniqueLock() = default;
  explicit UniqueLock(M& m) : m_(&m), owns_(true) { m_->lock(); }
  UniqueLock(M& m, try_to_lock_t) : m_(&m), owns_(m.try_lock()) {}

  ~UniqueLock() {
    if (owns_ && m_)
      m_->unlock();
  }

  UniqueLock(const UniqueLock&) = delete;
  UniqueLock& operator=(const UniqueLock&) = delete;

  UniqueLock(UniqueLock&& o) noexcept : m_(o.m_), owns_(o.owns_) {
    o.m_ = nullptr;
    o.owns_ = false;
  }
  UniqueLock& operator=(UniqueLock&& o) noexcept {
    if (this != &o) {
      if (owns_ && m_)
        m_->unlock();
      m_ = o.m_;
      owns_ = o.owns_;
      o.m_ = nullptr;
      o.owns_ = false;
    }
    return *this;
  }

  bool owns_lock() const { return owns_; }
  explicit operator bool() const { return owns_; }

 private:
  M* m_ = nullptr;
  bool owns_ = false;
};

template <class M>
class SharedLockGuard {
 public:
  explicit SharedLockGuard(M& m) : m_(m) { m_.lock_shared(); }
  ~SharedLockGuard() { m_.unlock_shared(); }

  SharedLockGuard(const SharedLockGuard&) = delete;
  SharedLockGuard& operator=(const SharedLockGuard&) = delete;

 private:
  M& m_;
};

}  // namespace base
