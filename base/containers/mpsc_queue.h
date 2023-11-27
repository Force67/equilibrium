#pragma once
// c++ translation of
// https://github.com/dbittman/waitfree-mpsc-queue/blob/master/mpsc.c

#include <atomic>

namespace base {
template <typename T>
class MpscQueue {
 public:
  explicit MpscQueue(size_t capacity)
      : count_(0), head_(0), tail_(0), max_(capacity) {
    buffer_ = static_cast<char*>(operator new[](capacity * sizeof(T)));
  }

  ~MpscQueue() {
    for (size_t i = 0; i < max_; ++i) {
      reinterpret_cast<T*>(buffer_ + i * sizeof(T))->~T();
    }
    operator delete[](buffer_);
  }

  // Enqueue by copying
  bool enqueue(const T& item) {
    size_t count = count_.fetch_add(1, std::memory_order_acquire);
    if (count >= max_) {
      // Queue is full
      count_.fetch_sub(1, std::memory_order_release);
      return false;
    }

    size_t head = head_.load(std::memory_order_relaxed);
    new (buffer_ + (head % max_) * sizeof(T)) T(item);
    head_.store((head + 1) % max_, std::memory_order_release);
    return true;
  }

  
  T dequeue() {
    if (count_.load(std::memory_order_acquire) == 0) {
      // Queue is empty
      return T();  // Return default-constructed object
    }

    T* obj = reinterpret_cast<T*>(buffer_ + (tail_ % max_) * sizeof(T));

    // Additional check after calculating the tail index
    if (count_.load(std::memory_order_acquire) == 0) {
      return T();  // Queue might have become empty
    }


    T ret = std::move(*obj);  // Move object out of queue
    obj->~T();                // Call destructor

    if (++tail_ >= max_) {
      tail_ = 0;
    }
    count_.fetch_sub(1, std::memory_order_release);
    return ret;
  }

  size_t count() const { return count_.load(std::memory_order_relaxed); }
  bool empty() const { return count_.load(std::memory_order_acquire) == 0; }

  size_t capacity() const { return max_; }

 private:
  std::atomic<size_t> count_;
  std::atomic<size_t> head_;
  size_t tail_;
  char* buffer_;
  size_t max_;
};

}  // namespace base