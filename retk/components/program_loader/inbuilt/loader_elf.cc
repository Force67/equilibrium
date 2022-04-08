// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "loader_elf.h"
#include <program_loader/file_reader.h>
#include <program_loader/binary_format/elf.h>

namespace program_loader {
namespace {

inline Arch TranslateElfMachineType(ElfMachineType type) {
  switch (type) {
    case ElfMachineType::k386:
      return Arch::kX86;
    case ElfMachineType::kX86_64:
      return Arch::kX64;
  }
  return Arch::kNone;
}

bool ClassifyPlainELF(base::Span<byte> data, FileClassificationInfo& out) {
  FileReader reader(data);

  auto* as_ehdr = reader.Fetch<ElfHeader>();
  if (as_ehdr->checkMagic()) {
    out.format_type = Format::kELF;
    out.architecture = TranslateElfMachineType(as_ehdr->machine);
    return out.architecture != Arch::kNone;
  }

  return false;
}

constinit FileClassifier kClassifier{
    Format::kELF,
    Arch::kX86 | Arch::kX64,
    ClassifyPlainELF,
};

constinit ProgramLoadDescriptor kDescriptor{"ELF Files", "Load ELF Programs",
                                            Format::kELF, Arch::kX64 | Arch::kX86,
                                            ProgramLoadDescriptor::Flags::kInternal};
}  // namespace

ProgramLoadDescriptor* LoaderELF::descriptor() {
  return &kDescriptor;
}

FileClassifier* LoaderELF::classifier() {
  return &kClassifier;
}

bool LoaderELF::Accept(const base::Span<byte> data,
                       const FileClassificationInfo& info) {
  return true;
}

bool LoaderELF::Parse(const base::Span<byte> data,
                      const FileClassificationInfo& intel,
                      ProgramData& out) {
  FileReader reader(data);
  reader.Advance(0);

  return false;
}

}  // namespace program_loader
