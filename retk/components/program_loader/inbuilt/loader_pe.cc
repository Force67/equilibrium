// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "loader_pe.h"
#include <program_loader/file_reader.h>
#include <program_loader/binary_format/coff.h>

namespace program_loader {
namespace {
// On 64bit versions of Windows the smallest 32bit executable is 268 bytes
constexpr size_t kMinimumPEExecutableSize = 268;

inline Arch TranslateCoffMachineType(CoffMachineType machine_type) {
  switch (machine_type) {
    case CoffMachineType::kIntel86:
      return Arch::kX86;
    case CoffMachineType::kAmd64:
      return Arch::kX64;
  }
  return Arch::kNone;
}

bool ClassifyPlainPE(base::Span<byte> data, FileClassificationInfo& out) {
  if (data.length() < kMinimumPEExecutableSize)
    return false;

  FileReader reader(data);
  auto* as_dos = reader.Fetch<DOSHeader>();
  if (as_dos->magic == kDosMagic) {
    auto* coff = reader.Fetch<CoffFileHeader>();
    out.format_type = Format::kPE;
    out.architecture = TranslateCoffMachineType(coff->machine);
    return out.architecture != Arch::kNone;
  }
  return false;
}

constinit FileClassifier kClassifier{
    Format::kPE,
    Arch::kX86 | Arch::kX64,
    ClassifyPlainPE,
};

constinit ProgramLoadDescriptor kDescriptor{
    "Portable Executeable", "Load Windows Programs", Format::kPE,
    Arch::kX64 | Arch::kX86, ProgramLoadDescriptor::Flags::kInternal};
}  // namespace

ProgramLoadDescriptor* LoaderPE::descriptor() {
  return &kDescriptor;
}

FileClassifier* LoaderPE::classifier() {
    return &kClassifier;
}

bool LoaderPE::Accept(const base::Span<byte>,
                      const FileClassificationInfo& info) {
  return info.architecture == kDescriptor.arch;
}

bool LoaderPE::Parse(const base::Span<byte> data,
                     const FileClassificationInfo& intel,
                     ProgramData& out) {
  FileReader reader(data);
  reader.Advance(0);

  return false;
}

}  // namespace program_loader
