// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/memory/unique_pointer.h>
#include <base/strings/string_ref.h>
#include <base/memory/move.h>

namespace base {

// https://source.chromium.org/chromium/chromium/src/+/main:base/command_line.h
class CommandLine {
 public:
#if defined(OS_WIN)
  CommandLine();
#endif
  ~CommandLine() {}

#if defined(OS_WIN)
  void InitFromWideCommandline(const wchar_t* command_line);
#endif

  void Clear();

  base::StringRefU8 operator[](const mem_size index);

 private:
  base::UniquePointer<char8_t[]> buffer_;
  base::UniquePointer<u16[]> indices_;
  mem_size arg_count_{0};

  BASE_NOCOPYMOVE(CommandLine);
};
}  // namespace base
