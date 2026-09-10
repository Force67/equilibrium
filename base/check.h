// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Defines the global CHECK utility macros.
#pragma once

#include <base/compiler.h>
#include <base/export.h>
#include <base/meta/source_location.h>

#define TK_UNUSED(expr) (void)expr

// #define BASE_RECORD_CHECKS
#ifdef BASE_BUGCHECK
#error Something else defined BASE_BUGCHECK
#endif

#ifdef BASE_DCHECK
#error Something else defined BASE_DCHECK
#endif

namespace base {
// implementation detail
namespace detail {
// Second parameter defaults to null, so the __VA_OPT__ macro can insert the
// param if needed.
BASE_EXPORT void DCheck(const SourceLocation&, const char* message = nullptr);
BASE_EXPORT void BugCheck(const SourceLocation&, const char* message = nullptr);
}  // namespace detail

// Asserts are user facing exceptional cases, after which the program state is
// expected to be broken. A key philosphy of our system is to ensure the user
// gets to see the assert.
using CheckHandler = void(const char*, const char*, const char*, const char*);

// Those are check handlers.
BASE_EXPORT void SetCheckHandler(CheckHandler*);
}  // namespace base

// Do not apply [[likely]]/[[unlikely]] here; they actively harm optimization.
// See https://blog.aaronballman.com/2020/08/dont-use-the-likely-or-unlikely-attributes/

// All checks follow the format:
// check: project!file.cc!function!line >condition< (Reason)

// A DCHECK exists only in non-shipping builds. Use it for programmer misuse
// that must be fixed before release; plain asserts are usually stripped in
// shipping builds.

#ifndef CONFIG_SHIPPING

#if defined(BASE_RECORD_CHECKS)
#define BASE_DCHECK(expression, ...)                       \
  do {                                                     \
    if (!(expression)) {                                   \
      MAKE_SOURCE_LOC(BASE_FUNC_NAME, __FILE__, __LINE__); \
      ::base::detail::DCheck(kSourceLoc, ##__VA_ARGS__);   \
      CHECK_BREAK;                                         \
    }                                                      \
  } while (0);
#else
#define BASE_DCHECK(expression, ...) \
  do {                               \
    if (!(expression)) {             \
      CHECK_BREAK;                   \
    }                                \
  } while (0);
#endif

// constexpr if no dcheck
#define BASE_CONSTEXPR_ND
#define BASE_CONST_ND

#else
#define BASE_DCHECK(x, ...)
#define BASE_CONSTEXPR_ND constexpr
#define BASE_CONST_ND const
#endif

// BugChecks indicate a hard programmer error and are compiled into shipping
// builds aswell, as these need to be immedeatly fixed
#ifndef BASE_STRIP_BUGCHECK

#if defined(BASE_RECORD_CHECKS)
#define BASE_BUGCHECK(expression, ...)                     \
  do {                                                     \
    if (!(expression)) {                                   \
      MAKE_SOURCE_LOC(BASE_FUNC_NAME, __FILE__, __LINE__); \
      ::base::detail::BugCheck(kSourceLoc, ##__VA_ARGS__); \
      CHECK_BREAK;                                         \
    }                                                      \
  } while (0);
#else
#define BASE_BUGCHECK(expression, ...) \
  do {                                 \
    if (!(expression)) {               \
      CHECK_BREAK;                     \
    }                                  \
  } while (0);
#endif
#else

#define BASE_BUGCHECK(x, ...)
#endif

#if defined(BASE_RECORD_CHECKS)
// Another form of bugcheck.
#define BASE_IMPOSSIBLE                                  \
  {                                                      \
    MAKE_SOURCE_LOC(BASE_FUNC_NAME, __FILE__, __LINE__); \
    ::base::detail::BugCheck(kSourceLoc);                \
    CHECK_BREAK;                                         \
  }
// newline
#else
#define BASE_IMPOSSIBLE \
  {                     \
    CHECK_BREAK;        \
  }
#endif