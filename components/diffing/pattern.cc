// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <fmt/format.h>
#include "diffing/pattern.h"

namespace diffing {
base::String PrettyPrintPattern(const Pattern& p, const u64 target_address) {
  const auto type = TypeToString(p);
  const auto ref = ReferenceTypeToString(p.type);

  if (p.lv0_offset != 0 || p.lv1_offset != 0) {
    return fmt::format("{} {}: rip({} {:+}) {:+} = {:x}", type, ref,
                       p.bytes, p.lv1_offset, p.lv0_offset, target_address);
  }

  return fmt::format("{} {}: {} = {:x}", type, ref, p.bytes,
                     target_address);
}

base::String ExportCSVLine(const Pattern& pat, const u64 source_address) {
  const char* bytes = pat.bytes.empty() ? "<unknown>" : pat.bytes.c_str();

  return fmt::format("0x{:x},{},{},{:+},{:+}", source_address, bytes,
                     EncodeReferenceType(pat.type), pat.lv1_offset,
                     pat.lv0_offset);
}

bool ValidateWildcardCount(const base::String& pattern, bool with_spacing) {
  return true;

  size_t count = 0;
  size_t i = with_spacing ? 3 : 1;
  size_t offset = with_spacing ? 2 : 1;

  for (; i < pattern.length(); i++) {
    // count patterns in a row
    if (pattern[i] == '?' && pattern[i - offset] == '?') {
      count++;

      // We only allow 4 patterns in a row e.g
      // E9 ? ? ? ?
      if (count > 3)
        return false;
    }
    if (pattern[i] != '?' && pattern[i - offset] == '?') {
      count = 0;
    }
  }

  return true;
}

const char* const ReferenceTypeToString(Pattern::ReferenceType rt) {
  switch (rt) {
    case Pattern::ReferenceType::kDirect:
      return "direct";
    case Pattern::ReferenceType::kRef3:
      return "ref3";
    default:
      IMPOSSIBLE;
      return "<unknown>";
  }
}

char const EncodeReferenceType(Pattern::ReferenceType rt) {
  switch (rt) {
    case Pattern::ReferenceType::kDirect:
      return 'd';
    case Pattern::ReferenceType::kRef3:
      return '3';
    default:
      IMPOSSIBLE;
      return '-';
  }
}

Pattern::ReferenceType DecodeReferenceType(char enc) {
  switch (enc) {
    case 'd':
      return Pattern::ReferenceType::kDirect;
    case '3':
      return Pattern::ReferenceType::kRef3;
    default:
      BUGCHECK(false);
      return Pattern::ReferenceType::kDirect;
  }
}

const char* const TypeToString(const Pattern& pattern) {
  return pattern.is_data ? "data" : "code";
}
}  // namespace diffing