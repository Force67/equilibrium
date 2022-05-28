// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/endian.h>

// database_disk_format
namespace program_database::v1 {
static constexpr u32 kMainHeaderMagic{base::BSwap<u32>('VMH\n')};
static constexpr u32 kProgramHeaderMagic{base::BSwap<u32>('PROG')};
static constexpr u32 kExternalFileHeaderMagic{base::BSwap<u32>('EXTF')};

// names longer than 8 characters are pointing into the string table
struct NameRecord {
  union {
    const char name[8];
    u32 offset;
    u32 length;
  };
};
static_assert(sizeof(NameRecord) == sizeof(char[8]));

struct Header {
  u32 magic;                     //< unique identifier
  u32 db_version;                //< database version
  u32 user_id;                   //< user id
  u32 retk_version;              //< version of the program that created the db
  u64 create_date_time_stamp;    //< when the idb was created
  u64 last_modified_time_stamp;  //< when the idb was last modified
  u32 program_seg_offset;        //< offset to the SourceProgramHeader
  u32 section_header_offset;     //< where the binary tree begins
};
static_assert(sizeof(Header) == 40, "Header misaligned");

// these are database sections, not program sections
struct SegmentHeader {
  u16 num_sections;
  u16 section_alignment;
  u32 page_size;
};
static_assert(sizeof(SegmentHeader) == 8, "SegmentHeader misaligned");

enum class CompressionType : u32 { kNone, kLZ4, kZip };

// the program header always exists at the start of the first segment.
struct SourceProgramHeader {
  u32 magic;
  CompressionType compression;
  NameRecord name;
  u64 program_size;
};
static_assert(sizeof(SourceProgramHeader) == 24, "SourceProgramHeader misaligned");

// a file that is packed within one of our segments and has to be extracted and
// mounted to be used externally. this happens upon load of the database.
struct ExternalFile {
  u32 magic;
  u32 offset;
  u32 size;
  CompressionType compression;
  NameRecord name;
};
static_assert(sizeof(ExternalFile) == 24, "ExternalFile Record misaligned");

// TODO(Vince): migration folder
}  // namespace program_database::v1