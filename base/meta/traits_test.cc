// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/arch.h>
#include <base/memory/cxx_lifetime.h>
#include <base/meta/traits.h>

namespace {

struct Plain {};
struct FromInt {
  FromInt(int) {}
};
struct Explicitly {
  explicit Explicitly(int) {}
};
struct Derived : Plain {};
enum PlainEnum { kPlainEnumValue };
enum class ScopedEnum : u8 { kValue };

// These mirror the traits option.h dispatches on, so a wrong answer here is
// the same wrong answer a parsed option would get.
static_assert(base::is_floating_point_v<float>);
static_assert(base::is_floating_point_v<double>);
static_assert(base::is_floating_point_v<long double>);
static_assert(base::is_floating_point_v<const double>);
static_assert(base::is_floating_point_v<volatile float>);
static_assert(base::is_floating_point_v<const volatile double>);

static_assert(!base::is_floating_point_v<int>);
static_assert(!base::is_floating_point_v<bool>);
static_assert(!base::is_floating_point_v<char>);
// A reference to a float is not itself a floating point type; option.h relies
// on this so `float&` falls through to the static_assert rather than parsing.
static_assert(!base::is_floating_point_v<float&>);
static_assert(!base::is_floating_point_v<float*>);
static_assert(!base::is_floating_point_v<Plain>);

static_assert(base::is_arithmetic_v<int>);
static_assert(base::is_arithmetic_v<double>);
static_assert(base::is_arithmetic_v<bool>);
static_assert(!base::is_arithmetic_v<Plain>);
static_assert(!base::is_arithmetic_v<int*>);

// is_convertible_v and ConvertibleTo route through a different builtin on
// MSVC than on GCC/Clang, so both spellings are worth pinning.
static_assert(base::is_convertible_v<int, long>);
static_assert(base::is_convertible_v<int, double>);
static_assert(base::is_convertible_v<int, FromInt>);
static_assert(base::is_convertible_v<Derived*, Plain*>);
static_assert(base::is_convertible_v<Plain, Plain>);

static_assert(!base::is_convertible_v<Plain, int>);
static_assert(!base::is_convertible_v<Plain*, Derived*>);
static_assert(!base::is_convertible_v<int, Plain>);
// Explicit constructors are not implicit conversions.
static_assert(!base::is_convertible_v<int, Explicitly>);

static_assert(base::ConvertibleTo<int, long>);
static_assert(!base::ConvertibleTo<Plain, int>);

// Traits that predate this file, retested here because nothing else covers
// them and the option parser leans on them.
static_assert(base::is_same_v<int, int>);
static_assert(!base::is_same_v<int, const int>);
static_assert(base::is_integral_v<bool>);
static_assert(base::is_integral_v<unsigned long long>);
static_assert(base::is_integral_v<const int>);
static_assert(!base::is_integral_v<float>);
static_assert(!base::is_integral_v<PlainEnum>);

static_assert(base::is_enum_v<PlainEnum>);
static_assert(base::is_enum_v<ScopedEnum>);
static_assert(!base::is_enum_v<int>);
static_assert(base::is_same_v<base::underlying_type_t<ScopedEnum>, u8>);

static_assert(base::is_signed_v<int>);
static_assert(!base::is_signed_v<unsigned int>);
static_assert(base::is_base_of_v<Plain, Derived>);
static_assert(!base::is_base_of_v<Derived, Plain>);

static_assert(base::is_same_v<base::decay_t<const int&>, int>);
static_assert(base::is_same_v<base::decay_t<int&&>, int>);

// The traits are compile-time only; this keeps the file a real test target
// rather than a header that happens to assert.
TEST(Traits, EvaluateAtCompileTime) {
  EXPECT_TRUE(base::is_floating_point_v<double>);
  EXPECT_FALSE(base::is_floating_point_v<int>);
  EXPECT_TRUE((base::is_convertible_v<int, double>));
  EXPECT_FALSE((base::is_convertible_v<int, Explicitly>));
}

}  // namespace

namespace {
struct Movable {
  Movable(Movable&&) noexcept {}
};
struct ThrowingMove {
  ThrowingMove(ThrowingMove&&) {}
};
struct NoDefault {
  explicit NoDefault(int) {}
};
struct NonTrivial {
  ~NonTrivial() {}
};

static_assert(base::is_same_v<base::remove_cvref_t<const int&>, int>);
static_assert(base::is_same_v<base::remove_cvref_t<volatile int&&>, int>);
static_assert(base::is_same_v<base::remove_cvref_t<const int*>, const int*>);
static_assert(base::is_array_v<int[3]> && base::is_array_v<int[]> && !base::is_array_v<int*>);
static_assert(base::extent_v<float[4]> == 4 && base::extent_v<int> == 0);
static_assert(base::is_same_v<base::remove_extent_t<float[4]>, float>);
static_assert(base::is_same_v<base::remove_extent_t<int[2][3]>, int[3]>);
static_assert(base::is_trivially_copyable_v<int> && !base::is_trivially_copyable_v<NonTrivial>);
static_assert(base::is_trivially_destructible_v<int> && !base::is_trivially_destructible_v<NonTrivial>);
static_assert(base::is_default_constructible_v<int> && !base::is_default_constructible_v<NoDefault>);
static_assert(base::is_move_constructible_v<Movable>);
static_assert(base::is_nothrow_move_constructible_v<Movable>);
static_assert(!base::is_nothrow_move_constructible_v<ThrowingMove>);

TEST(TraitsTest, ObjectTraitsCompile) { SUCCEED(); }
}  // namespace
