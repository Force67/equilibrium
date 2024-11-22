// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/memory/move.h>
#include <base/filesystem/file.h>

#include <base/containers/span.h>
#include <base/containers/vector.h>

#if defined(OS_WIN)
#include <base/win/scoped_handle.h>
#endif

// https://github.com/apfeltee/memorymapped/blob/master/impl.linux.cpp

namespace base {

class MemoryMappedFile {
 public:
  explicit MemoryMappedFile(base::File& file) : parent_file_(file) {}
  BASE_NOCOPYMOVE(MemoryMappedFile);

  void Close();
  bool Map();

  bool ReMap(u64 offset, mem_size mapped_bytes);

  // Reads data from the memory-mapped file.
  base::Vector<byte> Read(u64 offset, mem_size size) {
    base::Vector<byte> data;
    if (offset + size > file_size_ || !memory_view_address_) {
      // Handle error: either out of bounds or memory not mapped
      return {};
    }
    data.resize(size);
    memcpy(data.data(), static_cast<byte*>(memory_view_address_) + offset, size);
    return data;
  }

  // Writes data to the memory-mapped file.
  bool Write(u64 offset, const base::Span<byte> data) {
    if (offset + data.size() > file_size_ || !memory_view_address_) {
      return false;
    }
    memcpy(static_cast<byte*>(memory_view_address_) + offset, data.data(), data.size());
    return true;
  }

 private:
  // TODO: review order of destructors!
  base::File& parent_file_;
  mem_size file_size_{0};
  void* memory_view_address_{nullptr};

#if defined(OS_WIN)
  win::ScopedHandle memory_handle_;
#endif

#if defined(OS_POSIX)
  int fd_{0};  // implementation defined
#endif
};
}  // namespace base