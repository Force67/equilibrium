// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// EqiString - A replacement for standard strings based off Facebook's excellent
// fbstring.
#pragma once

#include <atomic>
#include <base/compiler.h>
#include <base/check.h>
#include <base/arch.h>

namespace base {

static constexpr bool kDisableEqiSmallStringOptimization = false;

namespace fbstring_detail {
template <class Pod>
inline void podCopy(const Pod* b, const Pod* e, Pod* d) {
  memcpy(d, b, (e - b) * sizeof(Pod));
}
}  // namespace fbstring_detail

template <typename TTChar>
struct ExternalStorage {
  constexpr static u8 categoryExtractMask = base::kIsLittleEndian ? 0xC0 : 0x3;
  constexpr static size_t kCategoryShift = (sizeof(size_t) - 1) * 8;
  constexpr static size_t capacityExtractMask =
      base::kIsLittleEndian ? ~(size_t(categoryExtractMask) << kCategoryShift)
                            : 0x0 /* unused */;
  TTChar* data_;
  size_t size_;
  size_t capacity_;

  size_t capacity() const {
    return base::kIsLittleEndian ? capacity_ & capacityExtractMask : capacity_ >> 2;
  }

  void setCapacity(size_t cap, Category cat) {
    capacity_ = base::kIsLittleEndian
                    ? cap | (static_cast<size_t>(cat) << kCategoryShift)
                    : (cap << 2) | static_cast<size_t>(cat);
  }
};

template <class TChar>
class basic_eqi_string {
 public:
  // type of string.
  enum class Category : u8 {
    kSmall = 0,
    kMedium = base::kIsLittleEndian ? 0x80 : 0x2,
    kLarge = base::kIsLittleEndian ? 0x40 : 0x1,
  };

  static constexpr size_t maxSmallSize = lastTChar / sizeof(TChar);

  basic_eqi_string() noexcept { reset(); }
  basic_eqi_string(const basic_eqi_string& rhs);
  basic_eqi_string(basic_eqi_string&& goner) noexcept {
    ml_ = goner.ml_;
    goner.reset();
  }
  // copy data into string
  basic_eqi_string(const TChar* const data,
                   constexpr size_t size,
                   constexpr bool disableSSO = kDisableEqiSmallStringOptimization) {
    if constexpr (!disableSSO && size <= maxSmallSize) {
      InitSmall(data, size);
    } else if constexpr (size <= maxMediumSize) {
      InitMedium(data, size);
    } else {
      InitLarge(data, size);
    }
    DCHECK(this->size() == size);
    DCHECK(size == 0 || memcmp(this->data(), data, size * sizeof(TChar)) == 0);
  }
  // takes a previously allocated string. Remember to add one to the size in order to
  // account for the null terminator!
  basic_eqi_string(TChar* const data,
                   const size_t size,
                   const size_t allocatedSize) {
    if (size > 0) {
      DCHECK(allocatedSize >= size + 1);
      DCHECK(data[size] == '\0');
      // Use the medium string storage
      ml_.data_ = data;
      ml_.size_ = size;
      // Don't forget about null terminator
      ml_.setCapacity(allocatedSize - 1, Category::kMedium);
    } else {
      // No need for the memory
      free(data);
      reset();
    }
  }

  ~basic_eqi_string() noexcept {
    if (category() == Category::kSmall)
      return;
    destroyMediumLarge();
  }

  basic_eqi_string& operator=(const basic_eqi_string&) = delete;

  void Swap(basic_eqi_string& rhs) {
    auto const t = ml_;
    ml_ = rhs.ml_;
    rhs.ml_ = t;
  }

  const TChar* data() const { return c_str(); }
  TChar* data() { return c_str(); }

  TChar* mutableData() {
    switch (category()) {
      case Category::kSmall:
        return small_;
      case Category::kMedium:
        return ml_.data_;
      case Category::kLarge:
        return mutableDataLarge();
    }
    IMPOSSIBLE;
  }

  const TChar* c_str() const {
    const TChar* ptr = ml_.data_;
    // With this syntax, GCC and Clang generate a CMOV instead of a branch.
    ptr = (category() == Category::kSmall) ? small_ : ptr;
    return ptr;
  }

  void shrink(const size_t delta) {
    if (category() == Category::kSmall) {
      ShrinkSmall(delta);
    } else if (category() == Category::kMedium ||
               RefCounted<TChar>::refs(ml_.data_) == 1) {
      ShrinkMedium(delta);
    } else {
      ShrinkLarge(delta);
    }
  }

  NOINLINE
  void reserve(size_t minCapacity,
               bool disableSSO = kDisableEqiSmallStringOptimization) {
    switch (category()) {
      case Category::kSmall:
        ReserveSmall(minCapacity, disableSSO);
        break;
      case Category::kMedium:
        ReserveMedium(minCapacity);
        break;
      case Category::kLarge:
        ReserveLarge(minCapacity);
        break;
      default:
        IMPOSSIBLE;
    }
    DCHECK(capacity() >= minCapacity);
  }

