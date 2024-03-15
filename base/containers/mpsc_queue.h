#pragma once
// c++ translation of
// https://github.com/dbittman/waitfree-mpsc-queue/blob/master/mpsc.c

#include <atomic>
#include <deque>
#include <base/memory/move.h>

namespace base {
template <typename T>
class MPSCQueue {
 public:
  struct Node;
  // Iterator class
  class Iterator {
   public:
    explicit Iterator(Node* node) : node_(node) {}

    T& operator*() const { return node_->value; }
    T* operator->() const { return &node_->value; }

    // Prefix increment
    Iterator& operator++() {
      node_ = node_->next.load(std::memory_order_acquire);
      return *this;
    }

    // Postfix increment
    Iterator operator++(int) {
      Iterator temp = *this;
      ++(*this);
      return temp;
    }

    bool operator==(const Iterator& other) const {
      return node_ == other.node_;
    }

    bool operator!=(const Iterator& other) const { return !(*this == other); }

   private:
    Node* node_;
  };

  Iterator begin() const {
    Node* tail = tail_.load(std::memory_order_acquire);
    return Iterator(tail->next.load(std::memory_order_relaxed));
  }

  Iterator end() const { return Iterator(nullptr); }

  MPSCQueue() : head_(new Node), tail_(head_.load(std::memory_order_relaxed)) {
    Node* front = head_.load(std::memory_order_relaxed);
    front->next.store(nullptr, std::memory_order_relaxed);
  }

  ~MPSCQueue() {
    T output;
    while (this->dequeue(output)) {
    }
    delete head_.load(std::memory_order_relaxed);
  }

  void enqueue(T&& value) {
    Node* node = new Node(base::move(value));
    Node* prev_head = head_.exchange(node, std::memory_order_acq_rel);
    prev_head->next.store(node, std::memory_order_release);
  }

  template <typename... Args>
  void emplace(Args&&... args) {
    Node* node = new Node(std::forward<Args>(args)...);
    Node* prev_head = head_.exchange(node, std::memory_order_acq_rel);
    prev_head->next.store(node, std::memory_order_release);
  }

  bool dequeue(T& value) {
    Node* tail = tail_.load(std::memory_order_relaxed);
    Node* next = tail->next.load(std::memory_order_acquire);

    if (next == nullptr) {
      return false;
    }

    value = next->value;
    tail_.store(next, std::memory_order_release);
    delete tail;
    return true;
  }

  bool peek(T*& value) const {
    Node* tail = tail_.load(std::memory_order_relaxed);
    Node* next = tail->next.load(std::memory_order_acquire);

    if (next == nullptr) {
      return false;  // Queue is empty, nothing to peek
    }

    value = &next->value;  // Copy the value of the next item
    return true;
  }

  bool removeFront() {
    Node* tail = tail_.load(std::memory_order_relaxed);
    Node* next = tail->next.load(std::memory_order_acquire);

    if (next == nullptr) {
      return false;  // Queue is empty, nothing to remove
    }

    // Skip fetching the value, just update the tail and delete the old tail
    // node
    tail_.store(next, std::memory_order_release);
    delete tail;
    return true;
  }

  bool empty() const {
    return tail_.load(std::memory_order_acquire)
               ->next.load(std::memory_order_acquire) == nullptr;
  }

  struct Node {
    T value;
    std::atomic<Node*> next;

    Node() : next(nullptr) {}
    explicit Node(const T&& value) : value(base::move(value)), next(nullptr) {}

    template <typename... Args>
    explicit Node(Args&&... args)
        : value(std::forward<Args>(args)...), next(nullptr) {}
  };
 private:
  std::atomic<Node*> head_;
  std::atomic<Node*> tail_;
};

}  // namespace base