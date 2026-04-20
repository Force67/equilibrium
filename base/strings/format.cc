// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/strings/format.h>

#include <cstdio>
#include <cstring>

namespace base::fmt_detail {
namespace {

inline bool DecDigit(char c, int* out) noexcept {
  if (c < '0' || c > '9') return false;
  *out = c - '0';
  return true;
}

void ParseSpec(const char*& p, const char* end, Spec& s) noexcept {
  if (p + 1 < end && (p[1] == '<' || p[1] == '>' || p[1] == '^') &&
      p[0] != '\0' && p[0] != '{' && p[0] != '}') {
    s.fill = p[0];
    s.align = p[1];
    p += 2;
  } else if (p < end && (*p == '<' || *p == '>' || *p == '^')) {
    s.align = *p++;
  }

  if (p < end && (*p == '+' || *p == '-' || *p == ' ')) {
    s.sign = *p++;
  }

  if (p < end && *p == '#') {
    s.alt = true;
    ++p;
  }

  if (p < end && *p == '0') {
    s.zero_pad = true;
    ++p;
  }

  int digit;
  while (p < end && DecDigit(*p, &digit)) {
    s.width = s.width * 10 + digit;
    ++p;
  }

  if (p < end && *p == '.') {
    ++p;
    s.precision = 0;
    while (p < end && DecDigit(*p, &digit)) {
      s.precision = s.precision * 10 + digit;
      ++p;
    }
  }

  if (p < end && *p != '}') {
    s.type = *p++;
  }
}

void EmitFill(Sink& sink, char c, int n) noexcept {
  char buf[32];
  for (int i = 0; i < (int)sizeof(buf); ++i) buf[i] = c;
  while (n > 0) {
    int chunk = n > (int)sizeof(buf) ? (int)sizeof(buf) : n;
    sink.Write(buf, static_cast<mem_size>(chunk));
    n -= chunk;
  }
}

template <typename BodyFn>
void EmitAligned(Sink& sink, char align, char fill, int body_len, int width,
                 BodyFn&& body_writer) noexcept {
  int pad = width > body_len ? width - body_len : 0;
  if (pad == 0) {
    body_writer();
    return;
  }
  if (align == '<') {
    body_writer();
    EmitFill(sink, fill, pad);
  } else if (align == '^') {
    int left = pad / 2;
    EmitFill(sink, fill, left);
    body_writer();
    EmitFill(sink, fill, pad - left);
  } else {
    EmitFill(sink, fill, pad);
    body_writer();
  }
}

// UTF-8 encode one Unicode codepoint into out (up to 4 bytes). Returns the
// byte count, or 0 for an invalid codepoint.
int EncodeUtf8(u32 cp, char out[4]) noexcept {
  if (cp <= 0x7F) {
    out[0] = static_cast<char>(cp);
    return 1;
  }
  if (cp <= 0x7FF) {
    out[0] = static_cast<char>(0xC0 | (cp >> 6));
    out[1] = static_cast<char>(0x80 | (cp & 0x3F));
    return 2;
  }
  if (cp <= 0xFFFF) {
    if (cp >= 0xD800 && cp <= 0xDFFF) return 0;
    out[0] = static_cast<char>(0xE0 | (cp >> 12));
    out[1] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
    out[2] = static_cast<char>(0x80 | (cp & 0x3F));
    return 3;
  }
  if (cp <= 0x10FFFF) {
    out[0] = static_cast<char>(0xF0 | (cp >> 18));
    out[1] = static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
    out[2] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
    out[3] = static_cast<char>(0x80 | (cp & 0x3F));
    return 4;
  }
  return 0;
}

// UTF-16 to UTF-8 conversion into a stack/heap byte buffer. Counts bytes if
// out is null. Stops at the null terminator.
mem_size TranscodeUtf16(const char16_t* src, char* out, mem_size out_cap) noexcept {
  mem_size written = 0;
  if (!src) return 0;
  while (*src) {
    u32 cp;
    char16_t high = *src++;
    if (high >= 0xD800 && high <= 0xDBFF && *src >= 0xDC00 && *src <= 0xDFFF) {
      char16_t low = *src++;
      cp = 0x10000 + ((static_cast<u32>(high) - 0xD800) << 10) +
           (static_cast<u32>(low) - 0xDC00);
    } else {
      cp = high;
    }
    char tmp[4];
    int n = EncodeUtf8(cp, tmp);
    if (out) {
      for (int i = 0; i < n; ++i) {
        if (written + i < out_cap) out[written + i] = tmp[i];
      }
    }
    written += n;
  }
  return written;
}

mem_size TranscodeUtf32(const char32_t* src, char* out, mem_size out_cap) noexcept {
  mem_size written = 0;
  if (!src) return 0;
  while (*src) {
    char tmp[4];
    int n = EncodeUtf8(static_cast<u32>(*src++), tmp);
    if (out) {
      for (int i = 0; i < n; ++i) {
        if (written + i < out_cap) out[written + i] = tmp[i];
      }
    }
    written += n;
  }
  return written;
}

void FormatString(Sink& sink, const char* data, mem_size length,
                  const Spec& spec) noexcept {
  if (spec.precision >= 0 && length > static_cast<mem_size>(spec.precision)) {
    length = static_cast<mem_size>(spec.precision);
  }
  char align = spec.align ? spec.align : '<';
  EmitAligned(sink, align, spec.fill, static_cast<int>(length), spec.width,
              [&] { sink.Write(data, length); });
}

// Formats a wide string by transcoding it into a small stack buffer, with a
// heap fallback when the result spills past the inline capacity.
template <typename Transcoder>
void FormatWideString(Sink& sink, const Spec& spec, Transcoder&& transcode) {
  char inline_buf[256];
  mem_size needed = transcode(inline_buf, sizeof(inline_buf));
  if (needed <= sizeof(inline_buf)) {
    FormatString(sink, inline_buf, needed, spec);
    return;
  }
  base::String big;
  big.resize(static_cast<base::String::size_type>(needed));
  transcode(big.data(), needed);
  FormatString(sink, big.data(), needed, spec);
}

void FormatInteger(Sink& sink, u64 abs, bool negative,
                   const Spec& spec) noexcept {
  unsigned base = 10;
  bool upper = false;
  const char* prefix = "";
  int prefix_len = 0;

  switch (spec.type) {
    case 'x':
      base = 16;
      if (spec.alt) { prefix = "0x"; prefix_len = 2; }
      break;
    case 'X':
      base = 16;
      upper = true;
      if (spec.alt) { prefix = "0X"; prefix_len = 2; }
      break;
    case 'o':
      base = 8;
      if (spec.alt) { prefix = "0"; prefix_len = 1; }
      break;
    case 'b':
      base = 2;
      if (spec.alt) { prefix = "0b"; prefix_len = 2; }
      break;
    case 'B':
      base = 2;
      if (spec.alt) { prefix = "0B"; prefix_len = 2; }
      break;
    case 'c':
      sink.WriteChar(static_cast<char>(abs));
      return;
    default:
      break;
  }

  char digits[64];
  int digit_count = 0;
  if (abs == 0) {
    digits[digit_count++] = '0';
  } else {
    while (abs) {
      unsigned d = static_cast<unsigned>(abs % base);
      abs /= base;
      digits[digit_count++] =
          (d < 10) ? static_cast<char>('0' + d)
                   : static_cast<char>((upper ? 'A' : 'a') + d - 10);
    }
  }

  char sign_char = '\0';
  if (negative) {
    sign_char = '-';
  } else if (spec.sign == '+') {
    sign_char = '+';
  } else if (spec.sign == ' ') {
    sign_char = ' ';
  }
  int sign_len = sign_char ? 1 : 0;
  int body_len = sign_len + prefix_len + digit_count;

  auto write_digits = [&] {
    for (int i = digit_count - 1; i >= 0; --i) sink.WriteChar(digits[i]);
  };

  // Numeric zero-pad: pad sits between sign/prefix and digits.
  if (spec.zero_pad && !spec.align && spec.width > body_len) {
    int pad = spec.width - body_len;
    if (sign_char) sink.WriteChar(sign_char);
    if (prefix_len) sink.Write(prefix, static_cast<mem_size>(prefix_len));
    EmitFill(sink, '0', pad);
    write_digits();
    return;
  }

  char align = spec.align ? spec.align : '>';
  EmitAligned(sink, align, spec.fill, body_len, spec.width, [&] {
    if (sign_char) sink.WriteChar(sign_char);
    if (prefix_len) sink.Write(prefix, static_cast<mem_size>(prefix_len));
    write_digits();
  });
}

void FormatFloat(Sink& sink, f64 v, const Spec& spec) noexcept {
  char fmt[16];
  int fp = 0;
  fmt[fp++] = '%';
  if (spec.sign == '+') fmt[fp++] = '+';
  else if (spec.sign == ' ') fmt[fp++] = ' ';
  if (spec.alt) fmt[fp++] = '#';
  if (spec.precision >= 0) {
    int n = ::snprintf(fmt + fp, sizeof(fmt) - fp, ".%d", spec.precision);
    if (n > 0) fp += n;
  }
  char type = spec.type;
  if (type == '\0') type = (spec.precision >= 0) ? 'f' : 'g';
  fmt[fp++] = type;
  fmt[fp] = '\0';

  char out[64];
  int n = ::snprintf(out, sizeof(out), fmt, v);
  if (n < 0) return;
  if (n > (int)sizeof(out) - 1) n = (int)sizeof(out) - 1;

  if (spec.zero_pad && !spec.align && spec.width > n) {
    int pad = spec.width - n;
    EmitFill(sink, '0', pad);
    sink.Write(out, static_cast<mem_size>(n));
    return;
  }

  char align = spec.align ? spec.align : '>';
  EmitAligned(sink, align, spec.fill, n, spec.width,
              [&] { sink.Write(out, static_cast<mem_size>(n)); });
}

void FormatBool(Sink& sink, bool v, const Spec& spec) noexcept {
  if (spec.type == 'd' || spec.type == 'x' || spec.type == 'X' ||
      spec.type == 'o' || spec.type == 'b' || spec.type == 'B') {
    FormatInteger(sink, v ? 1u : 0u, false, spec);
    return;
  }
  const char* text = v ? "true" : "false";
  mem_size len = v ? 4 : 5;
  FormatString(sink, text, len, spec);
}

void FormatPointer(Sink& sink, const void* p, const Spec& spec) noexcept {
  Spec s = spec;
  s.alt = true;
  s.type = 'x';
  s.sign = '-';
  FormatInteger(sink, reinterpret_cast<u64>(p), false, s);
}

void FormatArgValue(Sink& sink, const Arg& arg, const Spec& spec) noexcept {
  switch (arg.tag) {
    case Arg::Tag::kBool:
      FormatBool(sink, arg.b, spec);
      return;
    case Arg::Tag::kChar:
      if (spec.type == 'd' || spec.type == 'x' || spec.type == 'X' ||
          spec.type == 'o' || spec.type == 'b' || spec.type == 'B') {
        FormatInteger(sink, static_cast<u64>(static_cast<unsigned char>(arg.c)),
                      false, spec);
      } else {
        FormatString(sink, &arg.c, 1, spec);
      }
      return;
    case Arg::Tag::kI64: {
      bool neg = arg.i < 0;
      // Cast through u64 first so INT64_MIN doesn't overflow on negation.
      u64 abs = neg ? static_cast<u64>(0) - static_cast<u64>(arg.i)
                    : static_cast<u64>(arg.i);
      FormatInteger(sink, abs, neg, spec);
      return;
    }
    case Arg::Tag::kU64:
      FormatInteger(sink, arg.u, false, spec);
      return;
    case Arg::Tag::kF64:
      FormatFloat(sink, arg.d, spec);
      return;
    case Arg::Tag::kCStr: {
      const char* s = arg.cstr ? arg.cstr : "(null)";
      FormatString(sink, s, ::strlen(s), spec);
      return;
    }
    case Arg::Tag::kStrSpan:
      FormatString(sink, arg.span.data, arg.span.length, spec);
      return;
    case Arg::Tag::kU16Str: {
      const char16_t* s = arg.u16str;
      if (!s) {
        FormatString(sink, "(null)", 6, spec);
        return;
      }
      FormatWideString(sink, spec, [&](char* out, mem_size cap) {
        return TranscodeUtf16(s, out, cap);
      });
      return;
    }
    case Arg::Tag::kU32Str: {
      const char32_t* s = arg.u32str;
      if (!s) {
        FormatString(sink, "(null)", 6, spec);
        return;
      }
      FormatWideString(sink, spec, [&](char* out, mem_size cap) {
        return TranscodeUtf32(s, out, cap);
      });
      return;
    }
    case Arg::Tag::kPtr:
      FormatPointer(sink, arg.ptr, spec);
      return;
    case Arg::Tag::kNone:
    default:
      sink.Write("(?)", 3);
      return;
  }
}

}  // namespace

void FormatCore(Sink& sink, const char* fmt, const Arg* args,
                mem_size argc) noexcept {
  if (!fmt) return;

  mem_size auto_index = 0;
  const char* p = fmt;

  while (*p) {
    char c = *p;
    if (c == '{') {
      if (p[1] == '{') {
        sink.WriteChar('{');
        p += 2;
        continue;
      }
      ++p;

      mem_size idx = auto_index;
      bool explicit_idx = false;
      if (*p >= '0' && *p <= '9') {
        idx = 0;
        while (*p >= '0' && *p <= '9') {
          idx = idx * 10 + static_cast<mem_size>(*p - '0');
          ++p;
        }
        explicit_idx = true;
      }

      Spec spec;
      if (*p == ':') {
        ++p;
        const char* spec_end = p;
        while (*spec_end && *spec_end != '}') ++spec_end;
        ParseSpec(p, spec_end, spec);
        p = spec_end;
      }
      if (*p == '}') ++p;

      if (idx >= argc) {
        sink.Write("{?}", 3);
      } else {
        FormatArgValue(sink, args[idx], spec);
        if (!explicit_idx) ++auto_index;
      }
    } else if (c == '}') {
      if (p[1] == '}') {
        sink.WriteChar('}');
        p += 2;
      } else {
        sink.WriteChar('}');
        ++p;
      }
    } else {
      const char* start = p;
      while (*p && *p != '{' && *p != '}') ++p;
      sink.Write(start, static_cast<mem_size>(p - start));
    }
  }
}

}  // namespace base::fmt_detail
