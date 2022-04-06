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

  template <typename T>
  T* Fetch(size_t at_offset = 0) {
    T* head = nullptr;
    return Read(reinterpret_cast<void*>(head), sizeof(T)) ? head : nullptr;
  }

  template <typename T>
  T* FetchRewind(size_t at_offset = 0) {
    T* head = nullptr;
    pos_ = 0;
    return Read(reinterpret_cast<void*>(head), sizeof(T)) ? head : nullptr;
  }

  void Advance(size_t pos) { pos_ += pos; }

 private:
  bool Read(byte* new_head, size_t n) {
    pos_ += n;
    if (pos_ > buffer_.size())
      return false;

    new_head = &buffer_[pos_];
    return true;
  }

 private:
  size_t pos_ = 0;
  base::Span<byte> buffer_;
};
}  // namespace program_loader