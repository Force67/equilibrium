// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Core definitions for compiler specifics.
#pragma once

#if defined(__x86_64__) || defined(_M_AMD64) || defined(_M_X64)
#define ARCH_X86_64
#elif defined(__i386) || defined(_M_IX86)
#define ARCH_X86
#endif

#if defined(__AVX2__)
#define HAS_SIMD_AVX2
#endif

#if defined(__AVX__) || defined(SIMD_AVX2)
#define HAS_SIMD_AVX
#endif

#if defined(__SSSE3__) || defined(SIMD_AVX)
#define HAS_SIMD_SSSE3
#endif

#if defined(__SSE3__) || defined(_M_AMD64) || defined(_M_X64) || \
    (defined(_M_IX86_FP) && (_M_IX86_FP == 2)) || defined(SIMD_SSSE3)
#define SIMD_SSE3
#endif

#if defined(__SSE2__) || defined(_M_AMD64) || defined(_M_X64) || \
    (defined(_M_IX86_FP) && (_M_IX86_FP == 2)) || defined(SIMD_SSE3)
#define SIMD_SSE2
#endif

#if defined(__SSE__) || (defined(_M_IX86_FP) && (_M_IX86_FP == 1)) || \
    defined(SIMD_SSE2)
#define SIMD_SSE
#endif

#if defined(__GNUC__) || defined(__clang__)
#define LIKELY(x) __builtin_expect(static_cast<bool>(x), 1)
#define UNLIKELY(x) __builtin_expect(static_cast<bool>(x), 0)
#else
#define LIKELY(x) static_cast<bool>(x)
#define UNLIKELY(x) static_cast<bool>(x)
#endif

#if defined(__GNUC__) || defined(__clang__)
#define STRONG_INLINE __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
#define STRONG_INLINE __pragma(warning(suppress : 4714)) inline __forceinline
#else
#define STRONG_INLINE inline
#endif

#if defined(__GNUC__) || defined(__clang__)
#define NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE
#endif

#if defined(__GNUC__) || defined(__clang__)
#define DEBUG_TRAP __asm__ volatile("int $0x03")
#elif defined(_MSC_VER)
#define DEBUG_TRAP __debugbreak()
#else
#define DEBUG_TRAP
#endif

#if defined(__cplusplus) && defined(__has_cpp_attribute)
#define HAS_ATTRIBUTE(attrib, value) (__has_cpp_attribute(attrib) >= value)
#else
#define HAS_ATTRIBUTE(attrib, value) (0)
#endif

#if HAS_ATTRIBUTE(likely, 201803L)
#define ATTR_LIKELY likely
#else
#define ATTR_LIKELY
#endif

#if HAS_ATTRIBUTE(unlikely, 201803L)
#define ATTR_UNLIKELY unlikely
#else
#define ATTR_UNLIKELY
#endif


// requires a rename later on.
#if defined(__GNUC__) || defined(__clang__)
// Clang & GCC
#define FOLLY_PUSH_WARNING _Pragma("GCC diagnostic push")
#define FOLLY_POP_WARNING _Pragma("GCC diagnostic pop")
#define FOLLY_GNU_DISABLE_WARNING_INTERNAL2(warningName) #warningName
#define FOLLY_GNU_DISABLE_WARNING(warningName) \
  _Pragma(                                     \
      FOLLY_GNU_DISABLE_WARNING_INTERNAL2(GCC diagnostic ignored warningName))
#ifdef __clang__
#define FOLLY_CLANG_DISABLE_WARNING(warningName) \
  FOLLY_GNU_DISABLE_WARNING(warningName)
#define FOLLY_GCC_DISABLE_WARNING(warningName)
#else
#define FOLLY_CLANG_DISABLE_WARNING(warningName)
#define FOLLY_GCC_DISABLE_WARNING(warningName) \
  FOLLY_GNU_DISABLE_WARNING(warningName)
#endif
#define FOLLY_MSVC_DISABLE_WARNING(warningNumber)
#elif defined(_MSC_VER)
#define FOLLY_PUSH_WARNING __pragma(warning(push))
#define FOLLY_POP_WARNING __pragma(warning(pop))
// Disable the GCC warnings.
#define FOLLY_GNU_DISABLE_WARNING(warningName)
#define FOLLY_GCC_DISABLE_WARNING(warningName)
#define FOLLY_CLANG_DISABLE_WARNING(warningName)
#define FOLLY_MSVC_DISABLE_WARNING(warningNumber) \
  __pragma(warning(disable : warningNumber))
#else
#define FOLLY_PUSH_WARNING
#define FOLLY_POP_WARNING
#define FOLLY_GNU_DISABLE_WARNING(warningName)
#define FOLLY_GCC_DISABLE_WARNING(warningName)
#define FOLLY_CLANG_DISABLE_WARNING(warningName)
#define FOLLY_MSVC_DISABLE_WARNING(warningNumber)
#endif

// for compatibility with chromium source code
#define WARN_UNUSED_RESULT

#define FALLTHROUGH [[fallthrough]]

// TODO: CACHELINE_SIZE 64

// TODO: Evaluate if this belongs here.
namespace base {
#ifdef _MSC_VER
// It's MSVC, so we just have to guess ... and allow an override
#ifdef FOLLY_ENDIAN_BE
constexpr auto kIsLittleEndian = false;
#else
constexpr auto kIsLittleEndian = true;
#endif
#else
constexpr auto kIsLittleEndian = __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__;
#endif
constexpr auto kIsBigEndian = !kIsLittleEndian;
}  // namespace base
