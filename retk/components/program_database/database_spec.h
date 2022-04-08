// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>

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
  u32 tree_offset;               //< where the binary tree begins
};

enum class CompressionType : u32 { kNone, kLZ4, kZip };

static constexpr u32 kProgramHeaderMagic = 'PROG';

struct SourceProgramHeader {
  u32 magic;
  CompressionType compression_type;
  u64 program_size;
};

// TODO(Vince): migration folder
}  // namespace program_database::v1