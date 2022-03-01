// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace base {

class AllocatorBase {
  friend class MemoryCoordinator;

  // prevent the user from accessing these functions directly.
 protected:
  // with the hopes that the compiler will devirutalize this.
  constexpr virtual void* Allocate(size_t size, size_t alignment = 1024) = 0;
  constexpr virtual void Free(void* block) = 0;
};

class MemoryCoordinator {
 public:
  MemoryCoordinator();

  void* Allocate(size_t size);
  void Free(void*);
 private:
  AllocatorBase* allocators_[3]{};
};
}  // namespace base
