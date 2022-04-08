// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <vector>
#include <base/arch.h>
#include <base/filesystem/file.h>
#include <program_loader/file_classifier.h>

namespace program_loader {
struct ProgramData {
  struct Info {
    u64 base_addr;
    u32 entry_ea;
  } info;
  struct Segment {
    u32 start_va;
    u32 size;
    const char* name;

    enum class Flags { kRW, kR, KW } flags;
  };
  std::vector<Segment> segments;
};

// 'Loader' represents a unit that can be asked to provide specific data about
// given program
class ProgramLoader {
 public:
  virtual ~ProgramLoader() = default;
  // Can this particular loader deal with this particular form of format?
  // Using this you could have multiple PE loaders for different architectures
  // for instance this is used to have a windows PE loader, and an XBOX 360 PE loader
  virtual bool Accept(const base::Span<byte>, const FileClassificationInfo&) {
    return true;
  }
  // Parse the file into given data.
  virtual bool Parse(const base::Span<byte> data,
                     const FileClassificationInfo& intel,
                     ProgramData& out) = 0;

 private:
  // You may set this from your loader in order to share state across functions
  ProgramData* data_{nullptr};
};

// Describes the loader capeabilities.
// Make sure to export this from your module.
struct ProgramLoadDescriptor {
  const char* const name;
  const char* const desc;
  // program format we target
  Format format;
  // combined architectures supported
  Arch arch;

  enum class Flags : u32 { kInternal = 1 << 0, kFix = 1 << 1 } flags;

  // These must be null for an internal loader
  ProgramLoader* (*CreateLoader)();
  void (*DestroyLoader)(ProgramLoader*);
};

// beautiful, isn't it
inline constexpr bool operator&(ProgramLoadDescriptor::Flags lhs,
                                ProgramLoadDescriptor::Flags rhs) {
  return static_cast<bool>(static_cast<arch_types::u32>(lhs) &
                           static_cast<arch_types::u32>(rhs));
}

}  // namespace program_loader