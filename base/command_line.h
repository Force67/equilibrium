// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

namespace base {

// https://source.chromium.org/chromium/chromium/src/+/main:base/command_line.h
class CommandLine {
 public:
  CommandLine(const wchar_t* command_line, const mem_size arg_count){};
  ~CommandLine() {}
};
}  // namespace base
