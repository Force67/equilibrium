// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/arch.h>
#include <base/check.h>

namespace base {
// a special size value, for indicating when we failed to find something
static constexpr mem_size kStringNotFoundPos = mem_size(-1);

// keep in mind that these free functions do not check for the validity of the
// in_buffer
template <typename TChar>
mem_size StringSearch(const TChar* haystack,
                      const mem_size size,
                      const TChar* needle,
                      const mem_size start_pos,
                      const mem_size needle_size) {
  DCHECK(haystack, "haystack is null");
  DCHECK(needle, "needle is null");
  DCHECK(size, "size is null");
  DCHECK(needle_size, "needle_size is null");

  // nsize + pos can overflow (eg pos == npos), guard against that by checking
  // that nsize + pos does not wrap around.
  if (needle_size + start_pos > size || needle_size + start_pos < start_pos)
    return kStringNotFoundPos;

  if (needle_size == 0)
    return start_pos;

  // compare the last characters first
  const auto needle_size_len = needle_size - 1;
  const auto last_needle = needle[needle_size_len];

  // Boyer-Moore skip value for the last char in the needle. Zero is
  // not a valid value; skip will be computed the first time it's
  // needed.
  mem_size skip = 0;

  const TChar* i = haystack + start_pos;
  auto iEnd = haystack + size - needle_size_len;

  while (i < iEnd) {
    // Boyer-Moore: match the last element in the needle
    while (i[needle_size_len] != last_needle) {
      if (++i == iEnd) {
        // not found
        return kStringNotFoundPos;
      }
    }
    // Here we know that the last char matches
    // Continue in pedestrian mode
    for (mem_size j = 0;;) {
      DCHECK(j < needle_size);
      if (i[j] != needle[j]) {
        // Not found, we can skip
        // Compute the skip value lazily
        if (skip == 0) {
          skip = 1;
          while (skip <= needle_size_len &&
                 needle[needle_size_len - skip] != last_needle) {
            ++skip;
          }
        }
        i += skip;
        break;
      }
      // Check if done searching
      if (++j == needle_size) {
        // Yay
        return i - haystack;
      }
    }
  }
  return kStringNotFoundPos;
}

template <typename TChar>
mem_size StringSearchNotOf(const TChar* haystack,
                           const mem_size size,
                           const TChar* needle,
                           const mem_size start_pos,
                           const mem_size needle_size) {
  DCHECK(haystack, "haystack is null");
  DCHECK(needle, "needle is null");
  DCHECK(size, "size is null");
  DCHECK(needle_size, "needle_size is null");

  const TChar* haystackEnd = haystack + size;
  for (const TChar* curPos = haystack + start_pos; curPos < haystackEnd; ++curPos) {
    bool found = true;
    for (mem_size i = 0; i < needle_size; ++i) {
      if (*curPos == needle[i]) {
        found = false;
        break;
      }
    }
    if (found) {
      return curPos - haystack;
    }
  }
  return kStringNotFoundPos;
}

template <typename TChar>
size_t StringSearchLastNotOf(const TChar* haystack,
                             size_t size,
                             const TChar* needle,
                             size_t needle_size) {
  const TChar* curPos = haystack + size - 1;  // Start at the end of the string
  for (; curPos >= haystack; --curPos) {
    bool found = true;
    for (size_t i = 0; i < needle_size; ++i) {
      if (*curPos == needle[i]) {
        found = false;
        break;
      }
    }
    if (found) {
      return curPos - haystack;
    }
  }
  return kStringNotFoundPos;
}

auto Min(auto a, auto b) {
  return (b < a) ? b : a;
}

#if 0
// keep in mind - we dont validate if the haystack is valid!
template <typename TChar>
mem_size FindLastOf(const TChar* haystack,
                    const mem_size haystack_length,
                    const TChar* needle,
                    mem_size pos,
                    mem_size needle_length) {
  if (!needle || needle_length <= 0) {
    return kStringNotFoundPos;
  }

  pos = base::Min(pos, haystack_length - 1);
  const TChar* i = haystack + pos;

  for (;; --i) {
    const mem_size offset = i - haystack;

    if (base::StringSearch(haystack, haystack_length, needle, offset,
                           needle_length) != base::kStringNotFoundPos) {
      return offset;
    }

    if (i == haystack)
      break;
  }

  return kStringNotFoundPos;
}
#endif

}  // namespace base