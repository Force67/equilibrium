// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Scan a windows portable executbale image for patterns.

#include <mem/pattern.h>
#include "nt_executable_scanner.h"

NtExecutableScanner::NtExecutableScanner(const base::Path& path) {
  LoadFromFile(path);
  BUGCHECK(buffer_);
}
NtExecutableScanner::~NtExecutableScanner() {}

uint32_t NtExecutableScanner::TranslateToVirtual(uint32_t offset) {
  for (const IMAGE_SECTION_HEADER& s : module_.section_headers()) {
    if (offset >= s.PointerToRawData &&
        offset <= s.PointerToRawData + s.SizeOfRawData) {
      return offset + s.VirtualAddress - s.PointerToRawData;
    }
  }

  return 0u;
}

bool NtExecutableScanner::CheckAndLoad() {
  if (module_.dos_header().e_magic != IMAGE_DOS_SIGNATURE)
    return false;

  // collect all code segments of image.
  module_.enum_segments([&](mem::region range, mem::prot_flags prot) {
    if (prot & mem::prot_flags::X)
      m_codeSegs.push_back(range);
    return false;
  });

  return true;
}

mem::pointer NtExecutableScanner::Scan(const std::string_view signature) {
  const mem::pattern Pattern(signature.data());

  // Scan all code segments for the pattern,
  // but return on first hit
  for (const mem::region& r : m_codeSegs) {
    if (mem::pointer p = mem::scan(Pattern, r)) {
      uint32_t offset = (p.as<char*>() - buffer_.get());

      return 0x140000000 +
          TranslateToVirtual(offset);
    }
  }

  return {};
}

void NtExecutableScanner::LoadFromFile(const base::Path& path) {
  base::File file(path, base::File::FLAG_OPEN | base::File::FLAG_READ);
  if (!file.IsValid()) {
    return;
  }

  auto length = file.Seek(base::File::Whence::FROM_END, 0);
  buffer_ = std::make_unique<char[]>(length);
  file.Seek(base::File::Whence::FROM_BEGIN, 0);
  file.Read(0, buffer_.get(), length);
  module_ = mem::module::nt(buffer_.get());

  if (!CheckAndLoad()) {
    buffer_.reset();
  }
}