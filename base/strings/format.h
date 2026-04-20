// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// nanofmt-style {} formatter. STL-free.
//
// Public API:
//   base::FormatTo(char* buf, mem_size n, fmt, args...)  -> mem_size
//   base::FormatTo(base::String& out, fmt, args...)       -> void
//   base::Format(fmt, args...)                            -> base::String
//
// Format syntax (subset of std::format):
//   {}                                  auto-indexed
//   {N}                                 explicit positional
//   {:[[fill]align][sign][#][0][width][.precision][type]}
//
//     align    < > ^                    left / right / centered
//     sign     - + space                numeric only
//     #                                 alt form (0x / 0b / 0)
//     0                                 numeric zero-pad
//     width    decimal                  minimum field width
//     .prec    decimal                  string truncation, float decimals
//     type     d x X o b B c            integers
//              f F e E g G              floats
//              s                        strings
//              p                        pointer (forces 0xHEX)
//
// Argument types: bool, integral, floating, char, char8_t/char16_t/char32_t/wchar_t
// (and pointers to them: wide strings transcode to UTF-8 on output),
// const char*, base::String, base::StringRef, void*, nullptr_t.
//
// `{{` and `}}` escape literal braces.
#pragma once

#include <base/arch.h>
#include <base/export.h>
#include <base/meta/traits.h>
#include <base/strings/string_ref.h>
#include <base/strings/xstring.h>

namespace base {
namespace fmt_detail {

struct Arg {
  enum class Tag : u8 {
    kNone,
    kBool,
    kChar,
    kI64,
    kU64,
    kF64,
    kCStr,
    kStrSpan,
    kU16Str,  // const char16_t*, transcoded to UTF-8 on output
    kU32Str,  // const char32_t*, transcoded to UTF-8 on output
    kPtr,
  };

  struct StrSpan {
    const char* data;
    mem_size length;
  };

  Tag tag;
  union {
    bool b;
    char c;
    i64 i;
    u64 u;
    f64 d;
    const char* cstr;
    StrSpan span;
    const char16_t* u16str;
    const char32_t* u32str;
    const void* ptr;
  };
};

struct Spec {
  char fill = ' ';
  char align = '\0';
  char sign = '-';
  char type = '\0';
  bool alt = false;
  bool zero_pad = false;
  int width = 0;
  int precision = -1;
};

struct Sink {
  void* state;
  void (*write_fn)(void* state, const char* data, mem_size n);

