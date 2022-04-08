// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <program_loader/file_classifier.h>

namespace program_loader {
// On 64bit versions of Windows the smallest 32bit executable is 268 bytes
constexpr size_t kMinimumPEExecutableSize = 268;
constexpr i16 kDosMagic = 'MZ';

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
  u32 size_heap_reserve;
  u32 size_heap_commit;
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
  u64 size_heap_reserve;
  u64 size_heap_commit;
  u32 loader_flags;
  u32 number_rva_and_sizes;
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
}  // namespace program_loader