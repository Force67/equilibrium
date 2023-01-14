// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/external/icu/icu_utf.h>
#include <base/strings/string_ref.h>

namespace base {
class UTF8CodePointIterator {
 public:
  // note that this is 4 bytes since the biggest utf8 char can be 4 bytes
  using character_type = base_icu::UChar32;

  // Requires |str| to live as long as the UTF8CharIterator does.
  explicit inline UTF8CodePointIterator(base::StringRefU8 str) : str_(str) {
    CBU8_NEXT(str_.data(), next_pos_, str_.length(), char_);
  }

  UTF8CodePointIterator(const UTF8CodePointIterator&) = delete;
  UTF8CodePointIterator& operator=(const UTF8CodePointIterator&) = delete;
  ~UTF8CodePointIterator() = default;

  size array_pos() const { return array_pos_; }
  size char_pos() const { return char_pos_; }

  const character_type current_character() const { return char_; }

  bool end() const { return array_pos_ == str_.length(); }
  operator bool() const noexcept { return !end(); }

  //bool operator++() { return Advance(); }

  // Advance to the next actual character.  Returns false if we're at the
  // end of the string.
  bool Advance() {
    if (array_pos_ >= str_.length())
      return false;

    array_pos_ = next_pos_;
    char_pos_++;
    if (next_pos_ < str_.length())
      CBU8_NEXT(str_.data(), next_pos_, str_.length(), char_);

    return true;
  }

 private:
  base::StringRefU8 str_;
  size array_pos_{0};
  size next_pos_{0};
  size char_pos_{0};
  i32 char_{0};
};
}  // namespace base