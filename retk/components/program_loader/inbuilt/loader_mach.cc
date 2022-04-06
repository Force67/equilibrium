// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "loader_mach.h"
#include <program_loader/file_reader.h>
#include <program_loader/binary_format/marcho.h>

namespace program_loader {
static constinit ProgramLoadDescriptor kDescriptor{
    "MachO Files", "Load MacOS Programs", Format::kMachO, Arch::kX64 | Arch::kX86,
    ProgramLoadDescriptor::Flags::kInternal};

ProgramLoadDescriptor* LoaderMach::descriptor() {
  return &kDescriptor;
}

bool LoaderMach::Accept(const base::Span<byte> data, FileClassificationInfo& info) {
  FileReader reader(data);

  auto* as_mach = reader.FetchRewind<MachHeader>();
  if (as_mach->CheckMagic()) {
    info.format_type = Format::kMachO;
    info.architecture = TranslateMachMachineType(as_mach->cputype);

    return info.architecture != Arch::kNone;
  }

  return false;
}

bool LoaderMach::Parse(const base::Span<byte> data,
                      const FileClassificationInfo& intel,
                      ProgramData& out) {
  FileReader reader(data);
  reader.Advance(0);

  return false;
}

}  // namespace program_loader
