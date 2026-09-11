// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/external/icu/icu_utf.h>
#include <base/strings/string_ref.h>

// Terminology:
// A code point is the atomic unit of text, a number given meaning by Unicode.
// A code unit is one unit of storage for an encoded code point: 8 bits in
// UTF-8, 16 bits in UTF-16. A code point may need multiple code units, e.g.
// the snowman glyph (☃) is 1 code point, 3 UTF-8 units, 1 UTF-16 unit.

namespace base {
class UTF8CodePointIterator {
 public:
  // 4 bytes: the largest UTF-8 code point needs 4 bytes.
  using character_type = base_icu::UChar32;

  // Requires |str| to live as long as the UTF8CharIterator does.
  explicit inline UTF8CodePointIterator(base::StringRefU8 str) : str_(str) {
    CBU8_NEXT(str_.data(), next_pos_, str_.length(), char_);
  }

  UTF8CodePointIterator(const UTF8CodePointIterator&) = delete;
  UTF8CodePointIterator& operator=(const UTF8CodePointIterator&) = delete;
  ~UTF8CodePointIterator() = default;

  xsize array_pos() const { return array_pos_; }
  xsize char_pos() const { return char_pos_; }

  character_type current_character() const { return char_; }

  bool end() const { return array_pos_ == str_.length(); }
  operator bool() const noexcept { return !end(); }

  // bool operator++() { return Advance(); }

  // advance to the next actual character.
  bool Advance() noexcept {
    if (array_pos_ >= str_.length())
      return false;

    array_pos_ = next_pos_;
    char_pos_++;
    if (next_pos_ < str_.length())
      CBU8_NEXT(str_.data(), next_pos_, str_.length(), char_);

    return true;
  }

  // peek at current location + n characters (in code points).
  // e.g Hi 世界 -> Peek at pos 1 with 3 yields 界
  character_type PeekCharacter(const xsize number_of_code_points) {
    character_type the_char = 0;
    xsize the_pos = array_pos_;
    if (the_pos >= str_.length())
      return 0;
    // skip past this character.
    CBU8_NEXT(str_.data(), the_pos, str_.length(), the_char);
    for (xsize i = 0; i < number_of_code_points; i++) {
      if (the_pos >= str_.length())
        return 0;
      CBU8_NEXT(str_.data(), the_pos, str_.length(), the_char);
    }
    return the_char;
  }

 private:
  base::StringRefU8 str_;
  xsize array_pos_{0};
  xsize next_pos_{0};
  xsize char_pos_{0};
  character_type char_{0};
};
}  // namespace base