// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/arch.h>
#include <base/xstring.h>

namespace diffing {

using namespace arch_types;

// represents a signature for matching function
// or data
struct Pattern {
  enum class ReferenceType : u8 { kDirect, kRef3 };

  base::XString<char> bytes;  //< Bytes in text form
  i8 reference_offset;  //< Offset within the reference towards the functor
  i8 function_offset;   //< Offset within the function towards the location
  bool is_data;         //< Does it point to data or code
  ReferenceType type;   //< Is it a direct reference, or do we point to a
                        //  CALL/LEA/MOV instruction
};
}  // namespace diffing