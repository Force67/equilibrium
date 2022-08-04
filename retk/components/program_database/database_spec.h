// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/endianess.h>
#include <base/version.h>
#include <base/math/math_helpers.h>

// The database format is the following (in order):
// - Header
// - SegmentHeader -
// --------------
// - (First segment)
// - <SourceProgramheader>
// - n other segments containing records such as 'Functions'

// database_disk_format
namespace program_database::v1 {
static constexpr u32 kMainHeaderMagic{base::ByteSwapToLittleEndian<u32>('VMH\n')};
static constexpr u32 kProgramHeaderMagic{base::ByteSwapToLittleEndian<u32>('PROG')};
static constexpr u32 kExternalFileHeaderMagic{
    base::ByteSwapToLittleEndian<u32>('EXTF')};

struct Header {
  u32 magic;                //< unique identifier
  u16 current_version_key;  // < current db, determines if the current tk version is
                            // able to load this
  u16 create_version_key;   // < version that the db was originally written as, here
                            // for future migration strategies
  u32 retk_version;         //< version of the program that created the db
  u32 headers_size;         //< unused field
  u32 section_header_offset;  //< points to the SegmentHeader, due to possible
                              // alignment padding limitations the segmentheader
                              // could not immedeatly follow the header.
  u32 seg_0_offset;  //< offset to the first segment (where the BTree begins), this
                     // exists cause due to alignment of the segment header itself.
  i64 create_date_time_stamp;    //< when the tkb was created
  i64 last_modified_time_stamp;  //< when the tkb was last modified
};
static_assert(sizeof(Header) == 40, "Header misaligned");

// these are database sections, not program sections
struct SegmentHeader {
  u16 num_sections;
  u16 section_alignment;
  u32 page_size;
};
static_assert(sizeof(SegmentHeader) == 8, "SegmentHeader misaligned");

// kSizeOfHeaders points directly into the first segment
constexpr u32 kPastHeaders = sizeof(v1::Header) + sizeof(v1::SegmentHeader);
constexpr u32 kSizeOfHeaders =
    kPastHeaders + base::NextPowerOf2_Compile(kPastHeaders) - kPastHeaders;

// names longer than 8 characters are pointing into the string table
struct NameRecord {
  union {
    char name[sizeof(pointer_size)];
    pointer_size whole;
  };
  u32 offset;
  u32 length;
};
static_assert(sizeof(NameRecord) == 16, "NameRecord misaligned.");

enum class CompressionType : u32 { kNone, kLZ4, kZip };

// the program header always exists at the start of the first segment.
struct SourceProgramHeader {
  u32 magic;
  CompressionType compression;
  NameRecord name;
  mem_size program_size;
};
static_assert(sizeof(SourceProgramHeader) == 32, "SourceProgramHeader misaligned");

// a file that is packed within one of our segments and has to be extracted and
// mounted to be used externally. this happens upon load of the database.
struct ExternalFile {
  u32 magic;
  u32 offset;
  u32 size;
  CompressionType compression;
  NameRecord name;
};
static_assert(sizeof(ExternalFile) == 32, "ExternalFile Record misaligned");

struct Functions {
  u32 count;
};

struct Names {
  u32 count;
};

struct Exports {};

struct Imports {};

// TODO(Vince): migration folder
}  // namespace program_database::v1