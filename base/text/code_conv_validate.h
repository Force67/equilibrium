// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <cstdint>

namespace base {

inline bool IsValidCodepoint(uint32_t code_point) {
  // Excludes code points that are not Unicode scalar values, i.e.
  // surrogate code points ([0xD800, 0xDFFF]). Additionally, excludes
  // code points larger than 0x10FFFF (the highest codepoint allowed).
  // Non-characters and unassigned code points are allowed.
  // https://unicode.org/glossary/#unicode_scalar_value
  return code_point < 0xD800u ||
         (code_point >= 0xE000u && code_point <= 0x10FFFFu);
}

// Assuming that a pointer is the size of a "machine word", then
// uintptr_t is an integer type that is also a machine word.
using MachineWord = uintptr_t;

inline bool IsMachineWordAligned(const void* pointer) {
  return !(reinterpret_cast<MachineWord>(pointer) & (sizeof(MachineWord) - 1));
}

template <class Char>
bool DoIsStringASCII(const Char* characters, size_t length) {
  // Bitmasks to detect non ASCII characters for character sizes of 8, 16 and 32
  // bits.
  constexpr MachineWord NonASCIIMasks[] = {
      0, MachineWord(0x8080808080808080ULL), MachineWord(0xFF80FF80FF80FF80ULL),
      0, MachineWord(0xFFFFFF80FFFFFF80ULL),
  };

  if (!length)
    return true;
  constexpr MachineWord non_ascii_bit_mask = NonASCIIMasks[sizeof(Char)];
  static_assert(non_ascii_bit_mask, "Error: Invalid Mask");
  MachineWord all_char_bits = 0;
  const Char* end = characters + length;

  // Prologue: align the input.
  while (!IsMachineWordAligned(characters) && characters < end)
    all_char_bits |= *characters++;
  if (all_char_bits & non_ascii_bit_mask)
    return false;

  // Compare the values of CPU word size.
  constexpr size_t chars_per_word = sizeof(MachineWord) / sizeof(Char);
  constexpr int batch_count = 16;
  while (characters <= end - batch_count * chars_per_word) {
    all_char_bits = 0;
    for (int i = 0; i < batch_count; ++i) {
      all_char_bits |= *(reinterpret_cast<const MachineWord*>(characters));
      characters += chars_per_word;
    }
    if (all_char_bits & non_ascii_bit_mask)
      return false;
  }

  // Process the remaining words.
  all_char_bits = 0;
  while (characters <= end - chars_per_word) {
    all_char_bits |= *(reinterpret_cast<const MachineWord*>(characters));
    characters += chars_per_word;
  }

  // Process the remaining bytes.
  while (characters < end)
    all_char_bits |= *characters++;

  return !(all_char_bits & non_ascii_bit_mask);
}
}  // namespace base