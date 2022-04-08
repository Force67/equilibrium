// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "loader_pe.h"
#include <program_loader/file_reader.h>
#include <program_loader/binary_format/coff.h>

namespace program_loader {
namespace {
// On 64bit versions of Windows the smallest 32bit executable is 268 bytes
constexpr size_t kMinimumPEExecutableSize = 268;
constexpr size_t kSizeDosProgram = 56;

inline Arch TranslateCoffMachineType(CoffMachineType machine_type) {
  switch (machine_type) {
    case CoffMachineType::kIntel86:
      return Arch::kX86;
    case CoffMachineType::kAmd64:
      return Arch::kX64;
  }
  return Arch::kNone;
}

bool Is64BitArchitecture(Arch arch) {
  switch (arch) {
    case Arch::kX64:
      return true;
  }

  return false;
}

bool ClassifyPlainPE(base::Span<byte> data, FileClassificationInfo& out) {
  if (data.length() < kMinimumPEExecutableSize)
    return false;

  FileReader reader(data);
  auto* as_dos = reader.Fetch<DOSHeader>();
  if (as_dos->magic != kDosMagic)
    return false;

  // skip past dos_program
  reader.AdvanceFromStart(as_dos->farnew);
  auto* nt_header = reader.Fetch<NtHeaders>();
  if (nt_header->signature != kNtSignature)
    return false;

  out.format_type = Format::kPE;
  out.offset = as_dos->farnew;
  out.architecture = TranslateCoffMachineType(nt_header->coff.machine);

  return out.architecture != Arch::kNone;
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

bool LoaderPE::Accept(const base::Span<byte>, const FileClassificationInfo& info) {
  return info.architecture == kDescriptor.arch;
}

bool LoaderPE::Parse(const base::Span<byte> data,
                     const FileClassificationInfo& intel,
                     ProgramData& out) {
  FileReader reader(data);
  reader.Advance(intel.offset);

  // this should be valid at all times, since the classifier already validated this
  NtHeaders* nt_header;
  BUGCHECK(nt_header = reader.Fetch<NtHeaders>());

  bool is64Bit = Is64BitArchitecture(intel.architecture);
  if (is64Bit) {
    auto* pe_plus = reader.FetchNoAdvance<PE32PlusHeader>();

    out.info.base_addr = pe_plus->image_base;
    out.info.entry_ea = pe_plus->entry_address;
  } else {
    auto* pe_32 = reader.FetchNoAdvance<PE32Header>();

    out.info.base_addr = pe_32->image_base;
    out.info.entry_ea = pe_32->entry_address;
  }

  // due to alignment we cannot advance the cursor normally
  reader.Advance(nt_header->coff.size_optional_header);
  auto& segs = out.segments;

  ProgramData::Segment seg{};
  for (u16 i = 0; i < nt_header->coff.section_count; i++) {
    NtSectionHeader* section = reader.Fetch<NtSectionHeader>();
    seg.name = section->name;
    seg.size = section->raw_data_size;
    seg.start_va = section->virtual_address;
    segs.push_back(seg);
  }

  return true;
}

}  // namespace program_loader
