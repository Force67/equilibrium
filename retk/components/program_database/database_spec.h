// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

// database_disk_format
namespace program_database::v1 {
static constexpr u32 kMainHeaderMagic = 'VMH\n';

struct Header {
  u32 magic;                     //< unique identifier
  u32 db_version;                //< database version
  u32 user_id;                   //< user id
  u32 retk_version;              //< version of the program that created the db
  u64 create_date_time_stamp;    //< when the idb was created
  u64 last_modified_time_stamp;  //< when the idb was last modified
  u32 progarm_seg_offset;        //< offset to the SourceProgramHeader
  u32 section_header_offset;     //< where the binary tree begins
};

// these are database sections, not program sections
struct SegmentHeader {
  u16 num_sections;
  u16 section_alignment;
  u16 reserved1;
  u16 reserved2;
};

static constexpr u32 kProgramHeaderMagic = 'PROG';

enum class CompressionType : u32 { kNone, kLZ4, kZip };

// the program header exists within the source prog section.
struct SourceProgramHeader {
  u32 magic;
  CompressionType compression_type;
  u64 program_size;
};

// TODO(Vince): migration folder
}  // namespace program_database::v1