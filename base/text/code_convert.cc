// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/text/code_convert.h>

#include <cstdint>
#include <limits.h>
#include <type_traits>

#include <base/check.h>
#include <base/external/icu/icu_utf.h>
#include <base/strings/string_ref.h>
#include <base/text/code_point_validation.h>
#include <build/build_config.h>

namespace base {
namespace {

constexpr base_icu::UChar32 kErrorCodePoint = 0xFFFD;

// Upper bound on output length (in destination code units) per source code
// unit. Worst cases:
//   UTF-16 in -> UTF-8 out: 3 bytes per BMP char.
//   UTF-32 in -> UTF-8 out: 4 bytes per char.
//   UTF-32 in -> UTF-16 out: 2 char16_t per char (surrogate pair).
// Everything else is bounded by 1.
template <typename SrcChar, typename DestChar>
struct SizeCoefficient {
  static constexpr mem_size value = 1;
};
template <>
struct SizeCoefficient<char16_t, char> {
  static constexpr mem_size value = 3;
};
template <>
struct SizeCoefficient<char16_t, char8_t> {
  static constexpr mem_size value = 3;
};
#if defined(WCHAR_T_IS_UTF32)
template <>
struct SizeCoefficient<wchar_t, char> {
  static constexpr mem_size value = 4;
};
template <>
struct SizeCoefficient<wchar_t, char8_t> {
  static constexpr mem_size value = 4;
};
template <>
struct SizeCoefficient<wchar_t, char16_t> {
  static constexpr mem_size value = 2;
};
#endif

template <typename SrcChar, typename DestChar>
constexpr mem_size size_coefficient_v =
    SizeCoefficient<std::decay_t<SrcChar>, std::decay_t<DestChar>>::value;

// Appends a single codepoint to `out`, advancing `*size`. Caller must
// ensure enough space via the size coefficient above.
template <typename Char, std::enable_if_t<CHAR_BIT * sizeof(Char) == 8, int> = 0>
void UnicodeAppendUnsafe(Char* out, int32_t* size, uint32_t code_point) {
  CBU8_APPEND_UNSAFE(out, *size, code_point);
}
template <typename Char, std::enable_if_t<CHAR_BIT * sizeof(Char) == 16, int> = 0>
void UnicodeAppendUnsafe(Char* out, int32_t* size, uint32_t code_point) {
  CBU16_APPEND_UNSAFE(out, *size, code_point);
}
template <typename Char, std::enable_if_t<CHAR_BIT * sizeof(Char) == 32, int> = 0>
void UnicodeAppendUnsafe(Char* out, int32_t* size, uint32_t code_point) {
  out[(*size)++] = code_point;
}

template <typename Char>
bool IsStringASCII(base::BasicStringRef<Char> str) {
  return DoIsStringASCII(str.data(), str.length());
}

// UTF-8 source (both char and char8_t share the same decoder).
template <typename SrcChar, typename DestChar>
bool DecodeUtf8(const SrcChar* src, int32_t src_len, DestChar* dest,
                int32_t* dest_len) {
  static_assert(CHAR_BIT * sizeof(SrcChar) == 8, "UTF-8 source must be 8-bit");
  bool success = true;
  for (int32_t i = 0; i < src_len;) {
    int32_t cp;
    CBU8_NEXT(src, i, src_len, cp);
    if (!IsValidCodepoint(cp)) {
      success = false;
      cp = kErrorCodePoint;
    }
    UnicodeAppendUnsafe(dest, dest_len, cp);
  }
  return success;
}

// UTF-16 source with surrogate-pair handling.
template <typename DestChar>
bool DecodeUtf16(const char16_t* src, int32_t src_len, DestChar* dest,
                 int32_t* dest_len) {
  bool success = true;
  int32_t i = 0;
  while (i < src_len - 1) {
    int32_t cp;
    if (CBU16_IS_LEAD(src[i]) && CBU16_IS_TRAIL(src[i + 1])) {
      cp = CBU16_GET_SUPPLEMENTARY(src[i], src[i + 1]);
      i += 2;
      if (!IsValidCodepoint(cp)) {
        success = false;
        cp = kErrorCodePoint;
      }
    } else {
      char16_t c = src[i++];
      if (!CBU16_IS_SINGLE(c) || !IsValidCodepoint(c)) {
        success = false;
        cp = kErrorCodePoint;
      } else {
        cp = c;
      }
    }
    UnicodeAppendUnsafe(dest, dest_len, cp);
  }
  if (i < src_len) {
    // Trailing code unit must be a BMP scalar to be valid.
    char16_t c = src[i];
    int32_t cp;
    if (!CBU16_IS_SINGLE(c) || !IsValidCodepoint(c)) {
      success = false;
      cp = kErrorCodePoint;
    } else {
      cp = c;
    }
    UnicodeAppendUnsafe(dest, dest_len, cp);
  }
  return success;
}

#if defined(WCHAR_T_IS_UTF32)
// UTF-32 source (used when wchar_t is 32-bit).
template <typename DestChar>
bool DecodeUtf32(const wchar_t* src, int32_t src_len, DestChar* dest,
                 int32_t* dest_len) {
  bool success = true;
  for (int32_t i = 0; i < src_len; ++i) {
    int32_t cp = static_cast<int32_t>(src[i]);
    if (!IsValidCodepoint(cp)) {
      success = false;
      cp = kErrorCodePoint;
    }
    UnicodeAppendUnsafe(dest, dest_len, cp);
  }
  return success;
}
#endif

// Common driver used by the two-argument public overloads.
template <typename InputString, typename DestString, typename Decoder>
bool Convert(const InputString& src, DestString* dest, Decoder&& decoder) {
  using SrcChar = typename InputString::value_type;
  using DestChar = typename DestString::value_type;

  if (src.empty()) {
    dest->clear();
    return true;
  }

  // ASCII fast path: same code unit count, values pass through unchanged.
  if (IsStringASCII(src)) {
    dest->resize(src.size());
    for (mem_size i = 0; i < src.size(); ++i) {
      (*dest)[i] = static_cast<DestChar>(src[i]);
    }
    return true;
  }

  dest->resize(src.length() * size_coefficient_v<SrcChar, DestChar>);
  int32_t dest_len = 0;
  bool ok = decoder(src.data(), static_cast<int32_t>(src.length()),
                    &(*dest)[0], &dest_len);
  dest->resize(dest_len);
  return ok;
}

}  // namespace

// UTF-8 <-> UTF-16.

bool UTF8ToUTF16(const char* src, mem_size src_len, base::StringU16* output) {
  return Convert(base::StringRef(src, src_len), output,
                 [](const char* s, int32_t n, char16_t* d, int32_t* dn) {
                   return DecodeUtf8(s, n, d, dn);
                 });
}

base::StringU16 UTF8ToUTF16(base::StringRef utf8) {
  base::StringU16 out;
  UTF8ToUTF16(utf8.data(), utf8.size(), &out);
  return out;
}

bool UTF16ToUTF8(const char16_t* src, mem_size src_len, base::StringU8* output) {
  return Convert(base::StringRefU16(src, src_len), output,
                 [](const char16_t* s, int32_t n, char8_t* d, int32_t* dn) {
                   return DecodeUtf16(s, n, d, dn);
                 });
}

base::StringU8 UTF16ToUTF8(base::StringRefU16 utf16) {
  base::StringU8 out;
  UTF16ToUTF8(utf16.data(), utf16.length(), &out);
  return out;
}

// UTF-8 <-> wide.

bool UTF8ToWide(const char* src, mem_size src_len, base::StringW* output) {
  return Convert(base::StringRef(src, src_len), output,
                 [](const char* s, int32_t n, wchar_t* d, int32_t* dn) {
                   return DecodeUtf8(s, n, d, dn);
                 });
}

bool UTF8ToWide(const char8_t* src, mem_size src_len, base::StringW* output) {
  return Convert(base::StringRefU8(src, src_len), output,
                 [](const char8_t* s, int32_t n, wchar_t* d, int32_t* dn) {
                   return DecodeUtf8(s, n, d, dn);
                 });
}

base::StringW UTF8ToWide(base::StringRefU8 utf8) {
  base::StringW out;
  UTF8ToWide(utf8.data(), utf8.length(), &out);
  return out;
}

#if defined(WCHAR_T_IS_UTF16)

bool WideToUTF8(const wchar_t* src, mem_size src_len, base::StringU8* output) {
  return UTF16ToUTF8(reinterpret_cast<const char16_t*>(src), src_len, output);
}

base::StringU8 WideToUTF8(base::StringRefW wide) {
  return UTF16ToUTF8(
      base::StringRefU16(reinterpret_cast<const char16_t*>(wide.data()),
                         wide.size()));
}

bool WideToUTF16(const wchar_t* src, mem_size src_len, base::StringU16* output) {
  output->assign(reinterpret_cast<const char16_t*>(src),
                 static_cast<base::StringU16::size_type>(src_len));
  return true;
}

base::StringU16 WideToUTF16(base::StringRefW wide) {
  return base::StringU16(reinterpret_cast<const char16_t*>(wide.data()),
                         static_cast<base::StringU16::size_type>(wide.size()));
}

bool UTF16ToWide(const char16_t* src, mem_size src_len, base::StringW* output) {
  output->assign(reinterpret_cast<const wchar_t*>(src),
                 static_cast<base::StringW::size_type>(src_len));
  return true;
}

base::StringW UTF16ToWide(base::StringRefU16 utf16) {
  return base::StringW(reinterpret_cast<const wchar_t*>(utf16.data()),
                       static_cast<base::StringW::size_type>(utf16.length()));
}

#elif defined(WCHAR_T_IS_UTF32)

bool WideToUTF8(const wchar_t* src, mem_size src_len, base::StringU8* output) {
  return Convert(base::StringRefW(src, src_len), output,
                 [](const wchar_t* s, int32_t n, char8_t* d, int32_t* dn) {
                   return DecodeUtf32(s, n, d, dn);
                 });
}

base::StringU8 WideToUTF8(base::StringRefW wide) {
  base::StringU8 out;
  WideToUTF8(wide.data(), wide.size(), &out);
  return out;
}

bool WideToUTF16(const wchar_t* src, mem_size src_len, base::StringU16* output) {
  return Convert(base::StringRefW(src, src_len), output,
                 [](const wchar_t* s, int32_t n, char16_t* d, int32_t* dn) {
                   return DecodeUtf32(s, n, d, dn);
                 });
}

base::StringU16 WideToUTF16(base::StringRefW wide) {
  base::StringU16 out;
  WideToUTF16(wide.data(), wide.size(), &out);
  return out;
}

bool UTF16ToWide(const char16_t* src, mem_size src_len, base::StringW* output) {
  return Convert(base::StringRefU16(src, src_len), output,
                 [](const char16_t* s, int32_t n, wchar_t* d, int32_t* dn) {
                   return DecodeUtf16(s, n, d, dn);
                 });
}

base::StringW UTF16ToWide(base::StringRefU16 utf16) {
  base::StringW out;
  UTF16ToWide(utf16.data(), utf16.length(), &out);
  return out;
}

#else
#error wchar_t width is undefined; expected WCHAR_T_IS_UTF16 or WCHAR_T_IS_UTF32
#endif

base::StringW ASCIIToWide(base::StringRef ascii) {
  BASE_DCHECK(IsStringASCII(ascii));
  base::StringW out;
  out.resize(static_cast<base::StringW::size_type>(ascii.size()));
  for (mem_size i = 0; i < ascii.size(); ++i) {
    out[i] = static_cast<wchar_t>(static_cast<unsigned char>(ascii[i]));
  }
  return out;
}

base::String WideToASCII(base::StringRefW wide) {
  BASE_BUGCHECK(IsStringASCII(wide));
  base::String out;
  out.resize(static_cast<base::String::size_type>(wide.size()));
  for (mem_size i = 0; i < wide.size(); ++i) {
    out[i] = static_cast<char>(wide.data()[i] & 0x7F);
  }
  return out;
}

}  // namespace base
