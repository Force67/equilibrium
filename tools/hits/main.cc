// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Utility for matching 'hits' of patterns.

#include <base/arch.h>
#include <base/xstring.h>
using namespace arch_types;

#include "nt_executable_scanner.h"

struct PatternEntry {
  u64 former = 0;
  u64 latter = 0;
  i32 offset = 0;
  base::XString<char> pattern;
};

using PatternBucket = std::vector<PatternEntry>;

bool ParseCSVList(const base::FilePath& path, PatternBucket& out) {
  base::File file(path, false);
  if (!file.IsValid()) {
    return false;
  }

  base::XString<char> contents;
  auto length = file.Seek(base::File::Whence::FROM_END, 0);
  contents.resize(length);
  file.Seek(base::File::Whence::FROM_BEGIN, 0);
  file.Read(0, contents.data(), length);

  size_t last_pos = 0;
  size_t depth = 0;

  // world's shittiest CSV parser
  PatternEntry& e = out.emplace_back();
  for (size_t i = 0; i < contents.length(); i++) {
    const char c = contents[i];
    if (c == ',') {
      if (depth == 0)
        e.former = _atoi64(&contents[i - last_pos]);
      if (depth == 1)
        e.pattern = &contents[i - last_pos];
      if (depth == 2)
        e.offset = std::atoi(&contents[i - last_pos]);

      last_pos = i;
      depth++;
    }
    if (c == '\n') {
      last_pos = 0;
      depth = 0;

      e = out.emplace_back();
    }
  }

  return true;
}

void WriteNewMappingCSV(const PatternBucket& bucket) {
  for (const auto& it : bucket) {

  }
}

int main(int argc, char** argv) {
  constexpr char kFileName[] =
      R"(C:\Users\vince\Documents\Development\Tilted\Reverse\diffing\test.txt)";
  const base::FilePath csv_path(kFileName);

  std::vector<PatternEntry> entries;
  TK_BUGCHECK(ParseCSVList(csv_path, entries));

  constexpr char kExeName[] = "";
  const base::FilePath exe_path(kExeName);
  NtExecutableScanner scanner(exe_path);

  for (const PatternEntry& e : entries) {
    auto r = scanner.Scan(e.pattern);
    __debugbreak();
  }

  return 0;
}