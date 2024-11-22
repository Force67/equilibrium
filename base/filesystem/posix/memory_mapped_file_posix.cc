// Copyright (C) 2024 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <base/filesystem/memory_mapped_file.h>
#include <base/logging.h>
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

  return true;
}

bool MemoryMappedFile::ReMap(u64 offset, mem_size mapped_bytes) {
  if (memory_view_address_ != nullptr) {
    munmap(memory_view_address_, file_size_);
  }

  if (offset > file_size_) {
    return false;
  }

  if (offset + mapped_bytes > file_size_) {
    mapped_bytes = file_size_ - offset;
  }

  memory_view_address_ =
      mmap(nullptr, mapped_bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, offset);
  if (memory_view_address_ == MAP_FAILED) {
    BASE_LOG_ERROR("Error remapping file: {}", strerror(errno));
    memory_view_address_ = nullptr;
    return false;
  }

  return true;
}

void MemoryMappedFile::Close() {
  if (memory_view_address_) {
    munmap(memory_view_address_, file_size_);
    memory_view_address_ = nullptr;
  }
  if (fd_ != -1) {
    close(fd_);
    fd_ = -1;
  }
}
}  // namespace base