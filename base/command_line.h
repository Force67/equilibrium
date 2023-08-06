// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/memory/unique_pointer.h>
#include <base/strings/string_ref.h>
#include <base/memory/move.h>
#include <base/containers/vector.h>

#include <vector>

namespace base {
// be careful, this class is designed to live throughout the entire runtime of the
// application
class CommandLine {
 public:
  // tries to fetch cmdl itself.
  CommandLine();

  // resets the current - per process - commandline aswell, after this calls to
  // ForCurrentProcess will DCHECK
  ~CommandLine();

  // Get the singleton CommandLine representing the current process's
  // command line. Note: returned value is mutable, but not thread safe;
  // only mutate if you know what you're doing!
  static CommandLine* ForCurrentProcess();

  // Gets the command line string, as provided by the underlying operating system.
  base::StringU8 GetNativeUTF8CommandlineString();

  // Initialze from a wide windows command line, such as the one returned by calling
  // ::GetCommandlineW() or the one passed to WinMain.
  void ParseFromString(const base::StringRefU8 command_line);

  // reset internal buffers and counters.
  void Clear();

  // get an argument at a given index, note that index 0 is the executable path
  base::StringRefU8 operator[](const mem_size index);

  // tries to match a switch in the command line, returns true if found.
  bool HasSwitch(const base::StringRefU8 switch_name);
  //bool HasSwitch(const char* switch_name);

  xsize FindPositionalArgumentsIndex();

  const xsize parameter_count() { return pieces_.size(); }

  // cxx iterator
  auto begin() { return pieces_.begin(); }
  auto end() { return pieces_.end(); }

 private:
  void InitializeBuffer(const base::StringRefU8);

 private:
  base::Vector<base::StringU8> pieces_;
  //std::vector<base::StringU8> pieces_;

  static CommandLine* current_commandline_;

  BASE_NOCOPYMOVE(CommandLine);
};
}  // namespace base
