// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "loader_mach-o.h"
#include <program_loader/file_reader.h>
#include <program_loader/binary_format/mach-o.h>

namespace program_loader {
namespace {

inline Arch TranslateMachMachineType(u32 type) {
  switch (static_cast<MachCPUType>(type)) {
    case MachCPUType::kX86:
      return Arch::kX86;
    case MachCPUType::kX86_64:
      return Arch::kX64;
  }
  return Arch::kNone;
}

bool ClassifyPlainMach(base::Span<byte> data, FileClassificationInfo& out) {
  FileReader reader(data);

  auto* as_mach = reader.Fetch<MachHeader>();
  if (as_mach->CheckMagic()) {
    out.format_type = Format::kMachO;
    out.architecture = TranslateMachMachineType(as_mach->cputype);

    return out.architecture != Arch::kNone;
  }
  return false;
}

constinit FileClassifier kClassifier{
    Format::kMachO,
    Arch::kX86 | Arch::kX64,
    ClassifyPlainMach,
};

constinit ProgramLoadDescriptor kDescriptor{"MachO Files", "Load MacOS Programs",
                                            Format::kMachO, Arch::kX64 | Arch::kX86,
                                            ProgramLoadDescriptor::Flags::kInternal};

}  // namespace

ProgramLoadDescriptor* LoaderMachO::descriptor() {
  return &kDescriptor;
}

FileClassifier* LoaderMachO::classifier() {
  return &kClassifier;
}

bool LoaderMachO::Accept(const base::Span<byte> data,
                         const FileClassificationInfo& info) {
  return true;
}

bool LoaderMachO::Parse(const base::Span<byte> data,
                        const FileClassificationInfo& intel,
                        ProgramData& out) {
  FileReader reader(data);
  reader.Advance(0);

  return false;
}

}  // namespace program_loader
