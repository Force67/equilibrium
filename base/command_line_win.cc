// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/check.h>
#include <base/text/code_convert.h>
#include <base/strings/string_ref.h>

#include "command_line.h"

extern "C" __declspec(dllimport) const wchar_t* __stdcall GetCommandLineW();

namespace base {
// https://github.com/futurist/CommandLineToArgvA/blob/master/CommandLineToArgvA.c#L259

CommandLine::CommandLine() {
  const wchar_t* command_line = ::GetCommandLineW();
  DCHECK(command_line, "GetCommandLineW failed. This should never happen!");
  InitFromWideCommandline(command_line);
}

// member function for unit testing
void CommandLine::InitFromWideCommandline(const wchar_t* wide_command_line) {
  auto utf8_command_line = base::WideToUTF8(wide_command_line);

  const mem_size utf8_command_line_length = utf8_command_line.length();
  // we cant be bigger than the source, as we don't append new content, the source
  // already contains spaces too.
  buffer_ = base::MakeUnique<char8_t[]>(utf8_command_line_length);
  memset(buffer_.Get_UseOnlyIfYouKnowWhatYouareDoing(), 0, utf8_command_line_length);

  // TODO: is there a bug in there with utf8 character length? this needs unit
  // testing.
  mem_size buffer_index = 0;
  for (mem_size i = 0; i < utf8_command_line_length; i++) {
    switch (utf8_command_line[i]) {
        // split at space.
      case u8' ': {
        // since we zero the whole buffer before we dont even have to put a null
        // character.
        buffer_index++;
        arg_count_++;
        break;
      }
      case u8'\t':
      case u8'\r':
      case u8'\"':
        break;
      default: {
        buffer_[buffer_index] = utf8_command_line[i];
        buffer_index++;
        break;
      }
    }
  }

  indices_ = base::MakeUnique<u16[]>(arg_count_);

  mem_size j = 0;
  mem_size k = 0;
  for (mem_size i = 0; i < buffer_index; i++) {
    if (buffer_[i] == 0) {
      if (i + 1 <= buffer_index) {
        if (buffer_[i + 1] == 0)
          continue;
        if (i != (buffer_index - 1))
          continue;
      }

      indices_[j] = i - k;
      j++;
      k = i;
    }
  }
}

base::StringRefU8 CommandLine::operator[](const mem_size index) {
  BUGCHECK(index < arg_count_, "CommandLine::operator[]: Access out of bounds");
  u16 size = indices_[index];
  return base::StringRefU8(&buffer_[size * (index - 1)], size);
}
}  // namespace base