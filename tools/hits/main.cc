// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Utility for matching 'hits' of patterns.

#include <base/arch.h>
#include <base/xstring.h>
using namespace arch_types;

#include "nt_executable_scanner.h"

struct PatternEntry {
  PatternEntry() = default;
  u64 former = 0;
  u64 latter = 0;
  i32 offset = 0;
  char ref_type = 0;
  char buf[512] = {};
};

using PatternBucket = std::vector<PatternEntry>;

// ea | pattern | offset | encoded reference type
void ScanCSVLine(const std::string& x, PatternBucket& buck64) {
  // ignore this shit code.
  auto &out = buck64.emplace_back();

  const std::string line = x;
  const std::string address_hex(&line[line.find_first_of("0x") + 2], 0, 9);

  out.former = std::strtoull(address_hex.c_str(), nullptr, 16);
  out.latter = 0;

  size_t pos = 12;
  size_t lp = line.find_last_of(',');

  size_t delta = lp - pos;
  std::string str = line.substr(pos, delta);
  std::strncpy(out.buf, str.c_str(), delta + 1);
  const std::string s = line.substr(lp + 1, line.length() - lp + 1);
  out.offset = std::atoi(s.c_str());
}

bool ParseCSVList(const base::Path& path, PatternBucket& out) {
  base::File file(path, base::File::FLAG_OPEN | base::File::FLAG_READ);
  if (!file.IsValid()) {
    return false;
  }

  base::XString<char> contents;
  auto length = file.Seek(base::File::Whence::FROM_END, 0);
  contents.resize(length);
  file.Seek(base::File::Whence::FROM_BEGIN, 0);
  size_t read = file.Read(0, contents.data(), length);

  // Sanitize text document
  contents.erase(std::remove_if(contents.begin(), contents.end(),
                                [](char const c) {
                                  return '\r' == c || '\0' == c || '\x1A' == c;
                                }),
                 contents.end());

  size_t last_pos = 0;

  // world's shittiest CSV parser
  for (size_t i = 0; i < contents.length(); i++) {
    if (contents[i] == '\n') {
      const base::XString<char> line(&contents[last_pos], i - last_pos);
      ScanCSVLine(line, out);
      last_pos = i + 1;
    }
  }

  return true;
}

void WriteNewMappingCSV(const base::Path& path, PatternBucket& bucket) {
  base::XString<char> buffer;

  for (PatternEntry& e : bucket) {
    if (e.latter != 0) {
        // intentional signed and unsigned mismatch
      e.latter += e.offset;
    }

    buffer += fmt::format("0x{:x},0x{:x}\n", e.former, e.latter);
  }

  constexpr auto flags = base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE;
  base::File file(path, flags);
  file.Write(0, buffer.c_str(), buffer.length());
}

int main(int argc, char** argv) {
  constexpr char kInputFileName[] =
      R"(C:\Users\vince\Documents\Development\Tilted\Reverse\diffing\patterns.csv)";
  constexpr char kOutputFileName[] =
      R"(C:\Users\vince\Documents\Development\Tilted\Reverse\diffing\out.csv)";

  const base::Path csv_path(kInputFileName);

  PatternBucket bucket;
  BUGCHECK(ParseCSVList(csv_path, bucket));

  constexpr char kExeName[] =
      R"(D:\Games\steamapps\common\Skyrim Special Edition\SkyrimSE.exe.unpacked.exe)";
  const base::Path exe_path(kExeName);
  NtExecutableScanner scanner(exe_path);

  // collide patterns
  for (PatternEntry& e : bucket) {
    auto r = scanner.Scan(e.buf);

    e.latter = r.as<u64>();
  }

  // dump out the new file
  WriteNewMappingCSV(kOutputFileName, bucket);
  return 0;
}