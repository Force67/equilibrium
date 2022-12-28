// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "command_line.h"

namespace base {
CommandLine* CommandLine::current_commandline_ = nullptr;

CommandLine::CommandLine() {
  current_commandline_ = this;
  auto string = GetNativeUTF8CommandlineString();
  
  ParseFromString(string);
}

CommandLine::~CommandLine() {
  current_commandline_ = nullptr;
}

CommandLine* CommandLine::ForCurrentProcess() {
  DCHECK(current_commandline_,
         "Commandline instance pointer is invalid! (Access before/after "
         "construction?)");
  return current_commandline_;
}

void CommandLine::Clear() {
  pieces_.reset();
}

void CommandLine::ParseFromString(const base::StringRefU8 command_line) {
  if (pieces_.size())
    pieces_.reset();
  ///pieces_.clear();
  InitializeBuffer(command_line);
}

base::StringRefU8 CommandLine::operator[](const mem_size index) {
  auto cap = pieces_.size();
  BUGCHECK(index < cap, "CommandLine::operator[]: Access out of bounds");
  BUGCHECK(index < USHRT_MAX, "CommandLine::operator[]: Index out of bounds");
  const auto& piece = pieces_[index];
  return base::StringRefU8(piece.c_str(), piece.length());
}
}  // namespace base