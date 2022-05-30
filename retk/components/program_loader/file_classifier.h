// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/enum_traits.h>
#include <base/containers/span.h>
#include <base/filesystem/file.h>

namespace program_loader {
enum class Format : u16 {
  kELF,    //< Linux Elf Format,
  kXEX,    //< XBOX 360 Executable
  kPE,     //< Windows portable executable
  kMachO,  //< MacOS executable
  kSELF,   //< Signed Elf for PS4
};

enum class Arch : u32 {
  kNone,
  kX86 = 1 << 0,
  kX64 = 1 << 1,
  kMips = 1 << 2,
  kPPC = 1 << 3,
  kARMV732 = 1 << 4,
  kARMV764 = 1 << 5,
  kARMV832 = 1 << 6,
  kARMV864 = 1 << 7,
};
BASE_IMPL_ENUM_BIT_TRAITS(Arch, u32)

inline bool Is64BitArchitecture(Arch arch) {
  switch (arch) {
    case Arch::kARMV764:
    case Arch::kARMV864:
    case Arch::kX64:
      return true;
    default:
      return false;
  }
}
// Prototype for private data.
struct PrivateData {};

// Findings about given file.
struct FileClassificationInfo {
  Format format_type;
  bool is_complex_data;  // is the union below pointing to any private data.
  bool pad4;
  Arch architecture{Arch::kNone};

  union {
    PrivateData* data;
    size_t numeric;
  };
};

struct FileClassifier {
  Format format;
  Arch arch;
  // this function must be implemented, or the classifier will be ignored
  bool (*classify)(base::Span<byte>, FileClassificationInfo&) = nullptr;
};

// this will try to determine some common format.
bool ClassifyFile(base::Span<byte> buffer, FileClassificationInfo& out);
}  // namespace program_loader