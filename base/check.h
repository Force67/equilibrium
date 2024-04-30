// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Defines the global CHECK utility macros.
#pragma once

#include <base/compiler.h>
#include <base/meta/source_location.h>

#define TK_UNUSED(expr) (void)expr
#define BASE_RECORD_CHECKS
#ifdef BUGCHECK
#error Something else defined BUGCHECK
#endif

#ifdef DCHECK
#error Something else defined DCHECK
#endif

namespace base {
// implementation detail
namespace detail {
// Second parameter defaults to null, so the __VA_OPT__ macro can insert the param
// if needed.
void DCheck(const SourceLocation&, const char* message = nullptr);
void BugCheck(const SourceLocation&, const char* message = nullptr);
}  // namespace detail

// Asserts are user facing exceptional cases, after which the program state is
// expected to be broken. A key philosphy of our system is to ensure the user
// gets to see the assert.
using CheckHandler = void(const char*, const char*, const char*, const char*);

// Those are check handlers.
void SetCheckHandler(CheckHandler*);
}  // namespace base

// NOTE(Vince): do not apply the [[likely]] or [[unlikely]] attributes here, as these
// actively harm optimization. See
// https://blog.aaronballman.com/2020/08/dont-use-the-likely-or-unlikely-attributes/

// All checks follow the format:
// check: project!file.cc!function!line >conidition< (Reason)

// A DCHECK is only present in non shipping builds and is ment for catching
// programmer misuse that needs to be fixed before release
//
// Q: How do DCHECKs relate to asserts?
// A: As asserts usually get stripped in shipping builds, a dcheck is the
//    logical choice to use in cases which can be triggered and fixed during
//    development.

#ifndef CONFIG_SHIPPING

#if defined(BASE_RECORD_CHECKS)
#define DCHECK(expression, ...)                            \
  do {                                                     \
    if (!(expression)) {                                   \
      MAKE_SOURCE_LOC(BASE_FUNC_NAME, __FILE__, __LINE__); \
      ::base::detail::DCheck(kSourceLoc, ##__VA_ARGS__);   \
      CHECK_BREAK;                                         \
    }                                                      \
  } while (0);
#else
#define DCHECK(expression, ...) \
  do {                          \
    if (!(expression)) {        \
      CHECK_BREAK;              \
    }                           \
  } while (0);
#endif

// constexpr if no dcheck
#define CONSTEXPR_ND
#define CONST_ND

#else
#define DCHECK(x, ...)
#define CONSTEXPR_ND constexpr
#define CONST_ND const
#endif

// BugChecks indicate a hard programmer error and are compiled into shipping builds
// aswell, as these need to be immedeatly fixed
#ifndef BASE_STRIP_BUGCHECK

#if defined(BASE_RECORD_CHECKS)
#define BUGCHECK(expression, ...)                          \
  do {                                                     \
    if (!(expression)) {                                   \
      MAKE_SOURCE_LOC(BASE_FUNC_NAME, __FILE__, __LINE__); \
      ::base::detail::BugCheck(kSourceLoc, ##__VA_ARGS__); \
      CHECK_BREAK;                                         \
    }                                                      \
  } while (0);
#else
#define BUGCHECK(expression, ...) \
  do {                            \
    if (!(expression)) {          \
      CHECK_BREAK;                \
    }                             \
  } while (0);
#endif
#else

#define BUGCHECK(x, ...)
#endif

#if defined(BASE_RECORD_CHECKS)
// Another form of bugcheck.
#define IMPOSSIBLE                                       \
  {                                                      \
    MAKE_SOURCE_LOC(BASE_FUNC_NAME, __FILE__, __LINE__); \
    ::base::detail::BugCheck(kSourceLoc);                \
    CHECK_BREAK;                                         \
  }
// newline
#else
#define IMPOSSIBLE \
  { CHECK_BREAK; }
#endif