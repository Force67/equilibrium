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

  // from argc
  CommandLine(int argc, char** argv);

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

  // For convenience, initializes from pieces provided by a regular main(argc, argv)
  void FromArray(int count, char** args);

  // reset internal buffers and counters.
  void Clear();

  // get an argument at a given index, note that index 0 is usually the executable
  // path
  base::StringRefU8 operator[](const mem_size index);

  // simply checks if the command line has exactly these contents, somewhere
  bool HasItem(const base::StringRefU8 contents);

  // tries to match a switch in the command line, switches can either start with
  // - or -- and can contain values. 
  // if found, returns the index in the pieces_ array.
  // if not found, returns -1
  i32 HasSwitch(const base::StringRefU8 switch_name);

  // Get the value for a given switch; returns an empty string if the switch isn't
  // found or has no value these switches usually start with - or -- and the
  // parameter is provided by --myswitch=myvalue
  base::StringU8 ExtractSwitchValue(const base::StringRefU8 switch_name);

  // Finds the index for when positonal arguments start, usually after the optional
  // arguments
  xsize FindPositionalArgumentsIndex();

  const xsize parameter_count() { return pieces_.size(); }

  // cxx iterator
  auto begin() { return pieces_.begin(); }
  auto end() { return pieces_.end(); }

 private:
  void InitializeBuffer(const base::StringRefU8);

 private:
  base::Vector<base::StringU8> pieces_;

  static CommandLine* current_commandline_;

  BASE_NOCOPYMOVE(CommandLine);
};
}  // namespace base
