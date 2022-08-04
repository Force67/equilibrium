// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/logging.h>
#include <base/filesystem/memory_mapped_file.h>

namespace base {

bool MemoryMappedFile::Map() {
  file_size_ = parent_file_.GetLength();
  if (file_size_ == -1)
    return false;

  memory_handle_.Set(
      ::CreateFileMapping(parent_file_.underlying_platform_file().Get(), nullptr,
                          PAGE_READONLY, 0, 0, nullptr));

  if (!memory_handle_.IsValid())
    return false;

  return true;
}

// https://github.com/apfeltee/memorymapped/blob/master/impl.win32.cpp

bool MemoryMappedFile::ReMap(u64 offset, mem_size mapped_bytes) {
  DCHECK(!memory_handle_.IsValid(), "Attempted to remap existing view");

  // invalid offset
  if (offset > file_size_)
    return false;

  if ((offset + mapped_bytes) > file_size_)
    mapped_bytes = mem_size(file_size_ - offset);

  // get memory address
  memory_view_address_ =
      ::MapViewOfFile(memory_handle_.Get(), FILE_MAP_READ, offset >> 32,
                      offset & 0xFFFFFFFF, mapped_bytes);
  if (!memory_view_address_) {
    memory_view_address_ = nullptr;
    LOG_ERROR("MapViewOfFile() failed");
    return false;
  }

  return true;
}

void MemoryMappedFile::Close() {
  if (memory_view_address_) {
    ::UnmapViewOfFile(memory_view_address_);
  }
}
}  // namespace base