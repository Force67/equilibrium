// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "loader_elf.h"
#include <program_loader/file_reader.h>
#include <program_loader/binary_format/elf.h>

namespace program_loader {
static constinit ProgramLoadDescriptor kDescriptor{
    "ELF Files", "Load ELF Programs", Format::kELF,
    Arch::kX64 | Arch::kX86, ProgramLoadDescriptor::Flags::kInternal};

ProgramLoadDescriptor* LoaderELF::descriptor() {
  return &kDescriptor;
}

bool LoaderELF::Accept(const base::Span<byte> data, FileClassificationInfo& info) {
  FileReader reader(data);

  auto* as_ehdr = reader.FetchRewind<ElfHeader>();
  if (as_ehdr->checkMagic()) {
    info.format_type = Format::kELF;

    info.architecture = TranslateElfMachineType(as_ehdr->machine);
    return info.architecture != Arch::kNone;
  }

  return false;
}

bool LoaderELF::Parse(const base::Span<byte> data,
                     const FileClassificationInfo& intel,
                     ProgramData& out) {
  FileReader reader(data);
  reader.Advance(0);

  return false;
}

}  // namespace program_loader
