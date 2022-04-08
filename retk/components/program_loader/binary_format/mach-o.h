// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <memory>
#include <base/arch.h>
#include <program_loader/file_classifier.h>

namespace program_loader {

enum class MachCPUArch : u32 {
  // Capability bits used in the definition of cpu_type.
  kMASK = 0xff000000,      // Mask for architecture bits
  kABI64 = 0x01000000,     // 64 bit ABI
  kABI64_32 = 0x02000000,  // ILP32 ABI on 64-bit hardware
};
enum class MachCPUType : i32;

inline consteval i32 operator|(i32 lhs, MachCPUArch rhs) {
  return static_cast<i32>(static_cast<arch_types::i32>(lhs) |
                          static_cast<arch_types::u32>(rhs));
}

enum class MachCPUType : i32 {
  kANY = -1,
  kX86 = 7,
  kI386 = kX86,
  kX86_64 = kX86 | MachCPUArch::kABI64,
  /* kMIPS      = 8, */
  kMC98000 = 10,  // Old Motorola PowerPC
  kARM = 12,
  kARM64 = kARM | MachCPUArch::kABI64,
  kARM64_32 = kARM | MachCPUArch::kABI64_32,
  kSPARC = 14,
  kPOWERPC = 18,
  kPOWERPC64 = kPOWERPC | MachCPUArch::kABI64
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

}  // namespace program_loader