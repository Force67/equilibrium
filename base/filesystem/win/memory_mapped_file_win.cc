// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/logging.h>
#include <base/filesystem/memory_mapped_file.h>

namespace base {

bool MemoryMappedFile::Map() {
  // GetLength reports -1 on failure. Assigning it to an unsigned file_size_
  // first would turn that into the largest possible file.
  const i64 length = parent_file_.GetLength();
  if (length < 0)
    return false;
  file_size_ = static_cast<u64>(length);

  // Windows does not allocate disk space for a newly created file until it is
  // written to. Mapping a zero-length file therefore fails, so write one byte
  // first -- only when the file is empty, because the write lands on offset 0
  // and would otherwise overwrite the first byte of the file's contents.
  if (file_size_ == 0) {
    if (parent_file_.Write(0, "", 1) != 1)
      return false;
    file_size_ = 1;
  }

  auto handle = parent_file_.underlying_platform_file().Get();

  memory_handle_.Set(
      ::CreateFileMappingW(handle, nullptr, PAGE_READWRITE, 0, 0, nullptr));
  if (!memory_handle_.IsValid())
    return false;

  // Map the whole file as the initial view, the way the POSIX side does.
  // Creating the section alone left memory_view_address_ null, so Read and
  // Write rejected everything until a caller happened to know it had to ReMap
  // first. A zero length means "to the end of the section".
  memory_view_address_ = ::MapViewOfFile(memory_handle_.Get(),
                                         FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
  if (!memory_view_address_) {
    BASE_LOG_ERROR("MapViewOfFile() failed");
    return false;
  }

  view_offset_ = 0;
  view_size_ = static_cast<mem_size>(file_size_);
  return true;
}

bool MemoryMappedFile::ReMap(u64 offset, mem_size mapped_bytes) {
  // MapViewOfFile needs the section handle Map() created, so a valid handle is
  // the precondition here -- not its absence. What must not already exist is a
  // view, and one that does is dropped rather than leaked.
  BASE_DCHECK(memory_handle_.IsValid(), "Attempted to remap without a mapping");
  if (!memory_handle_.IsValid())
    return false;

  if (memory_view_address_) {
    ::UnmapViewOfFile(memory_view_address_);
    memory_view_address_ = nullptr;
    view_offset_ = 0;
    view_size_ = 0;
  }

  // invalid offset
  if (offset > file_size_)
    return false;

  // Clamped by subtraction. `(offset + mapped_bytes) > file_size_` wraps for a
  // large offset, which skips the clamp and maps past the end of the file.
  const u64 available = file_size_ - offset;
  if (mapped_bytes > available)
    mapped_bytes = static_cast<mem_size>(available);
  if (mapped_bytes == 0)
    return false;

  // get memory address
  // FILE_MAP_WRITE as well as READ: the section is PAGE_READWRITE and Write()
  // memcpys into the view, which a read-only view answers with an access
  // violation.
  memory_view_address_ = ::MapViewOfFile(
      memory_handle_.Get(), FILE_MAP_READ | FILE_MAP_WRITE,
      static_cast<DWORD>(offset >> 32), static_cast<DWORD>(offset & 0xFFFFFFFF),
      mapped_bytes);
  if (!memory_view_address_) {
    memory_view_address_ = nullptr;
    BASE_LOG_ERROR("MapViewOfFile() failed");
    return false;
  }

  view_offset_ = offset;
  view_size_ = mapped_bytes;
  return true;
}

void MemoryMappedFile::Close() {
  if (memory_view_address_) {
    ::UnmapViewOfFile(memory_view_address_);
    memory_view_address_ = nullptr;
  }
  view_offset_ = 0;
  view_size_ = 0;
}
}  // namespace base