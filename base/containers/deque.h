#pragma once

#include <base/arch.h>
#include <base/check.h>
#include <base/memory/move.h>
#include <base/containers/container_traits.h>
#include <base/memory/cxx_lifetime.h>

#include <new>

namespace base {
template <typename T, class TAllocator = base::DefaultAllocator>
class SimpleDeque {
 private:
  T* data_;
  mem_size capacity_;
  mem_size front_index_;
  mem_size back_index_;
  mem_size size_;

  void Grow(mem_size new_capacity) {
    T* new_data = static_cast<T*>(TAllocator::Allocate(new_capacity * sizeof(T)));
    // Default-construct all slots.
    for (mem_size i = 0; i < new_capacity; ++i) {
      ::new (static_cast<void*>(&new_data[i])) T();
    }

    // Copy old elements into contiguous front of new buffer.
    if (data_ && size_ > 0) {
      const mem_size old_cap = capacity_;
      mem_size src = front_index_;
      for (mem_size i = 0; i < size_; ++i) {
        new_data[i] = base::move(data_[src]);
        src = (src + 1) % old_cap;
      }
    }

    // Destroy old buffer.
    if (data_) {
      for (mem_size i = 0; i < capacity_; ++i) {
        data_[i].~T();
      }
      TAllocator::Free(data_, capacity_ * sizeof(T));
    }

    data_ = new_data;
    capacity_ = new_capacity;
    front_index_ = 0;
    back_index_ = size_;
  }

 public:
  SimpleDeque()
      : data_(nullptr), capacity_(0), front_index_(0), back_index_(0), size_(0) {}

  ~SimpleDeque() {
    if (data_) {
      for (mem_size i = 0; i < capacity_; ++i) {
        data_[i].~T();
      }
      TAllocator::Free(data_, capacity_ * sizeof(T));
    }
  }

  // No copy (add later if needed).
  SimpleDeque(const SimpleDeque&) = delete;
  SimpleDeque& operator=(const SimpleDeque&) = delete;

  // Move support.
  SimpleDeque(SimpleDeque&& other) noexcept
      : data_(other.data_),
        capacity_(other.capacity_),
        front_index_(other.front_index_),
        back_index_(other.back_index_),
        size_(other.size_) {
    other.data_ = nullptr;
    other.capacity_ = other.front_index_ = other.back_index_ = other.size_ = 0;
  }

  SimpleDeque& operator=(SimpleDeque&& other) noexcept {
    if (this != &other) {
      if (data_) {
        for (mem_size i = 0; i < capacity_; ++i) data_[i].~T();
        TAllocator::Free(data_, capacity_ * sizeof(T));
      }
      data_ = other.data_;
      capacity_ = other.capacity_;
      front_index_ = other.front_index_;
      back_index_ = other.back_index_;
      size_ = other.size_;
      other.data_ = nullptr;
      other.capacity_ = other.front_index_ = other.back_index_ = other.size_ = 0;
    }
    return *this;
  }

  void push_front(const T& value) {
    if (size_ == capacity_) {
      Grow(capacity_ == 0 ? 4 : capacity_ * 2);
    }
    front_index_ = (front_index_ == 0) ? capacity_ - 1 : front_index_ - 1;
    data_[front_index_] = value;
    ++size_;
  }

  void push_back(const T& value) {
    if (size_ == capacity_) {
      Grow(capacity_ == 0 ? 4 : capacity_ * 2);
    }
    data_[back_index_] = value;
    back_index_ = (back_index_ + 1) % capacity_;
    ++size_;
  }

  // Move overloads, so a move-only element type (a Function, a UniquePointer)
  // can be queued at all.
  void push_front(T&& value) {
    if (size_ == capacity_) {
      Grow(capacity_ == 0 ? 4 : capacity_ * 2);
    }
    front_index_ = (front_index_ == 0) ? capacity_ - 1 : front_index_ - 1;
    data_[front_index_] = base::move(value);
    ++size_;
  }

  void push_back(T&& value) {
    if (size_ == capacity_) {
      Grow(capacity_ == 0 ? 4 : capacity_ * 2);
    }
    data_[back_index_] = base::move(value);
    back_index_ = (back_index_ + 1) % capacity_;
    ++size_;
  }

  template <typename... TArgs>
  T& emplace_back(TArgs&&... args) {
    if (size_ == capacity_) {
      Grow(capacity_ == 0 ? 4 : capacity_ * 2);
    }
    T& slot = data_[back_index_];
    slot = T(base::forward<TArgs>(args)...);
    back_index_ = (back_index_ + 1) % capacity_;
    ++size_;
    return slot;
  }

  void pop_front() {
    BASE_DCHECK(size_ > 0, "SimpleDeque::pop_front: empty");
    front_index_ = (front_index_ + 1) % capacity_;
    --size_;
  }

  void pop_back() {
    BASE_DCHECK(size_ > 0, "SimpleDeque::pop_back: empty");
    back_index_ = (back_index_ == 0) ? capacity_ - 1 : back_index_ - 1;
    --size_;
  }

  T& front() {
    BASE_DCHECK(size_ > 0, "SimpleDeque::front: empty");
    return data_[front_index_];
  }

  const T& front() const {
    BASE_DCHECK(size_ > 0, "SimpleDeque::front: empty");
    return data_[front_index_];
  }

  T& back() {
    BASE_DCHECK(size_ > 0, "SimpleDeque::back: empty");
    mem_size idx = (back_index_ == 0) ? capacity_ - 1 : back_index_ - 1;
    return data_[idx];
  }

  const T& back() const {
    BASE_DCHECK(size_ > 0, "SimpleDeque::back: empty");
    mem_size idx = (back_index_ == 0) ? capacity_ - 1 : back_index_ - 1;
    return data_[idx];
  }

  T& operator[](mem_size index) {
    BASE_DCHECK(index < size_, "SimpleDeque::[]: out of bounds");
    return data_[(front_index_ + index) % capacity_];
  }

  const T& operator[](mem_size index) const {
    BASE_DCHECK(index < size_, "SimpleDeque::[]: out of bounds");
    return data_[(front_index_ + index) % capacity_];
  }

  bool empty() const { return size_ == 0; }
  mem_size size() const { return size_; }
  mem_size deque_size() const { return size_; }

  // Front-to-back traversal. The storage is a ring, so this walks by index
  // rather than by pointer.
  template <bool kConst>
  class IteratorImpl {
   public:
    using Owner = base::conditional_t<kConst, const SimpleDeque, SimpleDeque>;
    using Reference = base::conditional_t<kConst, const T&, T&>;

    IteratorImpl(Owner* owner, mem_size index) : owner_(owner), index_(index) {}

    bool operator==(const IteratorImpl& other) const { return index_ == other.index_; }
    bool operator!=(const IteratorImpl& other) const { return index_ != other.index_; }

    IteratorImpl& operator++() {
      ++index_;
      return *this;
    }

    Reference operator*() const { return (*owner_)[index_]; }

   private:
    Owner* owner_;
    mem_size index_;
  };

  using Iterator = IteratorImpl<false>;
  using ConstIterator = IteratorImpl<true>;

  Iterator begin() { return Iterator(this, 0); }
  Iterator end() { return Iterator(this, size_); }
  ConstIterator begin() const { return ConstIterator(this, 0); }
  ConstIterator end() const { return ConstIterator(this, size_); }

  void clear() {
    size_ = 0;
    front_index_ = 0;
    back_index_ = 0;
  }
};

}  // namespace base