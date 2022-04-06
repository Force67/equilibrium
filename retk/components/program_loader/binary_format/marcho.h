// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <memory>
#include <base/arch.h>
#include <program_loader/file_classifier.h>

namespace program_loader {

enum class MachCPUType : u32 {
  CPU_TYPE_ANY = -1,
  CPU_TYPE_X86 = 7,
  CPU_TYPE_I386 = CPU_TYPE_X86,
  CPU_TYPE_X86_64 = CPU_TYPE_X86 | CPU_ARCH_ABI64,
  /* CPU_TYPE_MIPS      = 8, */
  CPU_TYPE_MC98000 = 10,  // Old Motorola PowerPC
  CPU_TYPE_ARM = 12,
  CPU_TYPE_ARM64 = CPU_TYPE_ARM | CPU_ARCH_ABI64,
  CPU_TYPE_ARM64_32 = CPU_TYPE_ARM | CPU_ARCH_ABI64_32,
  CPU_TYPE_SPARC = 14,
  CPU_TYPE_POWERPC = 18,
  CPU_TYPE_POWERPC64 = CPU_TYPE_POWERPC | CPU_ARCH_ABI64
};

enum class MachMagic : u32 {
  // Constants for the "magic" field in llvm::MachO::mach_header and
  // llvm::MachO::mach_header_64
  MH_MAGIC = 0xFEEDFACEu,
  MH_CIGAM = 0xCEFAEDFEu,
  MH_MAGIC_64 = 0xFEEDFACFu,
  MH_CIGAM_64 = 0xCFFAEDFEu,
  FAT_MAGIC = 0xCAFEBABEu,
  FAT_CIGAM = 0xBEBAFECAu,
  FAT_MAGIC_64 = 0xCAFEBABFu,
  FAT_CIGAM_64 = 0xBFBAFECAu
};

struct MachHeader {
  MachMagic magic;
  u32 cputype;
  u32 cpusubtype;
  u32 filetype;
  u32 ncmds;
  u32 sizeofcmds;
  u32 flags;

  bool CheckMagic() const {
    switch (magic) {
      case MachMagic::MH_MAGIC:
      case MachMagic::MH_CIGAM:
      case MachMagic::MH_MAGIC_64:
      case MachMagic::MH_CIGAM_64:
      case MachMagic::FAT_MAGIC:
      case MachMagic::FAT_CIGAM:
      case MachMagic::FAT_MAGIC_64:
      case MachMagic::FAT_CIGAM_64:
        return true;
      default:
        return false;
    }
  }
};

static Arch TranslateMachMachineType(MachCPUType type) {
  switch (type) {
    case MachCPUType::CPU_TYPE_X86:
      return Arch::kX86;
    case MachCPUType::CPU_TYPE_X86_64:
      return Arch::kX64;
  }
  return Arch::kNone;
}

}  // namespace program_loader