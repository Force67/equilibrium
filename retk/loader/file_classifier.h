// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/filesystem/file.h>

namespace loader {
struct FileClassificationInfo {
  enum class Format {
    kELF,     //< Linux Elf Format,
    kXEX,     //< XBOX 360 Executable
    kPE,      //< Windows portable executable
    kMarchO,  //< MacOS executable
    kSELF,    //< Signed Elf for PS4
  } format;

  enum class Arch {
    kX86,
    kX64,
    kMips,
    kPPC,
    kARMV732,
  } arch;
};

// TODO(Force): need some span
bool ClassifyFile(const u8* buffer,
                  size_t buf_len,
                  FileClassificationInfo* info);
}  // namespace loader