  TChar* expandNoinit(const size_t delta,
                      bool expGrowth = false,
                      bool disableSSO = kDisableEqiSmallStringOptimization);

  void push_back(TChar c) { *expandNoinit(1, /* expGrowth = */ true) = c; }

  size_t size() const {
    size_t ret = ml_.size_;
    if constexpr (base::kIsLittleEndian) {
      // We can save a couple instructions, because the category is
      // small if the last TChar, as unsigned, is <= maxSmallSize.
      typedef typename std::make_unsigned<TChar>::type UTChar;

      // using UTChar = unsigned TChar;
      auto maybeSmallSize =
          size_t(maxSmallSize) - size_t(static_cast<UTChar>(small_[maxSmallSize]));
      // With this syntax, GCC and Clang generate a CMOV instead of a branch.
      ret = (static_cast<size_t>(maybeSmallSize) >= 0) ? maybeSmallSize : ret;
    } else {
      ret = (category() == Category::kSmall) ? smallSize() : ret;
    }
    return ret;
  }

  size_t capacity() const {
    switch (category()) {
      case Category::kSmall:
        return maxSmallSize;
      case Category::kLarge:
        // For large-sized strings, a multi-referenced chunk has no
        // available capacity. This is because any attempt to append
        // data would trigger a new allocation.
        if (RefCounted<TChar>::refs(ml_.data_) > 1) {
          return ml_.size_;
        }
        break;
      case Category::kMedium:
      default:
        break;
    }
    return ml_.capacity();
  }

  bool isShared() const {
    return category() == Category::kLarge && RefCounted<TChar>::refs(ml_.data_) > 1;
  }

 private:
  void CopySmall(const basic_eqi_string&);
  void CopyMedium(const basic_eqi_string&);
  void CopyLarge(const basic_eqi_string&);

  void InitSmall(const TChar* data, size_t size);
  void InitMedium(const TChar* data, size_t size);
  void InitLarge(const TChar* data, size_t size);

  void ReserveSmall(size_t minCapacity, bool disableSSO);
  void ReserveMedium(size_t minCapacity);
  void ReserveLarge(size_t minCapacity);

  void ShrinkSmall(size_t delta);
  void ShrinkMedium(size_t delta);
  void ShrinkLarge(size_t delta);

  void unshare(size_t minCapacity = 0);
  TChar* mutableDataLarge();

  size_t smallSize() const {
    DCHECK(category() == Category::kSmall);
    constexpr auto shift = base::kIsLittleEndian ? 0 : 2;
    auto smallShifted = static_cast<size_t>(small_[maxSmallSize]) >> shift;
    DCHECK(static_cast<size_t>(maxSmallSize) >= smallShifted);
    return static_cast<size_t>(maxSmallSize) - smallShifted;
  }

  Category category() const {
    // works for both big-endian and little-endian
    return static_cast<Category>(bytes_[lastTChar] & categoryExtractMask);
  }

  void SetSmallSize(size_t s) {
    // Warning: this should work with uninitialized strings too,
    // so don't assume anything about the previous value of
    // small_[maxSmallSize].
    DCHECK(s <= maxSmallSize);
    constexpr auto shift = base::kIsLittleEndian ? 0 : 2;
    small_[maxSmallSize] = TChar((maxSmallSize - s) << shift);
    small_[s] = '\0';
    DCHECK(category() == Category::kSmall && size() == s);
  }

  TChar* c_str() {
    TChar* ptr = ml_.data_;
    // With this syntax, GCC and Clang generate a CMOV instead of a branch.
    ptr = (category() == Category::kSmall) ? small_ : ptr;
    return ptr;
  }

  void reset() { SetSmallSize(0); }

  NOINLINE void destroyMediumLarge() noexcept {
    auto const c = category();
    DCHECK(c != Category::kSmall);
    if (c == Category::kMedium) {
      free(ml_.data_);
    } else {
      RefCounted<TChar>::decrementRefs(ml_.data_);
    }
  }

 private:
  constexpr static size_t lastTChar = sizeof(ExternalStorage) - 1;
  constexpr static size_t maxMediumSize = 254 / sizeof(TChar);

  using MediumLarge = ExternalStorage<TChar>;
  union {
    u8 bytes_[sizeof(MediumLarge)];  // For accessing the last byte.
    TChar small_[sizeof(MediumLarge) / sizeof(TChar)];
    MediumLarge ml_;
  };
  static_assert(!(sizeof(MediumLarge) % sizeof(TChar)), "Corrupt memory layout");
};

}  // namespace base

#include <base/strings/eqi_string.inl>