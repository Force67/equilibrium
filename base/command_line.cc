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

CommandLine::CommandLine(int argc, char** argv) {
  current_commandline_ = this;
  FromArray(argc, argv);
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

void CommandLine::FromArray(int count, char** args) {
  for (int i = 0; i < count; i++) {
    char* arg = args[i];
    const base::StringU8 arg_str(reinterpret_cast<char8_t*>(arg));
    pieces_.push_back(arg_str);
  }
}

bool CommandLine::HasItem(const base::StringRefU8 switch_name) {
  return pieces_.Contains(base::StringU8(switch_name.c_str()));
}

i32 CommandLine::FindSwitchIndex(const base::StringRefU8 switch_name) {
  for (i32 i = 0; i < pieces_.size(); i++) {
    base::StringU8& piece = pieces_[i];
    // yes intentional, easier to debug.
    if (piece.size() < switch_name.size())
      continue;
    if (piece[0] != u8'-')
      continue;
    if (piece[0] != u8'-' && piece[1] != u8'-')
      continue;
    auto matches = [&](mem_size offset) {
      return piece.compare(offset, switch_name.length(), switch_name.data()) == 0;
    };
    if (matches(1) || matches(2)) {
      return i;
    }
  }
  return kNotFoundIndex;
}

base::StringRefU8 CommandLine::ExtractSwitchValue(const base::StringRefU8 item_contents) {
  // Find the '=' sign, if present
  mem_size eq_pos = item_contents.find(u8"=", 0, 1);
  if (eq_pos != base::StringRefU8::npos)
    return item_contents.subslice(eq_pos + 1);  // Return the value after '='
  return u8"";                                  // Default return: switch not found
}

base::StringRefU8 CommandLine::operator[](const mem_size index) CONST_ND {
  const mem_size cap = pieces_.size();
  BUGCHECK(index < cap, "CommandLine::operator[]: Access out of bounds");
  BUGCHECK(index < base::MinMax<u16>::max(),
           "CommandLine::operator[]: Index out of bounds");
  const auto& piece = pieces_[index];
  return base::StringRefU8(piece.c_str(), piece.length(),
                           true);  // No +1 as nterm doesnt count as character
}

base::StringRefU8 CommandLine::at(const mem_size index) {
  auto cap = pieces_.size();
  if (index > cap || index == kNotFoundIndex)
    return u8"";
  const auto& piece = pieces_[index];
  return base::StringRefU8(piece.c_str(), piece.length(),
                           true);  // No +1 as nterm doesnt count as character
}

xsize CommandLine::FindPositionalArgumentsIndex() {
  mem_size positional_index = 1;  // start at one, since the first arg, is the
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
