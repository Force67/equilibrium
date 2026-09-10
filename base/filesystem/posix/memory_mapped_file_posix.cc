// Copyright (C) 2024 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <base/filesystem/memory_mapped_file.h>
#include <base/logging.h>
#include <cerrno>
#include <cstring>
#include "logging.h"

namespace base {
bool MemoryMappedFile::Map() {
  // Example assumes parent_file_ provides a method to get the file path
  fd_ = open((char*)parent_file_.path().c_str(), O_RDWR);
  if (fd_ == -1) {
    BASE_LOG_ERROR("Error opening file: {}", strerror(errno));
    return false;
  }

  struct stat st;
  if (fstat(fd_, &st) == -1) {
    BASE_LOG_ERROR("Error getting file size: {}", strerror(errno));
    return false;
  }
  file_size_ = st.st_size;

  if (file_size_ == 0) {
    // Ensure the file has a non-zero size as in the original example
    if (write(fd_, "", 1) != 1) {
      BASE_LOG_ERROR("Error writing to file: {}", strerror(errno));
      return false;
    }
    file_size_ = 1;
  }

  memory_view_address_ =
      mmap(nullptr, file_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
  if (memory_view_address_ == MAP_FAILED) {
    BASE_LOG_ERROR("Error remapping file: {}", strerror(errno));
    memory_view_address_ = nullptr;
    return false;
  }

  view_offset_ = 0;
  view_size_ = file_size_;
  return true;
}

bool MemoryMappedFile::ReMap(u64 offset, mem_size mapped_bytes) {
  if (memory_view_address_ != nullptr) {
    // The length has to be the view's own, not the file's: they differ after
    // any partial ReMap, and munmap with the wrong length unmaps the wrong
    // range.
    munmap(memory_view_address_, view_size_);
    memory_view_address_ = nullptr;
    view_offset_ = 0;
    view_size_ = 0;
  }

  if (offset > file_size_) {
    return false;
  }

  // Clamped by subtraction. `offset + mapped_bytes > file_size_` wraps for a
  // large offset, which skips the clamp and maps past the end of the file.
  const u64 available = file_size_ - offset;
  if (mapped_bytes > available) {
    mapped_bytes = static_cast<mem_size>(available);
  }
  if (mapped_bytes == 0) {
    return false;  // mmap rejects a zero-length mapping.
  }

  memory_view_address_ = mmap(nullptr, mapped_bytes, PROT_READ | PROT_WRITE,
                              MAP_SHARED, fd_, static_cast<off_t>(offset));
  if (memory_view_address_ == MAP_FAILED) {
    BASE_LOG_ERROR("Error remapping file: {}", strerror(errno));
    memory_view_address_ = nullptr;
    return false;
  }

  view_offset_ = offset;
  view_size_ = mapped_bytes;
  return true;
}

void MemoryMappedFile::Close() {
  if (memory_view_address_) {
    munmap(memory_view_address_, view_size_);
    memory_view_address_ = nullptr;
  }
  view_offset_ = 0;
  view_size_ = 0;
  if (fd_ != -1) {
    close(fd_);
    fd_ = -1;
  }
}
}  // namespace base