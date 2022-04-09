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

ProgramData::Segment::Flags TranslateProgramFlags(u32 prot) {
  using SF = ProgramData::Segment::Flags;
  SF new_flags{SF::kNone};
  if (prot & VM_PROT_READ)
    new_flags |= SF::kR;
  if (prot & VM_PROT_WRITE)
    new_flags |= SF::kW;
  if (prot & VM_PROT_EXECUTE)
    new_flags |= SF::kX;
  return new_flags;
}

bool CheckMagic(MachMagic magic) {
  switch (magic) {
    case MachMagic::kMAGIC:
    case MachMagic::kCIGAM:
    case MachMagic::kMAGIC_64:
    case MachMagic::kCIGAM_64:
    case MachMagic::FAT_MAGIC:
    case MachMagic::FAT_CIGAM:
    case MachMagic::FAT_MAGIC_64:
    case MachMagic::FAT_CIGAM_64:
      return true;
    default:
      return false;
  }
}

bool ClassifyPlainMach(base::Span<byte> data, FileClassificationInfo& out) {
  FileReader reader(data);
  // the only difference between _64 and _32 are the reserved 4 bytes at the end
  // so we can use this to access these specific variables on both
  auto* as_mach = reader.Fetch<MachHeader32>();
  if (CheckMagic(as_mach->magic)) {
    out.format_type = Format::kMachO;
    out.numeric = static_cast<size_t>(as_mach->file_type);
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
  // our loader only supports executables and dynamic libraries
  // users are free to register a custom loader for the other formats
  const auto file_type = static_cast<MachFileType>(info.numeric);
  return file_type == MachFileType::kDYLIB || file_type == MachFileType::kEXECUTE;
}

template <class T>
static void ParseSegmentCommand(ProgramData::Segment& seg,
                                T* segment,
                                ProgramData& out) {
  seg.name = segment->segname;
  seg.disk_size = segment->filesize;
  seg.mem_size = segment->vmsize;
  seg.start_va = segment->vmaddr;
  seg.flags = TranslateProgramFlags(segment->initial_prot);
  out.segments.push_back(seg);

  // yea, thats how they do it...
  if (seg.name == "__TEXT")
    out.info.base_addr = seg.start_va;
}

// good documentation:
// https://h3adsh0tzz.com/2020/01/macho-file-format/
bool LoaderMachO::Parse(const base::Span<byte> data,
                        const FileClassificationInfo& intel,
                        ProgramData& out) {
  FileReader reader(data);
  auto* header = reader.Fetch<MachHeader32>();
  // skip the 4 byte reserved word on 64 bit.
  if (Is64BitArchitecture(intel.architecture))
    reader.Advance(4);

  ProgramData::Segment seg{};
  for (u32 i = 0; i < header->load_command_count; i++) {
    auto* command = reader.FetchNoAdvance<MachLoadCommand>();
    switch (command->type) {
      case MachLoadCommandType::kSEGMENT: {
        auto* segment = reader.FetchNoAdvance<SegmentCommand>();
        ParseSegmentCommand(seg, segment, out);
        break;
      }
      case MachLoadCommandType::kSEGMENT_64: {
        auto* segment = reader.FetchNoAdvance<SegmentCommand64>();
        ParseSegmentCommand(seg, segment, out);
        break;
      }
      case MachLoadCommandType::kMAIN: {
        // https://github.com/lief-project/LIEF/blob/261c49ebf5993926c10f152a829f414e3dd182ce/src/MachO/Binary.cpp#L188
        auto* entry = reader.FetchNoAdvance<EntryPointCommand>();
        out.info.entry_ea = entry->entry_offset;
        break;
      }
    }
    reader.Advance(command->size);
  }

  // EntryPointCommand
  // out.info.entry_ea =

  // https://h3adsh0tzz.com/2020/01/macho-file-format/

  return false;
}

}  // namespace program_loader
