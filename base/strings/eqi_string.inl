// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// EqiString - A replacement for standard strings based off Facebook's excellent
// fbstring.

#include <base/strings/eqi_string.h>

namespace base {

template <class TChar>
inline basic_eqi_string<TChar>::basic_eqi_string(const basic_eqi_string& rhs) {
  {
    DCHECK(&rhs != this);
    switch (rhs.category()) {
      case Category::kSmall:
        CopySmall(rhs);
        break;
      case Category::kMedium:
        CopyMedium(rhs);
        break;
      case Category::kLarge:
        CopyLarge(rhs);
        break;
      default:
        IMPOSSIBLE;
    }
    DCHECK(size() == rhs.size());
    DCHECK(memcmp(data(), rhs.data(), size() * sizeof(TChar)) == 0);
  }
}

template <class TChar>
inline void basic_eqi_string<TChar>::CopySmall(const basic_eqi_string& rhs) {
  static_assert(offsetof(MediumLarge, data_) == 0, "fbstring layout failure");
  static_assert(offsetof(MediumLarge, size_) == sizeof(ml_.data_),
                "fbstring layout failure");
  static_assert(offsetof(MediumLarge, capacity_) == 2 * sizeof(ml_.data_),
                "fbstring layout failure");
  // Just write the whole thing, don't look at details. In
  // particular we need to copy capacity anyway because we want
  // to set the size (don't forget that the last TCharacter,
  // which stores a short string's length, is shared with the
  // ml_.capacity field).
  ml_ = rhs.ml_;
  DCHECK(category() == Category::kSmall && this->size() == rhs.size());
}

template <class TChar>
NOINLINE void basic_eqi_string<TChar>::CopyMedium(const basic_eqi_string& rhs) {
  // Medium strings are copied eagerly. Don't forget to allocate
  // one extra TChar for the null terminator.
  auto const allocSize = goodMallocSize((1 + rhs.ml_.size_) * sizeof(TChar));
  ml_.data_ = static_cast<TChar*>(malloc(allocSize));
  // Also copies terminator.
  fbstring_detail::podCopy(rhs.ml_.data_, rhs.ml_.data_ + rhs.ml_.size_ + 1,
                           ml_.data_);
  ml_.size_ = rhs.ml_.size_;
  ml_.setCapacity(allocSize / sizeof(TChar) - 1, Category::kMedium);
  DCHECK(category() == Category::kMedium);
}

template <class TChar>
NOINLINE void basic_eqi_string<TChar>::CopyLarge(const basic_eqi_string& rhs) {
  // Large strings are just refcounted
  ml_ = rhs.ml_;
  RefCounted::incrementRefs(ml_.data_);
  DCHECK(category() == Category::kLarge && size() == rhs.size());
}

// Small strings are bitblitted
template <class TChar>
inline void basic_eqi_string<TChar>::InitSmall(const TChar* const data,
                                              const size_t size) {
  // Layout is: TChar* data_, size_t size_, size_t capacity_
  static_assert(sizeof(*this) == sizeof(TChar*) + 2 * sizeof(size_t),
                "fbstring has unexpected size");
  static_assert(sizeof(TChar*) == sizeof(size_t),
                "fbstring size assumption violation");
  // sizeof(size_t) must be a power of 2
  static_assert((sizeof(size_t) & (sizeof(size_t) - 1)) == 0,
                "fbstring size assumption violation");

// If data is aligned, use fast word-wise copying. Otherwise,
// use conservative memcpy.
// The word-wise path reads bytes which are outside the range of
// the string, and makes ASan unhappy, so we disable it when
// compiling with ASan.
#ifndef SANITIZE_ADDRESS
  if ((reinterpret_cast<size_t>(data) & (sizeof(size_t) - 1)) == 0) {
    const size_t byteSize = size * sizeof(TChar);
    constexpr size_t wordWidth = sizeof(size_t);
    switch ((byteSize + wordWidth - 1) / wordWidth) {  // Number of words.
      case 3:
        ml_.capacity_ = reinterpret_cast<const size_t*>(data)[2];
        [[fallthrough]];
      case 2:
        ml_.size_ = reinterpret_cast<const size_t*>(data)[1];
        [[fallthrough]];
      case 1:
        ml_.data_ = *reinterpret_cast<TChar**>(const_cast<TChar*>(data));
        [[fallthrough]];
      case 0:
        break;
    }
  } else
#endif
  {
    if (size != 0) {
      fbstring_detail::podCopy(data, data + size, small_);
    }
  }
  SetSmallSize(size);
}

template <class TChar>
NOINLINE void basic_eqi_string<TChar>::InitMedium(const TChar* const data,
                                                 const size_t size) {
  // Medium strings are allocated normally. Don't forget to
  // allocate one extra TChar for the terminating null.
  auto const allocSize = goodMallocSize((1 + size) * sizeof(TChar));
  ml_.data_ = static_cast<TChar*>(malloc(allocSize));
  if (LIKELY(size > 0)) {
    fbstring_detail::podCopy(data, data + size, ml_.data_);
  }
  ml_.size_ = size;
  ml_.setCapacity(allocSize / sizeof(TChar) - 1, Category::kMedium);
  ml_.data_[size] = '\0';
}

template <class TChar>
NOINLINE void basic_eqi_string<TChar>::InitLarge(const TChar* const data,
                                                const size_t size) {
  // Large strings are allocated differently
  size_t effectiveCapacity = size;
  auto const newRC = RefCounted::create(data, &effectiveCapacity);
  ml_.data_ = newRC->data_;
  ml_.size_ = size;
  ml_.setCapacity(effectiveCapacity, Category::kLarge);
  ml_.data_[size] = '\0';
}

template <class TChar>
NOINLINE void basic_eqi_string<TChar>::unshare(size_t minCapacity) {
  DCHECK(category() == Category::kLarge);
  size_t effectiveCapacity = std::max(minCapacity, ml_.capacity());
  auto const newRC = RefCounted::create(&effectiveCapacity);
  // If this fails, someone placed the wrong capacity in an
  // fbstring.
  DCHECK(effectiveCapacity >= ml_.capacity());
  // Also copies terminator.
  fbstring_detail::podCopy(ml_.data_, ml_.data_ + ml_.size_ + 1, newRC->data_);
  RefCounted::decrementRefs(ml_.data_);
  ml_.data_ = newRC->data_;
  ml_.setCapacity(effectiveCapacity, Category::kLarge);
  // size_ remains unchanged.
}

template <class TChar>
inline TChar* basic_eqi_string<TChar>::mutableDataLarge() {
  DCHECK(category() == Category::kLarge);
  if (RefCounted::refs(ml_.data_) > 1) {  // Ensure unique.
    unshare();
  }
  return ml_.data_;
}

template <class TChar>
NOINLINE void basic_eqi_string<TChar>::ReserveLarge(size_t minCapacity) {
  DCHECK(category() == Category::kLarge);
  if (RefCounted::refs(ml_.data_) > 1) {  // Ensure unique
    // We must make it unique regardless; in-place reallocation is
    // useless if the string is shared. In order to not surprise
    // people, reserve the new block at current capacity or
    // more. That way, a string's capacity never shrinks after a
    // call to reserve.
    unshare(minCapacity);
  } else {
    // String is not shared, so let's try to realloc (if needed)
    if (minCapacity > ml_.capacity()) {
      // Asking for more memory
      auto const newRC =
          RefCounted::reallocate(ml_.data_, ml_.size_, ml_.capacity(), &minCapacity);
      ml_.data_ = newRC->data_;
      ml_.setCapacity(minCapacity, Category::kLarge);
    }
    DCHECK(capacity() >= minCapacity);
  }
}

template <class TChar>
NOINLINE void basic_eqi_string<TChar>::ReserveMedium(const size_t minCapacity) {
  DCHECK(category() == Category::kMedium);
  // String is not shared
  if (minCapacity <= ml_.capacity()) {
    return;  // nothing to do, there's enough room
  }
  if (minCapacity <= maxMediumSize) {
    // Keep the string at medium size. Don't forget to allocate
    // one extra TChar for the terminating null.
    size_t capacityBytes = goodMallocSize((1 + minCapacity) * sizeof(TChar));
    // Also copies terminator.
    ml_.data_ = static_cast<TChar*>(
        smartRealloc(ml_.data_, (ml_.size_ + 1) * sizeof(TChar),
                     (ml_.capacity() + 1) * sizeof(TChar), capacityBytes));
    ml_.setCapacity(capacityBytes / sizeof(TChar) - 1, Category::kMedium);
  } else {
    // Conversion from medium to large string
    basic_eqi_string nascent;
    // Will recurse to another branch of this function
    nascent.reserve(minCapacity);
    nascent.ml_.size_ = ml_.size_;
    // Also copies terminator.
    fbstring_detail::podCopy(ml_.data_, ml_.data_ + ml_.size_ + 1,
                             nascent.ml_.data_);
    nascent.swap(*this);
    DCHECK(capacity() >= minCapacity);
  }
}

template <class TChar>
NOINLINE void basic_eqi_string<TChar>::ReserveSmall(size_t minCapacity,
                                                   const bool disableSSO) {
  DCHECK(category() == Category::kSmall);
  if (!disableSSO && minCapacity <= maxSmallSize) {
    // small
    // Nothing to do, everything stays put
  } else if (minCapacity <= maxMediumSize) {
    // medium
    // Don't forget to allocate one extra TChar for the terminating null
    auto const allocSizeBytes = goodMallocSize((1 + minCapacity) * sizeof(TChar));
    auto const pData = static_cast<TChar*>(malloc(allocSizeBytes));
    auto const size = smallSize();
    // Also copies terminator.
    fbstring_detail::podCopy(small_, small_ + size + 1, pData);
    ml_.data_ = pData;
    ml_.size_ = size;
    ml_.setCapacity(allocSizeBytes / sizeof(TChar) - 1, Category::kMedium);
  } else {
    // large
    auto const newRC = RefCounted::create(&minCapacity);
    auto const size = smallSize();
    // Also copies terminator.
    fbstring_detail::podCopy(small_, small_ + size + 1, newRC->data_);
    ml_.data_ = newRC->data_;
    ml_.size_ = size;
    ml_.setCapacity(minCapacity, Category::kLarge);
    DCHECK(capacity() >= minCapacity);
  }
}

template <class TChar>
inline TChar* basic_eqi_string<TChar>::expandNoinit(
    const size_t delta,
    bool expGrowth, /* = false */
    bool disableSSO /* = kDISABLESSAO */) {
  // Strategy is simple: make room, then change size
  DCHECK(capacity() >= size());
  size_t sz, newSz;
  if (category() == Category::kSmall) {
    sz = smallSize();
    newSz = sz + delta;
    if (!disableSSO && LIKELY(newSz <= maxSmallSize)) {
      SetSmallSize(newSz);
      return small_ + sz;
    }
    ReserveSmall(expGrowth ? std::max(newSz, 2 * maxSmallSize) : newSz, disableSSO);
  } else {
    sz = ml_.size_;
    newSz = sz + delta;
    if (UNLIKELY(newSz > capacity())) {
      // ensures not shared
      reserve(expGrowth ? std::max(newSz, 1 + capacity() * 3 / 2) : newSz);
    }
  }
  DCHECK(capacity() >= newSz);
  // Category can't be small - we took care of that above
  DCHECK(category() == Category::kMedium || category() == Category::kLarge);
  ml_.size_ = newSz;
  ml_.data_[newSz] = '\0';
  DCHECK(size() == newSz);
  return ml_.data_ + sz;
}

template <class TChar>
inline void basic_eqi_string<TChar>::ShrinkSmall(const size_t delta) {
  // Check for underflow
  DCHECK(delta <= smallSize());
  SetSmallSize(smallSize() - delta);
}

template <class TChar>
inline void basic_eqi_string<TChar>::ShrinkMedium(const size_t delta) {
  // Medium strings and unique large strings need no special
  // handling.
  DCHECK(ml_.size_ >= delta);
  ml_.size_ -= delta;
  ml_.data_[ml_.size_] = '\0';
}

template <class TChar>
inline void basic_eqi_string<TChar>::ShrinkLarge(const size_t delta) {
  DCHECK(ml_.size_ >= delta);
  // Shared large string, must make unique. This is because of the
  // durn terminator must be written, which may trample the shared
  // data.
  if (delta) {
    basic_eqi_string(ml_.data_, ml_.size_ - delta).swap(*this);
  }
  // No need to write the terminator.
}
}  // namespace base