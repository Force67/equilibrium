// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <limits.h>
#include <stdint.h>
#include <type_traits>

#include "base/text/code_convert.h"
#include "base/text/code_point_validation.h"

#include "base/external/icu/icu_utf.h"
#include "build/build_config.h"

#include "base/check.h"
#include "strings/string_ref.h"

namespace base {
namespace {

constexpr base_icu::UChar32 kErrorCodePoint = 0xFFFD;

template <typename SrcChar, typename DestChar>
struct SizeCoefficient {
  // static_assert(sizeof(SrcChar) < sizeof(DestChar),
  //               "Default case: from a smaller encoding to the bigger one");

  // ASCII symbols are encoded by one codeunit in all encodings.
  static constexpr int value = 1;
};

template <>
struct SizeCoefficient<char16_t, char> {
  // One UTF-16 codeunit corresponds to at most 3 codeunits in UTF-8.
  static constexpr int value = 3;
};

template <typename SrcChar, typename DestChar>
constexpr int size_coefficient_v =
    SizeCoefficient<std::decay_t<SrcChar>, std::decay_t<DestChar>>::value;

// UnicodeAppendUnsafe --------------------------------------------------------
// Function overloads that write code_point to the output string. Output string
// has to have enough space for the codepoint.

// Convenience typedef that checks whether the passed in type is integral (i.e.
// bool, char, int or their extended versions) and is of the correct size.
template <typename Char, size_t N>
using EnableIfBitsAre = std::enable_if_t<std::is_integral<Char>::value &&
                                             CHAR_BIT * sizeof(Char) == N,
                                         bool>;

template <typename Char, EnableIfBitsAre<Char, 8> = true>
void UnicodeAppendUnsafe(Char* out, int32_t* size, uint32_t code_point) {
  CBU8_APPEND_UNSAFE(out, *size, code_point);
}

template <typename Char, EnableIfBitsAre<Char, 16> = true>
void UnicodeAppendUnsafe(Char* out, int32_t* size, uint32_t code_point) {
  CBU16_APPEND_UNSAFE(out, *size, code_point);
}

template <typename Char, EnableIfBitsAre<Char, 32> = true>
void UnicodeAppendUnsafe(Char* out, int32_t* size, uint32_t code_point) {
  out[(*size)++] = code_point;
}

template <typename Char>
bool IsStringASCII(const base::BasicStringRef<Char> str) {
  return DoIsStringASCII(str.data(), str.length());
}

template <typename DestChar>
bool DoUTFConversion(const char* src,
                     int32_t src_len,
                     DestChar* dest,
                     int32_t* dest_len) {
  bool success = true;

  for (int32_t i = 0; i < src_len;) {
    int32_t code_point;
    CBU8_NEXT(src, i, src_len, code_point);

    if (!IsValidCodepoint(code_point)) {
      success = false;
      code_point = kErrorCodePoint;
    }

    UnicodeAppendUnsafe(dest, dest_len, code_point);
  }

  return success;
}

template <typename DestChar>
bool DoUTFConversion(const char8_t* src,
                     int32_t src_len,
                     DestChar* dest,
                     int32_t* dest_len) {
  bool success = true;

  for (int32_t i = 0; i < src_len;) {
    int32_t code_point;
    CBU8_NEXT(src, i, src_len, code_point);

    if (!IsValidCodepoint(code_point)) {
      success = false;
      code_point = kErrorCodePoint;
    }

    UnicodeAppendUnsafe(dest, dest_len, code_point);
  }

  return success;
}

template <typename DestChar>
bool DoUTFConversion(const char16_t* src,
                     int32_t src_len,
                     DestChar* dest,
                     int32_t* dest_len) {
  bool success = true;

  auto ConvertSingleChar = [&success](char16_t in) -> int32_t {
    if (!CBU16_IS_SINGLE(in) || !IsValidCodepoint(in)) {
      success = false;
      return kErrorCodePoint;
    }
    return in;
  };

  int32_t i = 0;

  // Always have another symbol in order to avoid checking boundaries in the
  // middle of the surrogate pair.
  while (i < src_len - 1) {
    int32_t code_point;

    if (CBU16_IS_LEAD(src[i]) && CBU16_IS_TRAIL(src[i + 1])) {
      code_point = CBU16_GET_SUPPLEMENTARY(src[i], src[i + 1]);
      if (!IsValidCodepoint(code_point)) {
        code_point = kErrorCodePoint;
        success = false;
      }
      i += 2;
    } else {
      code_point = ConvertSingleChar(src[i]);
      ++i;
    }

    UnicodeAppendUnsafe(dest, dest_len, code_point);
  }

  if (i < src_len)
    UnicodeAppendUnsafe(dest, dest_len, ConvertSingleChar(src[i]));

  return success;
}

#if defined(WCHAR_T_IS_UTF32)

template <typename DestChar>
bool DoUTFConversion(const wchar_t* src,
                     int32_t src_len,
                     DestChar* dest,
                     int32_t* dest_len) {
  bool success = true;

  for (int32_t i = 0; i < src_len; ++i) {
    int32_t code_point = src[i];

    if (!IsValidCodepoint(code_point)) {
      success = false;
      code_point = kErrorCodePoint;
    }

    UnicodeAppendUnsafe(dest, dest_len, code_point);
  }

  return success;
}

#endif  // defined(WCHAR_T_IS_UTF32)

// UTFConversion --------------------------------------------------------------
// Function template for generating all UTF conversions.

template <typename InputString, typename DestString>
bool UTFConversion(const InputString& src_str, DestString* dest_str) {
  if (IsStringASCII(src_str)) {
    dest_str->assign(
        reinterpret_cast<const DestString::value_type*>(src_str.begin()),
        reinterpret_cast<const DestString::value_type*>(src_str.end()));
    return true;
  }

  dest_str->resize(src_str.length() *
                   size_coefficient_v<typename InputString::value_type,
                                      typename DestString::value_type>);

  // Empty string is ASCII => it OK to call operator[].
  auto* dest = &(*dest_str)[0];

  // ICU requires 32 bit numbers.
  int32_t src_len32 = static_cast<int32_t>(src_str.length());
  int32_t dest_len32 = 0;

  bool res = DoUTFConversion(src_str.data(), src_len32, dest, &dest_len32);

  dest_str->resize(dest_len32);
  dest_str->shrink_to_fit();

  return res;
}

}  // namespace

// UTF16 <-> UTF8 --------------------------------------------------------------

bool UTF8ToUTF16(const char* src, size_t src_len, base::StringU16* output) {
  return UTFConversion(base::StringRef(src, src_len), output);
}

base::StringU16 UTF8ToUTF16(base::StringRef utf8) {
  base::StringU16 ret;
  // Ignore the success flag of this call, it will do the best it can for
  // invalid input, which is what we want here.
  UTF8ToUTF16(utf8.data(), utf8.size(), &ret);
  return ret;
}

bool UTF16ToUTF8(const char16_t* src, size_t src_len, base::StringU8* output) {
  return UTFConversion(base::StringRefU16(src, src_len), output);
}

base::StringU8 UTF16ToUTF8(base::StringRefU16 utf16) {
  base::StringU8 ret;
  // Ignore the success flag of this call, it will do the best it can for
  // invalid input, which is what we want here.
  UTF16ToUTF8(utf16.data(), utf16.length(), &ret);
  return ret;
}

// UTF-16 <-> Wide -------------------------------------------------------------

#if defined(WCHAR_T_IS_UTF16)
// When wide == UTF-16 the conversions are a NOP.
// they are the same on this selected platform, so we can just copy the data.

bool WideToUTF16(const wchar_t* src, size_t src_len, base::StringU16* output) {
  output->assign(reinterpret_cast<const char16_t*>(src),
                 reinterpret_cast<const char16_t*>(src + src_len));
  return true;
}

base::StringU16 WideToUTF16(base::StringRefW wide) {
  return base::StringU16(reinterpret_cast<const char16_t*>(wide.begin()),
                         reinterpret_cast<const char16_t*>(wide.end()));
}

bool UTF16ToWide(const char16_t* src, size_t src_len, base::StringW* output) {
  output->assign(reinterpret_cast<const wchar_t*>(src),
                 reinterpret_cast<const wchar_t*>(src + src_len));
  return true;
}

#elif defined(WCHAR_T_IS_UTF32)

bool WideToUTF16(const wchar_t* src, size_t src_len, base::StringU16* output) {
  // return UTFConversion(base::StringRefW(src, src_len), output);
  return false;
}

base::StringU16 WideToUTF16(base::StringRefW wide) {
  base::StringU16 ret;
  DEBUG_TRAP;
  // Ignore the success flag of this call, it will do the best it can for
  // invalid input, which is what we want here.
  // WideToUTF16(wide.data(), wide.length(), &ret);
  return ret;
}

bool UTF16ToWide(const char16_t* src, size_t src_len, base::StringW* output) {
  return UTFConversion(base::StringRefU16(src, src_len), output);
}

base::StringW UTF16ToWide(base::StringRefU16 utf16) {
  base::StringW ret;
  // Ignore the success flag of this call, it will do the best it can for
  // invalid input, which is what we want here.
  UTF16ToWide(utf16.data(), utf16.length(), &ret);
  return ret;
}

#endif  // defined(WCHAR_T_IS_UTF32)

// UTF-8 <-> Wide --------------------------------------------------------------

// UTF8ToWide is the same code, regardless of whether wide is 16 or 32 bits

bool UTF8ToWide(const char* src, size_t src_len, base::StringW* output) {
  return UTFConversion(base::StringRef(src, src_len), output);
}

bool UTF8ToWide(const char8_t* src, size_t src_len, base::StringW* output) {
  return UTFConversion(base::StringRefU8(src, src_len), output);
}

base::StringW UTF8ToWide(base::StringRefU8 utf8) {
  base::StringW ret;
  // Ignore the success flag of this call, it will do the best it can for
  // invalid input, which is what we want here.
  UTF8ToWide(utf8.data(), utf8.length(), &ret);
  return ret;
}

inline const char16_t* as_u16cstr(base::StringRefW str) {
  return reinterpret_cast<const char16_t*>(str.data());
}

bool WideToUTF8(const wchar_t* src, size_t src_len, base::StringU8* output) {
  return UTF16ToUTF8(as_u16cstr(src), src_len, output);
}

base::StringU8 WideToUTF8(const base::StringRefW wide) {
  return UTF16ToUTF8(base::StringRefU16(as_u16cstr(wide), wide.size()));
}

base::StringU16 ASCIIToUTF16(const base::StringRef ascii) {
  BUGCHECK(IsStringASCII(ascii));
  return base::StringU16(reinterpret_cast<const char16_t*>(ascii.begin()),
                         reinterpret_cast<const char16_t*>(ascii.end()));
}

base::String UTF16ToASCII(const base::StringRefU16 utf16) {
  BUGCHECK(IsStringASCII(utf16));
  return base::String(reinterpret_cast<const char*>(utf16.begin()),
                      reinterpret_cast<const char*>(utf16.end()));
}

#if defined(WCHAR_T_IS_UTF16)
base::StringW ASCIIToWide(const base::StringRef ascii) {
  BUGCHECK(IsStringASCII(ascii));
  return base::StringW(reinterpret_cast<const wchar_t*>(ascii.begin()),
                       reinterpret_cast<const wchar_t*>(ascii.end()));
}

base::String WideToASCII(const base::StringRefW wide) {
  BUGCHECK(IsStringASCII(wide));
  return base::String(reinterpret_cast<const char*>(wide.begin()),
                      reinterpret_cast<const char*>(wide.end()));
}
#endif  // defined(WCHAR_T_IS_UTF16)

}  // namespace base