// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

#include <base/arch.h>
#include <base/check.h>

namespace base {

// base::RingBuffer uses a fixed-size array, unlike base::circular_deque and
// std::deque, and so, one can access only the last |kSize| elements. Also, you
// can add elements to the front and read/modify random elements, but cannot
// remove elements from the back. Therefore, it does not have a |Size| method,
// only |BufferSize|, which is a constant, and |CurrentIndex|, which is the
// number of elements added so far.
//
// If the above is sufficient for your use case, base::RingBuffer should be more
// efficient than base::circular_deque.
template <typename T, mem_size kSize>
class RingBuffer {
 public:
  RingBuffer() : current_index_(0) {}
  RingBuffer(const RingBuffer&) = delete;
  RingBuffer& operator=(const RingBuffer&) = delete;

  mem_size BufferSize() const { return kSize; }

  mem_size CurrentIndex() const { return current_index_; }

  // Returns true if a value was saved to index |n|.
  bool IsFilledIndex(mem_size n) const {
    return IsFilledIndexByBufferIndex(BufferIndex(n));
  }

  // Returns the element at index |n| (% |kSize|).
  //
  // n = 0 returns the oldest value and
  // n = bufferSize() - 1 returns the most recent value.
  const T& ReadBuffer(mem_size n) const {
    const mem_size buffer_index = BufferIndex(n);
    DCHECK(IsFilledIndexByBufferIndex(buffer_index));
    return buffer_[buffer_index];
  }

  T* MutableReadBuffer(mem_size n) {
    const mem_size buffer_index = BufferIndex(n);
    DCHECK(IsFilledIndexByBufferIndex(buffer_index));
    return &buffer_[buffer_index];
  }

  void SaveToBuffer(const T& value) {
    buffer_[BufferIndex(0)] = value;
    current_index_++;
  }

  void Clear() { current_index_ = 0; }

  // Iterator has const access to the RingBuffer it got retrieved from.
  class Iterator {
   public:
    mem_size index() const { return index_; }

    const T* operator->() const { return &buffer_.ReadBuffer(index_); }
    const T* operator*() const { return &buffer_.ReadBuffer(index_); }

    Iterator& operator++() {
      index_++;
      if (index_ == kSize)
        out_of_range_ = true;
      return *this;
    }

    Iterator& operator--() {
      if (index_ == 0)
        out_of_range_ = true;
      index_--;
      return *this;
    }

    operator bool() const { return !out_of_range_ && buffer_.IsFilledIndex(index_); }

   private:
    Iterator(const RingBuffer<T, kSize>& buffer, mem_size index)
        : buffer_(buffer), index_(index), out_of_range_(false) {}

    const RingBuffer<T, kSize>& buffer_;
    mem_size index_;
    bool out_of_range_;

    friend class RingBuffer<T, kSize>;
  };

  // Returns an Iterator pointing to the oldest value in the buffer.
  // Example usage (iterate from oldest to newest value):
  //  for (RingBuffer<T, kSize>::Iterator it = ring_buffer.Begin(); it; ++it) {}
  Iterator Begin() const {
    if (current_index_ < kSize)
      return Iterator(*this, kSize - current_index_);
    return Iterator(*this, 0);
  }

  // Returns an Iterator pointing to the newest value in the buffer.
  // Example usage (iterate backwards from newest to oldest value):
  //  for (RingBuffer<T, kSize>::Iterator it = ring_buffer.End(); it; --it) {}
  Iterator End() const { return Iterator(*this, kSize - 1); }

 private:
  inline mem_size BufferIndex(mem_size n) const { return (current_index_ + n) % kSize; }

  // This specialization of |IsFilledIndex| is a micro-optimization that enables
  // us to do e.g. `DCHECK(IsFilledIndex(n))` without calling |BufferIndex|
  // twice. Since |BufferIndex| involves a % operation, it's not quite free at a
  // micro-scale.
  inline bool IsFilledIndexByBufferIndex(mem_size buffer_index) const {
    return buffer_index < current_index_;
  }

  T buffer_[kSize];
  mem_size current_index_;
};

}  // namespace base