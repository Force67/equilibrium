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
  kMAGIC = 0xFEEDFACEu,
  kCIGAM = 0xCEFAEDFEu,
  kMAGIC_64 = 0xFEEDFACFu,
  kCIGAM_64 = 0xCFFAEDFEu,
  FAT_MAGIC = 0xCAFEBABEu,
  FAT_CIGAM = 0xBEBAFECAu,
  FAT_MAGIC_64 = 0xCAFEBABFu,
  FAT_CIGAM_64 = 0xBFBAFECAu
};

enum class MachFileType : u32 {
  // Constants for the "filetype" field in llvm::MachO::mach_header and
  // llvm::MachO::mach_header_64
  kOBJECT = 0x1u,
  kEXECUTE = 0x2u,
  kFVMLIB = 0x3u,
  kCORE = 0x4u,
  kPRELOAD = 0x5u,
  kDYLIB = 0x6u,
  kDYLINKER = 0x7u,
  kBUNDLE = 0x8u,
  kDYLIB_STUB = 0x9u,
  kDSYM = 0xAu,
  kKEXT_BUNDLE = 0xBu
};

struct MachHeader32 {
  MachMagic magic;
  u32 cputype;
  u32 cpusubtype;
  MachFileType file_type;
  u32 load_command_count;
  u32 sizeofcmds;
  u32 flags;
};
static_assert(sizeof(MachHeader32) == 28, "MachHeader32 alignment wrong");

struct MachHeader64 {
  u32 magic;
  u32 cputype;
  u32 cpusubtype;
  MachFileType file_type;
  u32 load_command_count;
  u32 sizeofcmds;
  u32 flags;
  u32 reserved;
};
static_assert(sizeof(MachHeader64) == 32, "MachHeader64 alignment wrong");

enum class MachLoadCommandType : u32 {
  kSEGMENT = 1,
  kSYMTAB = 2,
  kSYMSEG = 3,
  kTHREAD = 4,
  kUNIXTHREAD = 5,
  kLOADFVMLIB = 6,
  kIDFVMLIB = 7,
  kIDENT = 8,
  kFVMFILE = 9,
  kPREPAGE = 10,
  kDYSYMTAB = 11,
  kLOAD_DYLIB = 12,
  kID_DYLIB = 13,
  kLOAD_DYLINKER = 14,
  kID_DYLINKER = 15,
  kPREBOUND_DYLIB = 16,
  kROUTINES = 17,
  kSUB_FRAMEWORK = 18,
  kSUB_UMBRELLA = 19,
  kSUB_CLIENT = 20,
  kSUB_LIBRARY = 21,
  kTWOLEVEL_HINTS = 22,
  kPREBIND_CKSUM = 23,
  kLOAD_WEAK_DYLIB = 2147483672,
  kSEGMENT_64 = 25,
  kROUTINES_64 = 26,
  kUUID = 27,
  kRPATH = 2147483676,
  kCODE_SIGNATURE = 29,
  kSEGMENT_SPLIT_INFO = 30,
  kREEXPORT_DYLIB = 2147483679,
  kLAZY_LOAD_DYLIB = 32,
  kENCRYPTION_INFO = 33,
  kDYLD_INFO = 34,
  kDYLD_INFO_ONLY = 2147483682,
  kLOAD_UPWARD_DYLIB = 2147483683,
  kVERSION_MIN_MACOSX = 36,
  kVERSION_MIN_IPHONEOS = 37,
  kFUNCTION_STARTS = 38,
  kDYLD_ENVIRONMENT = 39,
  kMAIN = 2147483688,
  kDATA_IN_CODE = 41,
  kSOURCE_VERSION = 42,
  kDYLIB_CODE_SIGN_DRS = 43,
  kENCRYPTION_INFO_64 = 44,
  kLINKER_OPTION = 45,
  kLINKER_OPTIMIZATION_HINT = 46,
  kVERSION_MIN_TVOS = 47,
  kVERSION_MIN_WATCHOS = 48,
  kNOTE = 49,
  kBUILD_VERSION = 50
};

struct MachLoadCommand {
  MachLoadCommandType type;
  u32 size;
};

struct EntryPointCommand {
  u32 cmd;
  u32 cmdsize;
  u64 entry_offset;
  u64 stacksize;
};

// Values for segment_command.initprot.
// From <mach/vm_prot.h>
enum { VM_PROT_READ = 0x1, VM_PROT_WRITE = 0x2, VM_PROT_EXECUTE = 0x4 };

struct SegmentCommand {
  u32 cmd;
  u32 cmdsize;
  char segname[16];
  u32 vmaddr;
  u32 vmsize;
  u32 fileoff;
  u32 filesize;
  u32 maxprot;
  u32 initial_prot;
  u32 nsects;
  u32 flags;
};

struct SegmentCommand64 {
  u32 cmd;
  u32 cmdsize;
  char segname[16];
  u64 vmaddr;
  u64 vmsize;
  u64 fileoff;
  u64 filesize;
  u32 maxprot;
  u32 initial_prot;
  u32 nsects;
  u32 flags;
};

struct section {
  char sectname[16];
  char segname[16];
  u32 addr;
  u32 size;
  u32 offset;
  u32 align;
  u32 reloff;
  u32 nreloc;
  u32 flags;
  u32 reserved1;
  u32 reserved2;
};

struct section_64 {
  char sectname[16];
  char segname[16];
  u64 addr;
  u64 size;
  u32 offset;
  u32 align;
  u32 reloff;
  u32 nreloc;
  u32 flags;
  u32 reserved1;
  u32 reserved2;
  u32 reserved3;
};
}  // namespace program_loader