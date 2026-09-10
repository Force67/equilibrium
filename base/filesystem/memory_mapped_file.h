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

  [[nodiscard]] u64 file_size() const { return file_size_; }

  // Byte range of the file that the current view covers. Read and Write take
  // file offsets, and this range is what bounds them: after a partial ReMap the
  // file stays larger than the view, so the file size bounds nothing.
  [[nodiscard]] u64 view_offset() const { return view_offset_; }
  [[nodiscard]] mem_size view_size() const { return view_size_; }

  // Reads |size| bytes at file offset |offset|. Returns an empty vector when
  // that range is not wholly inside the mapped view.
  base::Vector<byte> Read(u64 offset, mem_size size) {
    const byte* source = AddressFor(offset, size);
    if (!source)
      return {};
    base::Vector<byte> data;
    data.resize(size);
    memcpy(data.data(), source, size);
    return data;
  }

  // Writes |data| at file offset |offset|. Fails when that range is not wholly
  // inside the mapped view.
  bool Write(u64 offset, const base::Span<byte> data) {
    byte* target = AddressFor(offset, data.size());
    if (!target)
      return false;
    memcpy(target, data.data(), data.size());
    return true;
  }

 private:
  // Address of the |size|-byte range at file offset |offset|, or null when the
  // range falls outside the view. Every comparison here is a subtraction:
  // `offset + size > limit` wraps for an offset near the top of u64 and lets an
  // arbitrary out-of-bounds range through to the memcpy.
  byte* AddressFor(u64 offset, mem_size size) const {
    if (!memory_view_address_ || offset < view_offset_)
      return nullptr;
    const u64 relative = offset - view_offset_;
    if (relative > view_size_ || size > view_size_ - relative)
      return nullptr;
    return static_cast<byte*>(memory_view_address_) + relative;
  }

  // TODO: review order of destructors!
  base::File& parent_file_;
  u64 file_size_{0};
  void* memory_view_address_{nullptr};
  u64 view_offset_{0};
  mem_size view_size_{0};

#if defined(OS_WIN)
  win::ScopedHandle memory_handle_;
#endif

#if defined(OS_POSIX)
  // -1 rather than 0, so a Close() before any Map() cannot close stdin.
  int fd_{-1};  // implementation defined
#endif
};
}  // namespace base