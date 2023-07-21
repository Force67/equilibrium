// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "command_line.h"
#include "numeric_limits.h"

namespace base {
CommandLine* CommandLine::current_commandline_ = nullptr;

CommandLine::CommandLine() {
  current_commandline_ = this;
#if defined(OS_WIN)
  auto string = GetNativeUTF8CommandlineString();
  ParseFromString(string);
#endif
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
  /// pieces_.clear();
  InitializeBuffer(command_line);
}

bool CommandLine::HasSwitch(const base::StringRefU8 switch_name) {
  return pieces_.Contains(switch_name.c_str());
}

base::StringRefU8 CommandLine::operator[](const mem_size index) {
  auto cap = pieces_.size();
  BUGCHECK(index < cap, "CommandLine::operator[]: Access out of bounds");
  BUGCHECK(index < base::MinMax<u16>::max(),
           "CommandLine::operator[]: Index out of bounds");
  const auto& piece = pieces_[index];
  return base::StringRefU8(piece.c_str(), piece.length());
}

size CommandLine::FindPositionalArgumentsIndex() {
  mem_size positional_index = 1;  // start at one, si nce the first arg, is the
                                  // program path itself on most platforms
  for (auto i = 1; i < pieces_.size(); i++) {
    auto& piece = pieces_[i];
    if ((piece.length() > 1 && piece.data()[0] == u8'-') ||
        (i + 1 < pieces_.size() && pieces_[i + 1].data()[0] == u8'-')) {
      positional_index++;
    }
  }
  return positional_index;
}
}  // namespace base