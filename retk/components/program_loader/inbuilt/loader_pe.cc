// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "loader_pe.h"
#include <program_loader/file_reader.h>
#include <program_loader/binary_format/coff.h>

namespace program_loader {
static constinit ProgramLoadDescriptor kDescriptor{
    "Portable Executeable", "Load Windows Programs", Format::kPE,
    Arch::kX64 | Arch::kX86, ProgramLoadDescriptor::Flags::kInternal};

// On 64bit versions of Windows the smallest 32bit executable is 268 bytes
constexpr size_t kMinimumPEExecutableSize = 268;

ProgramLoadDescriptor* LoaderPE::descriptor() {
  return &kDescriptor;
}

bool LoaderPE::Accept(const base::Span<byte> data, FileClassificationInfo& info) {
  if (data.length() < kMinimumPEExecutableSize)
    return false;

  FileReader reader(data);
  auto* as_dos = reader.FetchRewind<DOSHeader>();
  if (as_dos->magic == kDosMagic) {
    info.format_type = Format::kPE;

    auto* coff = reader.Fetch<CoffFileHeader>();
    info.architecture = TranslateCoffMachineType(coff->machine);

    return info.architecture != Arch::kNone;
  }

  return false;
}

bool LoaderPE::Parse(const base::Span<byte> data,
                     const FileClassificationInfo& intel,
                     ProgramData& out) {
  FileReader reader(data);
  reader.Advance(0);


  return false;
}

}  // namespace program_loader
