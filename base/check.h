// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Defines the global CHECK utility macros.
#pragma once

#include "base/logging.h"

#define TK_UNUSED(expr) (void)expr

#ifndef EVAL_MACRO_
#define EVAL_MACRO_(x) #x
#define EVAL_MACRO__(x) EVAL_MACRO_(x)
#endif

#ifdef BUGCHECK
#error Something else defined BUGCHECK
#endif

#ifdef DCHECK
#error Something else defined DCHECK
#endif

namespace base {
// Small utility function to trim down the source path of any function
consteval const char* const TrimSourcePath(
    const char* const str,
    const char* const lastslash = nullptr) {
  return *str ? TrimSourcePath(str + 1,
                               ((*str == '/' || *str == '\\')
                                    ? str + 1
                                    : (nullptr == lastslash ? str : lastslash)))
              : (nullptr == lastslash ? str : lastslash);
}

using AssertHandler = void (*)();

void SetAssertHandler(AssertHandler);
void InvokeAssertHandler();
}  // namespace base

// TODO: yeet legacy macro
#if defined(TK_DBG)
#define BREAK __debugbreak()
#else
#define BREAK InvokeAssertHandler()
#endif

// All checks follow this format:
// ProjectName (This is the solution on vs), FilePath, FileLine,

// A DCHECK is only present in non shipping builds and is ment for catching
// programmer misuse that needs to be fixed before release
//
// Q: How do DCHECKs relate to asserts?
// A: As asserts usually get stripped in shipping builds, a dcheck is the
//    logical choice to use in most places.
#ifndef SHIPPING
#define DCHECK(expression)                                                      \
  do {                                                                          \
    if (!(expression)) {                                                        \
      static constexpr const char* kShortFile{                                  \
          ::base::TrimSourcePath(__FILE__)};                                    \
      ::base::PrintLogMessage(                                               \
          ::base::LogLevel::kFatal,                                          \
          "DCheck failed at " PROJECT_NAME                                   \
          "!{}!" __FUNCTION__ "!" EVAL_MACRO__(   \__LINE__) " >>"           \
                                                             " " #expression \
                                                             " <<",          \
          kShortFile); \
      BREAK;                                                                    \
    }                                                                           \
  } while (0)

#else
#define DCHECK(x)
#endif

// BugChecks indicate a hard programmer error and
// are compiled into shipping builds aswell, as
// these need to be immedeatly fixed. In Shipping
// mode, these are forwarded to the user.
#define BUGCHECK(expression)                                          \
  do {                                                                \
    if (!(expression)) {                                              \
      static constexpr const char* kShortFile{                        \
          ::base::TrimSourcePath(__FILE__)};                          \
      ::base::PrintLogMessage(::base::LogLevel::kFatal,               \
                              "Bugcheck failed at " PROJECT_NAME      \
                              "!{}!" __FUNCTION__ "!" EVAL_MACRO__(   \
                                  __LINE__) " >> " #expression " <<", \
                              kShortFile);                            \
      BREAK;                                                          \
    }                                                                 \
  } while (0)

// Indicate an impossible to reach case, such as a
// default value in a switch being it with a named
// enum These are also being compiled in shipping
// builds.
#define IMPOSSIBLE                                                             \
  {                                                                            \
    static constexpr const char* kShortFile{::base::TrimSourcePath(__FILE__)}; \
    ::base::PrintLogMessage(::base::LogLevel::kFatal,                          \
                            "Impossible case at " PROJECT_NAME                 \
                            "!{}!" __FUNCTION__ "!" EVAL_MACRO__(__LINE__),    \
                            kShortFile);                                       \
    BREAK;                                                                     \
  }
