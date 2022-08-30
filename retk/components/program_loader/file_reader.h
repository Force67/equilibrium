// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/check.h>
#include <base/containers/span.h>

namespace program_loader {

class FileReader {
 public:
  FileReader(base::Span<byte> buffer) : buffer_(buffer) {}

  void AdvanceFromStart(size_t pos) { pos_ = pos;
  }

  template <typename T>
  T* Fetch() {
    T* head = nullptr;
    return Read(reinterpret_cast<byte**>(&head), sizeof(T)) ? head : nullptr;
  }

  template <typename T>
  T* FetchNoAdvance() {
    T* head = nullptr;
    return Read(reinterpret_cast<byte**>(&head), sizeof(T), false) ? head : nullptr;
  }

  template <typename T>
  T* FetchRewind(size_t at_offset = 0) {
    T* head = nullptr;
    pos_ = 0;
    return Read(reinterpret_cast<byte**>(&head), sizeof(T)) ? head : nullptr;
  }

  void Advance(size_t pos) { pos_ += pos; }

 private:
  bool Read(byte** new_head, size_t n, bool advance = true) {
      #if 0
    if (pos_ > buffer_.size())
      return false;
    *new_head = &buffer_[pos_];
    if (advance)
        pos_ += n;
    #endif
    __debugbreak();
    return true;
  }

 private:
  size_t pos_ = 0;
  base::Span<byte> buffer_;
};
}  // namespace program_loader