  void Write(const char* data, mem_size n) noexcept {
    if (n) write_fn(state, data, n);
  }
  void WriteChar(char c) noexcept { write_fn(state, &c, 1); }
};

inline Arg MakeArg(bool v) noexcept {
  Arg a; a.tag = Arg::Tag::kBool; a.b = v; return a;
}
inline Arg MakeArg(char v) noexcept {
  Arg a; a.tag = Arg::Tag::kChar; a.c = v; return a;
}
inline Arg MakeArg(char8_t v) noexcept {
  Arg a; a.tag = Arg::Tag::kChar; a.c = static_cast<char>(v); return a;
}
template <typename T>
  requires(base::is_integral_v<T> && !base::is_same_v<T, bool> &&
           !base::is_same_v<T, char> && !base::is_same_v<T, char8_t> &&
           !base::is_same_v<T, char16_t> && !base::is_same_v<T, char32_t> &&
           !base::is_same_v<T, wchar_t>)
inline Arg MakeArg(T v) noexcept {
  Arg a;
  if constexpr (base::is_signed_v<T>) {
    a.tag = Arg::Tag::kI64;
    a.i = static_cast<i64>(v);
  } else {
    a.tag = Arg::Tag::kU64;
    a.u = static_cast<u64>(v);
  }
  return a;
}
inline Arg MakeArg(float v) noexcept {
  Arg a; a.tag = Arg::Tag::kF64; a.d = static_cast<f64>(v); return a;
}
inline Arg MakeArg(double v) noexcept {
  Arg a; a.tag = Arg::Tag::kF64; a.d = v; return a;
}

template <mem_size N>
inline Arg MakeArg(const char (&v)[N]) noexcept {
  Arg a;
  a.tag = Arg::Tag::kStrSpan;
  a.span.data = v;
  a.span.length = (N > 0 && v[N - 1] == '\0') ? N - 1 : N;
  return a;
}
template <mem_size N>
inline Arg MakeArg(const char8_t (&v)[N]) noexcept {
  Arg a;
  a.tag = Arg::Tag::kStrSpan;
  a.span.data = reinterpret_cast<const char*>(v);
  a.span.length = (N > 0 && v[N - 1] == u8'\0') ? N - 1 : N;
  return a;
}
inline Arg MakeArg(const char* v) noexcept {
  Arg a;
  a.tag = Arg::Tag::kCStr;
  a.cstr = v ? v : "(null)";
  return a;
}
inline Arg MakeArg(char* v) noexcept {
  return MakeArg(static_cast<const char*>(v));
}
inline Arg MakeArg(const char8_t* v) noexcept {
  return MakeArg(reinterpret_cast<const char*>(v));
}
inline Arg MakeArg(char8_t* v) noexcept {
  return MakeArg(reinterpret_cast<const char*>(v));
}
inline Arg MakeArg(const char16_t* v) noexcept {
  Arg a; a.tag = Arg::Tag::kU16Str; a.u16str = v; return a;
}
inline Arg MakeArg(char16_t* v) noexcept {
  return MakeArg(static_cast<const char16_t*>(v));
}
inline Arg MakeArg(const char32_t* v) noexcept {
  Arg a; a.tag = Arg::Tag::kU32Str; a.u32str = v; return a;
}
inline Arg MakeArg(char32_t* v) noexcept {
  return MakeArg(static_cast<const char32_t*>(v));
}
inline Arg MakeArg(const wchar_t* v) noexcept {
  if constexpr (sizeof(wchar_t) == 2) {
    return MakeArg(reinterpret_cast<const char16_t*>(v));
  } else {
    return MakeArg(reinterpret_cast<const char32_t*>(v));
  }
}
inline Arg MakeArg(wchar_t* v) noexcept {
  return MakeArg(static_cast<const wchar_t*>(v));
}

template <typename TAlloc, typename TSize>
inline Arg MakeArg(const base::BasicBaseString<char, TSize, TAlloc>& s) noexcept {
  Arg a;
  a.tag = Arg::Tag::kStrSpan;
  a.span.data = s.data();
  a.span.length = static_cast<mem_size>(s.size());
  return a;
}
template <typename TAlloc, typename TSize>
inline Arg MakeArg(const base::BasicBaseString<char8_t, TSize, TAlloc>& s) noexcept {
  Arg a;
  a.tag = Arg::Tag::kStrSpan;
  a.span.data = reinterpret_cast<const char*>(s.data());
  a.span.length = static_cast<mem_size>(s.size());
  return a;
}
template <typename TAlloc, typename TSize>
inline Arg MakeArg(const base::BasicBaseString<char16_t, TSize, TAlloc>& s) noexcept {
  return MakeArg(s.c_str());
}
template <typename TAlloc, typename TSize>
inline Arg MakeArg(const base::BasicBaseString<char32_t, TSize, TAlloc>& s) noexcept {
  return MakeArg(s.c_str());
}
template <typename TAlloc, typename TSize>
inline Arg MakeArg(const base::BasicBaseString<wchar_t, TSize, TAlloc>& s) noexcept {
  return MakeArg(s.c_str());
}
inline Arg MakeArg(base::StringRef s) noexcept {
  Arg a;
  a.tag = Arg::Tag::kStrSpan;
  a.span.data = s.data();
  a.span.length = s.length();
  return a;
}
inline Arg MakeArg(base::StringRefU8 s) noexcept {
  Arg a;
  a.tag = Arg::Tag::kStrSpan;
  a.span.data = reinterpret_cast<const char*>(s.data());
  a.span.length = s.length();
  return a;
}

template <typename T>
  requires(!base::is_same_v<T, char> && !base::is_same_v<T, const char> &&
           !base::is_same_v<T, char8_t> && !base::is_same_v<T, const char8_t> &&
           !base::is_same_v<T, char16_t> && !base::is_same_v<T, const char16_t> &&
           !base::is_same_v<T, char32_t> && !base::is_same_v<T, const char32_t> &&
           !base::is_same_v<T, wchar_t> && !base::is_same_v<T, const wchar_t>)
inline Arg MakeArg(T* v) noexcept {
  Arg a; a.tag = Arg::Tag::kPtr; a.ptr = static_cast<const void*>(v); return a;
}
inline Arg MakeArg(const void* v) noexcept {
  Arg a; a.tag = Arg::Tag::kPtr; a.ptr = v; return a;
}
inline Arg MakeArg(decltype(nullptr)) noexcept {
  Arg a; a.tag = Arg::Tag::kPtr; a.ptr = nullptr; return a;
}

BASE_EXPORT void FormatCore(Sink& sink, const char* fmt, const Arg* args,
                            mem_size argc) noexcept;

struct FixedBufferState {
  char* dst;
  mem_size capacity;
  mem_size written;
};

inline void FixedBufferWrite(void* state, const char* data, mem_size n) noexcept {
  auto* s = static_cast<FixedBufferState*>(state);
  if (s->written >= s->capacity) {
    s->written += n;
    return;
  }
  mem_size room = s->capacity - s->written;
  mem_size copy = n < room ? n : room;
  for (mem_size i = 0; i < copy; ++i) s->dst[s->written + i] = data[i];
  s->written += n;
}

inline void StringWrite(void* state, const char* data, mem_size n) noexcept {
  auto* s = static_cast<base::String*>(state);
  s->append(data, static_cast<base::String::size_type>(n));
}

}  // namespace fmt_detail

// Writes formatted output into `buffer`, always null-terminating when
// buffer_size > 0. Returns the full length the formatted output would have
// had (snprintf semantics). A return value >= buffer_size means truncation.
template <typename... Args>
inline mem_size FormatTo(char* buffer, mem_size buffer_size, const char* fmt,
                         const Args&... args) noexcept {
  if (buffer_size == 0) return 0;
  fmt_detail::FixedBufferState state{buffer, buffer_size - 1, 0};
  fmt_detail::Sink sink{&state, &fmt_detail::FixedBufferWrite};
  if constexpr (sizeof...(Args) == 0) {
    fmt_detail::FormatCore(sink, fmt, nullptr, 0);
  } else {
    fmt_detail::Arg a[sizeof...(Args)] = {fmt_detail::MakeArg(args)...};
    fmt_detail::FormatCore(sink, fmt, a, sizeof...(Args));
  }
  mem_size term = state.written < state.capacity ? state.written : state.capacity;
  buffer[term] = '\0';
  return state.written;
}

template <typename... Args>
inline void FormatTo(base::String& out, const char* fmt, const Args&... args) {
  fmt_detail::Sink sink{&out, &fmt_detail::StringWrite};
  if constexpr (sizeof...(Args) == 0) {
    fmt_detail::FormatCore(sink, fmt, nullptr, 0);
  } else {
    fmt_detail::Arg a[sizeof...(Args)] = {fmt_detail::MakeArg(args)...};
    fmt_detail::FormatCore(sink, fmt, a, sizeof...(Args));
  }
}

template <typename... Args>
inline base::String Format(const char* fmt, const Args&... args) {
  base::String out;
  FormatTo(out, fmt, args...);
  return out;
}

}  // namespace base
