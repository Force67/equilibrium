// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
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
};

// For concentration of flags
inline consteval Arch operator|(Arch lhs, Arch rhs) {
  return static_cast<Arch>(static_cast<arch_types::u32>(lhs) |
                           static_cast<arch_types::u32>(rhs));
}

// Protoype for private data.
struct PrivateData {};

// Findings about given file.
struct FileClassificationInfo {
  Format format_type;
  Arch architecture{Arch::kNone};
  // Data specific to the file format, to be consumed by the loader.
  std::unique_ptr<PrivateData> data;
};

struct FileClassifier {
  Format format;
  Arch arch;
  // this function must be implemented, or the classifier will be ignored
  bool (*classify)(base::Span<byte>, FileClassificationInfo&) = nullptr;
};

// this will try to determine some common format.
bool ClassifyFile(base::Span<byte> buffer, FileClassificationInfo* info);
}  // namespace program_loader