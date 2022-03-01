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
  // If the loader wants to consider given file.
  virtual bool Accept(const base::File&) { return false; }

  // Parse the file into given data.
  virtual bool Parse(const FileClassificationInfo& intel,
                     ProgramData& out,
                     const u8* ptr) = 0;

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
}  // namespace program_loader