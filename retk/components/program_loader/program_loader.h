// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <vector>
#include <base/arch.h>
#include <base/filesystem/file.h>
#include <program_loader/file_classifier.h>

namespace program_loader {
struct ProgramData {
  // we use 64 bit here everywhere, since ELF uses direct addressing on 64 bit
  struct Info {
    u64 base_addr;
    u64 entry_ea;
  } info;
  struct Segment {
    u64 start_va;
    u64 disk_size;
    u64 mem_size;
    base::String name;
    enum class Flags { kNone = 0, kX = 1, kW = 2, kR = 4 } flags;
  };
  std::vector<Segment> segments;
};

inline consteval ProgramData::Segment::Flags operator|(
    ProgramData::Segment::Flags lhs,
    ProgramData::Segment::Flags rhs) {
  return static_cast<ProgramData::Segment::Flags>(static_cast<arch_types::u32>(lhs) |
                                                  static_cast<arch_types::u32>(rhs));
}

// This sucks, and in the future we should use a bitset class.
inline ProgramData::Segment::Flags operator|=(ProgramData::Segment::Flags& lhs,
                                              ProgramData::Segment::Flags rhs) {
  auto new_val = reinterpret_cast<arch_types::u32*>(&lhs);
  *new_val |= static_cast<arch_types::u32>(rhs);
  return static_cast<ProgramData::Segment::Flags>(*new_val);
}

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