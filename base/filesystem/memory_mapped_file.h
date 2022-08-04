// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/memory/move.h>
#include <base/filesystem/file.h>

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
 private:
	 // TODO: review order of destructors!
  base::File& parent_file_;
  mem_size file_size_{0};

#if defined(OS_WIN)
  win::ScopedHandle memory_handle_;
  void* memory_view_address_{nullptr};
#endif
};
}  // namespace base