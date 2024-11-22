// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/logging.h>
#include <base/filesystem/memory_mapped_file.h>

namespace base {

bool MemoryMappedFile::Map() {
  file_size_ = parent_file_.GetLength();
  if (file_size_ == -1)
    return false;

  // weird af: In Windows, when a file is created, it does not automatically
  // allocate space on the disk unless it's written to. Therefore, trying to map
  // a newly created but empty (zero-length) file will fail because
  // memory-mapped files require the underlying file to have a non-zero size.
  parent_file_.Write(0, "", 1);

  auto handle = parent_file_.underlying_platform_file().Get();

  memory_handle_.Set(
      ::CreateFileMappingW(handle, nullptr, PAGE_READWRITE, 0, 0, nullptr));

  return memory_handle_.IsValid();
}

bool MemoryMappedFile::ReMap(u64 offset, mem_size mapped_bytes) {
  DCHECK(!memory_handle_.IsValid(), "Attempted to remap existing view");

  // invalid offset
  if (offset > file_size_)
    return false;

  if ((offset + mapped_bytes) > file_size_)
    mapped_bytes = mem_size(file_size_ - offset);

  // get memory address
  memory_view_address_ = ::MapViewOfFile(memory_handle_.Get(), FILE_MAP_READ,
                                         offset >> 32, offset & 0xFFFFFFFF, mapped_bytes);
  if (!memory_view_address_) {
    memory_view_address_ = nullptr;
    BASE_LOG_ERROR("MapViewOfFile() failed");
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