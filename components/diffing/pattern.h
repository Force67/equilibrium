// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/check.h>
#include <base/strings/xstring.h>

namespace diffing {

using namespace arch_types;

// represents a signature for matching function
// or data
struct Pattern {
  enum class ReferenceType : u8 { kDirect, kRef3 };

  base::String bytes;  //< Bytes in text form
  i8 lv1_offset;   //< Offset within the reference towards the functor
  i8 lv0_offset;   //< Offset within the function towards the location
  bool is_data;         //< Does it point to data or code
  ReferenceType type;   //< Is it a direct reference, or do we point to a
                        //  CALL/LEA/MOV instruction
};

// Fails if it finds too many wildcards in a row.
bool ValidateWildcardCount(const base::String& pattern, bool with_spacing);

// format the pattern in a human readable way
base::String PrettyPrintPattern(const Pattern&, const u64 target_address);

// format the pattern for CSV
base::String ExportCSVLine(const Pattern&, const u64 source_address);

// making this a struct member method is kinda ugly.
const char* const ReferenceTypeToString(Pattern::ReferenceType);

// encode/decode data for CSV format
char const EncodeReferenceType(Pattern::ReferenceType);
Pattern::ReferenceType DecodeReferenceType(char enc);

// Mainly a method for future proofing.
const char* const TypeToString(const Pattern&);
}  // namespace diffing