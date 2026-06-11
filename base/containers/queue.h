// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// base::Queue - FIFO adapter over SimpleDeque, std::queue-shaped.
#pragma once

#include <base/arch.h>
#include <base/containers/deque.h>

namespace base {

template <typename T, class TAllocator = DefaultAllocator>
class Queue {
 public:
  void push(const T& value) { deque_.push_back(value); }
  void pop() { deque_.pop_front(); }

  T& front() { return deque_.front(); }
  const T& front() const { return deque_.front(); }
  T& back() { return deque_.back(); }
  const T& back() const { return deque_.back(); }

  bool empty() const { return deque_.empty(); }
  mem_size size() const { return deque_.size(); }

 private:
  SimpleDeque<T, TAllocator> deque_;
};

}  // namespace base
