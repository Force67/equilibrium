// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/endianess.h>
#include <program_loader/file_classifier.h>

namespace program_loader {
// On 64bit versions of Windows the smallest 32bit executable is 268 bytes
constexpr size_t kMinimumPEExecutableSize = 268;

constexpr i16 kDosMagic{base::ByteSwapToLittleEndian<i16>('MZ')};

struct DOSHeader {
  i16 magic;
  u16 used_bytes_last_page;
  u16 file_size_pages;
  u16 numer_of_relocations;
  u16 header_size_paragraphs;
  u16 minimum_extra_paragraphs;
  u16 maximum_extra_paragraphs;
  u16 initial_rela_ss;
  u16 initial_sp;
  u16 checksum;
  u16 initial_ip;
  u16 initial_relative_cs;
  u16 addres_of_relocations;
  u16 overlay_number;
  u16 reserved[4];
  u16 oem_id;
  u16 oem_info;
  u16 reserved2[10];
  u32 farnew;
};

enum class CoffMachineType : u16 { kIntel86 = 0x14c, kAmd64 = 0x8664 };

struct CoffFileHeader {
  CoffMachineType machine;
  u16 section_count;
  u32 time_date_stamp;
  u32 pointer_to_symtab;
  u32 number_of_symbols;
  u16 size_optional_header;
  u16 characteristics;

  bool IsImportLibrary() const { return section_count == 0xffff; }
};

// 17744
constexpr u32 kNtSignature = 17744;

struct NtHeaders {
  u32 signature;
  CoffFileHeader coff;
};

struct PE32Header {
  u16 magic;
  u8 major_linker_version;
  u8 minor_linker_version;
  u32 size_code;
  u32 size_itialized_data;
  u32 size_unitialized_data;
  u32 entry_address;
  u32 base_of_code;
  u32 base_of_data;
  u32 image_base;
  u32 section_alignment;
  u32 file_alignment;
  u16 major_os_version;
  u16 minor_os_version;
  u16 major_image_version;
  u16 minor_image_version;
  u16 major_subsystem_version;
  u16 minor_subsystem_version;
  u32 win32_version;
  u32 image_size;
  u32 header_size;
  u32 check_sum;
  u16 subsystem;
  u16 dll_characteristics;
  u32 size_stack_reserve;
  u32 size_stack_commit;
  u32 size_heareserve;
  u32 size_heacommit;
  u32 loader_flags;
  u32 number_rva_and_sizes;
};

/// The 64-bit PE header that follows the COFF header.
struct PE32PlusHeader {
  u16 magic;
  u8 major_linker_version;
  u8 minor_linker_version;
  u32 size_code;
  u32 size_initialized_data;
  u32 size_unitialized_data;
  u32 entry_address;
  u32 base_of_code;
  u64 image_base;
  u32 section_alignment;
  u32 file_alignment;
  u16 major_os_version;
  u16 minor_os_version;
  u16 major_image_version;
  u16 minor_image_version;
  u16 major_subsystem_version;
  u16 minor_subsystem_version;
  u32 win32_version;
  u32 image_size;
  u32 header_size;
  u32 check_sum;
  u16 subsytem;
  u16 dll_characteristics;
  u64 size_stack_reserve;
  u64 size_stack_commit;
  u64 size_heareserve;
  u64 size_heacommit;
  u32 loader_flags;
  u32 number_rva_and_sizes;
};

enum SectionCharacteristics : u32 {
  kInvalid = 0xffffffff,
  kTYPE_NOLOAD = 0x00000002,
  kTYPE_NO_PAD = 0x00000008,
  kCNT_CODE = 0x00000020,
  kCNT_INITIALIZED_DATA = 0x00000040,
  kCNT_UNINITIALIZED_DATA = 0x00000080,
  kLNK_OTHER = 0x00000100,
  kLNK_INFO = 0x00000200,
  kLNK_REMOVE = 0x00000800,
  kLNK_COMDAT = 0x00001000,
  kGPREL = 0x00008000,
  kMEM_PURGEABLE = 0x00020000,
  kMEM_16BIT = 0x00020000,
  kMEM_LOCKED = 0x00040000,
  kMEM_PRELOAD = 0x00080000,
  kALIGN_1BYTES = 0x00100000,
  kALIGN_2BYTES = 0x00200000,
  kALIGN_4BYTES = 0x00300000,
  kALIGN_8BYTES = 0x00400000,
  kALIGN_16BYTES = 0x00500000,
  kALIGN_32BYTES = 0x00600000,
  kALIGN_64BYTES = 0x00700000,
  kALIGN_128BYTES = 0x00800000,
  kALIGN_256BYTES = 0x00900000,
  kALIGN_512BYTES = 0x00A00000,
  kALIGN_1024BYTES = 0x00B00000,
  kALIGN_2048BYTES = 0x00C00000,
  kALIGN_4096BYTES = 0x00D00000,
  kALIGN_8192BYTES = 0x00E00000,
  kALIGN_MASK = 0x00F00000,
  kLNK_NRELOC_OVFL = 0x01000000,
  kMEM_DISCARDABLE = 0x02000000,
  kMEM_NOT_CACHED = 0x04000000,
  kMEM_NOT_PAGED = 0x08000000,
  kMEM_SHARED = 0x10000000,
  kMEM_EXECUTE = 0x20000000,
  kMEM_READ = 0x40000000,
  kMEM_WRITE = 0x80000000
};

struct NtSectionHeader {
  char name[8];
  u32 virtual_size;
  u32 virtual_address;
  u32 raw_data_size;
  u32 raw_data_pointer;
  u32 relocations_pointer;
  u32 line_number_pointer;
  u16 relocation_count;
  u16 line_count;
  SectionCharacteristics characteristics;
};
}  // namespace program_loader