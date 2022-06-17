// Copyright (c) 2006-2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <base/export.h>
#include <base/filesystem/file.h>
#include <base/filesystem/scoped_file.h>

namespace base {

class File;

constexpr int kInvalidFd = -1;

// -----------------------------------------------------------------------------
// We introduct a special structure for file descriptors in order that we are
// able to use template specialisation to special-case their handling.
//
// IMPORTANT: This is primarily intended for use when sending file descriptors
// over IPC. Even if |auto_close| is true, base::FileDescriptor does NOT close()
// |fd| when going out of scope. Instead, a consumer of a base::FileDescriptor
// must invoke close() on |fd| if |auto_close| is true.
//
// In the case of IPC, the IPC subsystem knows to close() |fd| after sending
// a message that contains a base::FileDescriptor if auto_close == true. On the
// other end, the receiver must make sure to close() |fd| after it has finished
// processing the IPC message. See the IPC::ParamTraits<> specialization in
// ipc/ipc_message_utils.h for all the details.
// -----------------------------------------------------------------------------
struct BASE_EXPORT FileDescriptor {
  FileDescriptor() = default;
  FileDescriptor(int ifd, bool iauto_close);
  explicit FileDescriptor(File file);
  explicit FileDescriptor(ScopedFD fd);

  bool operator==(const FileDescriptor& other) const;
  bool operator!=(const FileDescriptor& other) const;

  // A comparison operator so that we can use these as keys in a std::map.
  bool operator<(const FileDescriptor& other) const;

  int fd = kInvalidFd;

  // If true, this file descriptor should be closed after it has been used. For
  // example an IPC system might interpret this flag as indicating that the
  // file descriptor it has been given should be closed after use.
  bool auto_close = false;
};

inline FileDescriptor::FileDescriptor(int ifd, bool iauto_close)
    : fd(ifd), auto_close(iauto_close) {}

inline FileDescriptor::FileDescriptor(File file)
    : fd(file.TakePlatformFile()), auto_close(true) {}

inline FileDescriptor::FileDescriptor(ScopedFD fd)
    : fd(fd.release()), auto_close(true) {}

inline bool FileDescriptor::operator==(const FileDescriptor& other) const {
  return fd == other.fd && auto_close == other.auto_close;
}

inline bool FileDescriptor::operator!=(const FileDescriptor& other) const {
  return !operator==(other);
}

inline bool FileDescriptor::operator<(const FileDescriptor& other) const {
  return other.fd < fd;
}

}  // namespace base