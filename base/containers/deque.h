#pragma once

namespace base {
template <typename T>
class SimpleDeque {
 private:
  T* data;
  size_t capacity;
  size_t frontIndex;
  size_t backIndex;
  size_t size;

  void resize(size_t new_capacity) {
    T* new_data = new T[new_capacity];
    size_t current = frontIndex;
    for (size_t i = 0; i < size; ++i) {
      new_data[i] = data[current];
      current = (current + 1) % capacity;
    }
    delete[] data;
    data = new_data;
    capacity = new_capacity;
    frontIndex = 0;
    backIndex = size;
  }

 public:
  SimpleDeque()
      : data(nullptr), capacity(0), frontIndex(0), backIndex(0), size(0) {}

  ~SimpleDeque() { delete[] data; }

  void push_front(const T& value) {
    if (size == capacity) {
      resize(capacity == 0 ? 1 : 2 * capacity);
    }
    frontIndex = (frontIndex == 0) ? capacity - 1 : frontIndex - 1;
    data[frontIndex] = value;
    size++;
  }

  void push_back(const T& value) {
    if (size == capacity) {
      resize(capacity == 0 ? 1 : 2 * capacity);
    }
    data[backIndex] = value;
    backIndex = (backIndex + 1) % capacity;
    size++;
  }

  void pop_front() {
    if (size == 0) {
      throw std::out_of_range("Deque is empty");
    }
    frontIndex = (frontIndex + 1) % capacity;
    size--;
  }

  void pop_back() {
    if (size == 0) {
      throw std::out_of_range("Deque is empty");
    }
    backIndex = (backIndex == 0) ? capacity - 1 : backIndex - 1;
    size--;
  }

  T& front() {
    if (size == 0) {
      throw std::out_of_range("Deque is empty");
    }
    return data[frontIndex];
  }

  T& back() {
    if (size == 0) {
      throw std::out_of_range("Deque is empty");
    }
    size_t backIdx = (backIndex == 0) ? capacity - 1 : backIndex - 1;
    return data[backIdx];
  }

  bool empty() const { return size == 0; }

  size_t deque_size() const { return size; }
};

}  // namespace base