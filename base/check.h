// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Defines the global CHECK utility macros.
#pragma once

#include <base/compiler.h>
#include <base/source_location.h>

#define TK_UNUSED(expr) (void)expr

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
using AssertHandler = void(const char*, const char*, const char*);

// Those are check handlers.
void SetAssertHandler(AssertHandler*);
}  // namespace base

#if defined(CONFIG_DEBUG)
#define CHECK_BREAK DEBUG_TRAP
#else
#define CHECK_BREAK /*noop*/
#endif

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
#define DCHECK(expression, ...)                          \
  do {                                                   \
    if (!(expression)) [[unlikely]] {                    \
      MAKE_SOURCE_LOC(__FUNCTION__, __FILE__, __LINE__); \
      ::base::detail::DCheck(kSourceLoc __VA_OPT__);     \
      CHECK_BREAK;                                       \
    }                                                    \
  } while (0);
#else
#define DCHECK(x, ...)
#endif

// BugChecks indicate a hard programmer error and are compiled into shipping builds
// aswell, as these need to be immedeatly fixed
#define BUGCHECK(expression, ...)                        \
  do {                                                   \
    if (!(expression)) [[unlikely]] {                    \
      MAKE_SOURCE_LOC(__FUNCTION__, __FILE__, __LINE__); \
      ::base::detail::BugCheck(kSourceLoc __VA_OPT__);   \
      CHECK_BREAK;                                       \
    }                                                    \
  } while (0);

// Another form of bugcheck.
#define IMPOSSIBLE                                     \
  {                                                    \
    MAKE_SOURCE_LOC(__FUNCTION__, __FILE__, __LINE__); \
    ::base::detail::BugCheck(kSourceLoc);              \
    CHECK_BREAK;                                       \
  }