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

ProgramData::Segment::Flags TranslateProgramFlags(u32 elf_flags) {
  using SF = ProgramData::Segment::Flags;
  SF new_flags{SF::kNone};
  if (elf_flags & PF_X)
    new_flags |= SF::kX;
  if (elf_flags & PF_W)
    new_flags |= SF::kW;
  if (elf_flags & PF_R)
    new_flags |= SF::kR;
  return new_flags;
}

// TODO: architecture specific switches for ARM & MIPS
static base::String MakeProgramName(ElfProgramType pt) {
  switch (pt) {
    case ElfProgramType::kLOAD:
      return "LOAD";
    case ElfProgramType::kDYNAMIC:
      return "DYNAMIC";
    case ElfProgramType::kINTERP:
      return "INTERP";
    case ElfProgramType::kNOTE:
      return "NOTE";
    case ElfProgramType::kSHLIB:
      return "SHLIB";
    case ElfProgramType::kPHDR:
      return "Program Header Table";
    case ElfProgramType::kTLS:
      return "TLS";
    case ElfProgramType::kLOOS:
      return "LOOS";
    case ElfProgramType::kHIOS:
      return "HIOS";
    case ElfProgramType::kLOPROC:
      return "LOPROC";
    case ElfProgramType::kHIPROC:
      return "HIPROC";
    case ElfProgramType::kGNU_EH_FRAME:
    case ElfProgramType::kSUNW_UNWIND:
      return "eh_frame";
    case ElfProgramType::kGNU_STACK:
      return "stack";
    case ElfProgramType::kGNU_RELRO:
      return "relocations";
    case ElfProgramType::kGNU_PROPERTY:
      return "property notes";
    case ElfProgramType::kOPENBSD_RANDOMIZE:
      return "OpenBSD random data";
    case ElfProgramType::kOPENBSD_WXNEEDED:
      return "OpenBSD wxviolate";
    case ElfProgramType::kOPENBSD_BOOTDATA:
      return "OpenBSD bootdata";
    case ElfProgramType::kNULL:
    default:
      break;
  }
  return "<unknown>";
}

bool ClassifyPlainELF(base::Span<byte> data, FileClassificationInfo& out) {
  FileReader reader(data);

  // architecture independant verification
  auto* ident = reader.Fetch<byte[16]>();
  if (memcmp(ident, kElfMagic, 4) != 0)
    return false;

  out.format_type = Format::kELF;
  out.numeric = *reader.Fetch<u16>();
  out.architecture = TranslateElfMachineType(*reader.Fetch<ElfMachineType>());

  return out.architecture != Arch::kNone;
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
  // our loader only supports executables and dynamic libraries
  // users are free to register a custom loader for the other formats
  auto elf_type = static_cast<ElfType>(info.numeric);
  return elf_type == ElfType::kEXEC || elf_type == ElfType::kDYN;
}

template <typename THeader, typename TProgramheader>
static void ParseElf(FileReader& reader, ProgramData& out) {
  auto* header = reader.Fetch<THeader>();
  out.info.entry_ea = header->entry_address;

  reader.AdvanceFromStart(header->program_header_offset);
  auto* segments = reader.Fetch<TProgramheader>();

  u64 base = -1;
  ProgramData::Segment seg{};
  for (u16 i = 0; i < header->program_header_count; i++) {
    const auto* p = &segments[i];
    seg.name = MakeProgramName(p->type);
    seg.disk_size = p->disk_size;
    seg.mem_size = p->memory_size;
    seg.start_va = p->virtual_address;
    seg.flags = TranslateProgramFlags(p->flags);

    // https://stackoverflow.com/questions/18296276/base-address-of-elf
    // The first (lowest) LOAD segment's virtual address is the default load base
    // of the file.
    if (p->type == ElfProgramType::kLOAD && !base) {
      base = seg.start_va;
    }

    out.segments.push_back(seg);
  }

  out.info.base_addr = base;
}

bool LoaderELF::Parse(const base::Span<byte> data,
                      const FileClassificationInfo& intel,
                      ProgramData& out) {
  FileReader reader(data);
  bool is_64 = Is64BitArchitecture(intel.architecture);

  if (is_64)
    ParseElf<Elf64Header, Elf64ProgramHeader>(reader, out);
  else
    ParseElf<Elf32Header, Elf32ProgramHeader>(reader, out);

  return true;
}

}  // namespace program_loader
