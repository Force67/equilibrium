// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <atomic>
#include <base/check.h>

namespace base {
template <typename TChar>
struct SharedString {
  std::atomic<size_t> refCount_;
  TChar data_[1];

  static inline size_t goodMallocSize(size_t minSize) noexcept {
    if (minSize == 0) {
      return 0;
    }

    return minSize;
  }

  constexpr static size_t getDataOffset() { return offsetof(SharedString, data_); }

  static SharedString* fromData(TChar* p) {
    return static_cast<SharedString*>(static_cast<void*>(
        static_cast<unsigned TChar*>(static_cast<void*>(p)) - getDataOffset()));
  }

  static size_t refs(TChar* p) {
    return fromData(p)->refCount_.load(std::memory_order_acquire);
  }

  static void incrementRefs(TChar* p) {
    fromData(p)->refCount_.fetch_add(1, std::memory_order_acq_rel);
  }

  static void decrementRefs(TChar* p) {
    auto const dis = fromData(p);
    size_t oldcnt = dis->refCount_.fetch_sub(1, std::memory_order_acq_rel);
    DCHECK(oldcnt > 0);
    if (oldcnt == 1) {
      free(dis);
    }
  }

  static SharedString* create(size_t* size) {
    const size_t allocSize =
        goodMallocSize(getDataOffset() + (*size + 1) * sizeof(TChar));
    auto result = static_cast<SharedString*>(malloc(allocSize));
    result->refCount_.store(1, std::memory_order_release);
    *size = (allocSize - getDataOffset()) / sizeof(TChar) - 1;
    return result;
  }

  static SharedString* create(const TChar* data, size_t* size) {
    const size_t effectiveSize = *size;
    auto result = create(size);
    if (LIKELY(effectiveSize > 0)) {
      fbstring_detail::podCopy(data, data + effectiveSize, result->data_);
    }
    return result;
  }

  static SharedString* reallocate(TChar* const data,
                                  const size_t currentSize,
                                  const size_t currentCapacity,
                                  size_t* newCapacity) {
    DCHECK(*newCapacity > 0 && *newCapacity > currentSize);
    const size_t allocNewCapacity =
        goodMallocSize(getDataOffset() + (*newCapacity + 1) * sizeof(TChar));
    auto const dis = fromData(data);
    DCHECK(dis->refCount_.load(std::memory_order_acquire) == 1);
    auto result = static_cast<SharedString*>(smartRealloc(
        dis, getDataOffset() + (currentSize + 1) * sizeof(TChar),
        getDataOffset() + (currentCapacity + 1) * sizeof(TChar), allocNewCapacity));
    DCHECK(result->refCount_.load(std::memory_order_acquire) == 1);
    *newCapacity = (allocNewCapacity - getDataOffset()) / sizeof(TChar) - 1;
    return result;
  }
};
}  // namespace base