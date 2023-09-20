// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Utils for creating source_locations.
#pragma once

#include <base/arch.h>
#include <base/compiler.h>

namespace base {
// A source location contains format text and the file name, that is truncated at
// compile time.
struct SourceLocation {
  const char* format;
  const char* file;
  const char* func;
};

}  // namespace base

namespace base::detail {

mem_size consteval CountStrlen(const char* str) {
  return *str ? 1 + CountStrlen(str + 1) : 0;
}

// Small utility function to trim down the source path of any function
consteval const char* TrimSourcePath(const char* const str,
                                           const char* const lastslash = nullptr) {
  return *str ? TrimSourcePath(str + 1,
                               ((*str == '/' || *str == '\\')
                                    ? str + 1
                                    : (nullptr == lastslash ? str : lastslash)))
              : (nullptr == lastslash ? str : lastslash);
}

static constexpr int PastLastOffset(int last_offset,
                                    int current_offset,
                                    const char* const str) {
  if (*str == '\0')
    return last_offset;
  if (*str == '/' || *str == '\\')
    return PastLastOffset(current_offset + 1, current_offset + 1, str + 1);
  return PastLastOffset(last_offset, current_offset + 1, str + 1);
}

// based off: https://stackoverflow.com/questions/56471708/c-compile-time-substring
// TODO(Force): move this to some compile time string util.
template <int... I>
struct Seq {};

template <int V, typename T>
struct Push;

template <int V, int... I>
struct Push<V, Seq<I...>> {
  using type = Seq<V, I...>;
};

template <int From, int To>
struct MakeSeqImpl;

template <int To>
struct MakeSeqImpl<To, To> {
  using type = Seq<To>;
};

template <int From, int To>
using MakeSeq = typename MakeSeqImpl<From, To>::type;

template <int From, int To>
struct MakeSeqImpl : Push<From, MakeSeq<From + 1, To>> {};

template <char... CHARS>
struct Chars {
  static constexpr const char value[] = {CHARS...};
};
template <char... CHARS>
constexpr const char Chars<CHARS...>::value[];

template <typename WRAPPER, typename IDXS>
struct LiteralToVariadicCharsImpl;

template <typename WRAPPER, int... IDXS>
struct LiteralToVariadicCharsImpl<WRAPPER, Seq<IDXS...>> {
  using type = Chars<WRAPPER::get()[IDXS]...>;
};

template <typename WRAPPER, typename SEQ>
struct LiteralToVariadicChars {
  using type = typename LiteralToVariadicCharsImpl<WRAPPER, SEQ>::type;
};
}  // namespace base::detail

#ifndef EVAL_MACRO_
#define EVAL_MACRO_(x) #x
#define EVAL_MACRO__(x) EVAL_MACRO_(x)
#endif

#define COMPILE_TIME_PAST_LAST_SLASH(STR)                                       \
  []() {                                                                        \
    struct Wrapper {                                                            \
      constexpr static const char* get() { return STR; }                        \
    };                                                                          \
    using Seq =                                                                 \
        ::base::detail::MakeSeq<::base::detail::PastLastOffset(0, 0,            \
                                                               Wrapper::get()), \
                                ::base::detail::CountStrlen(Wrapper::get())>;   \
    return ::base::detail::LiteralToVariadicChars<Wrapper, Seq>::type::value;   \
  }()

#define MAKE_SOURCE_LOC(function, file, line)                                      \
  STATIC_MSVC_ONLY CONSTINIT_MSVC_ONLY const ::base::SourceLocation kSourceLoc {   \
    PROJECT_NAME "!{}!{}!" EVAL_MACRO__(line), COMPILE_TIME_PAST_LAST_SLASH(file), \
        function                                                                   \
  }
