// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include <base/text/code_convert.h>
#include <base/strings/string_ref.h>

#include "command_line.h"

namespace base {
void CommandLine::InitializeBuffer(const base::StringRefU8 command_line) {
  base::StringU8 current_arg;
  for (auto i = 0; i < command_line.length(); i++) {
    const auto c = command_line[i];
    switch (c) {
      case u8' ': {
        pieces_.push_back(current_arg);
        current_arg.clear();
        break;
      }
      // ignore these.
      case u8'\t':
      case u8'\n':
      case u8'\r':
        break;
      default:
        current_arg += c;
        break;
    }

    if (i == (command_line.length() - 1) && !current_arg.empty())
      pieces_.push_back(current_arg);
  }
}
}  // namespace base