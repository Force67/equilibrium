// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Options files: a text file of option assignments that overrides base::Option
// defaults, so a long debugging configuration lives in a file instead of on the
// command line. Apply one at startup, after static initialization and before
// any subsystem reads an option.
//
// One entry per line, in the order they appear (a name assigned twice keeps the
// last value):
//
//   // comment, as is a line starting with #
//   +Name=Value      set Name to Value
//   Name=Value       same, the + is optional
//   +Name            set Name to 1 (for a bool option: on)
//   -Name            put Name back on its compiled-in default
//
// Values may be quoted ("a value with spaces"); surrounding quotes are dropped.
// Names match an option's own name or the environment variable it declares, and
// are case sensitive. A name several modules declare separately resolves to all
// of them: one entry sets every option that goes by it.
#pragma once

#include <base/arch.h>
#include <base/export.h>
#include <base/filesystem/path.h>
#include <base/strings/string_ref.h>
#include <base/strings/xstring.h>

namespace base {

class OptionBase;

// What applying a set of entries did. Report `unknown` and `invalid` back to
// whoever wrote the file: a mistyped name is otherwise silently ignored.
struct OptionFileResult {
  mem_size applied = 0;  // entries that set an option
  mem_size unknown = 0;  // entries naming no registered option
  mem_size invalid = 0;  // malformed lines, or values the option rejected
  bool read = false;     // the file existed and could be read
};

// The registered option going by `name`, either as its own name or as the
// environment variable it declares. Null when nothing matches.
BASE_EXPORT OptionBase* FindOption(const StringRef name);

// Assigns one option by name. The value is copied into storage that lives for
// the rest of the process, so an Option<const char*> keeps pointing at it.
BASE_EXPORT bool SetOptionValue(const StringRef name, const StringRef value);

// Applies every entry in the text / in the file at `path`.
BASE_EXPORT OptionFileResult ApplyOptionText(const StringRef text);
BASE_EXPORT OptionFileResult ApplyOptionFile(const Path& path);

// Appends every registered option as an entry this parser reads back, each
// preceded by its description. With `overridden_only` the dump holds just the
// options something actually set, i.e. the difference from a default run.
BASE_EXPORT void AppendOptionText(String& out, bool overridden_only = false);
BASE_EXPORT bool WriteOptionFile(const Path& path, bool overridden_only = false);

}  // namespace base
