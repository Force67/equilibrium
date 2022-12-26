// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "command_line.h"

namespace base {

void CommandLine::Clear() {
  buffer_.Free();
  indices_.Free();
  arg_count_ = 0;
}
}  // namespace base