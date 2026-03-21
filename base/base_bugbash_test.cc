// Comprehensive bug-bash tests for the equilibrium base library.
// Covers: Vector, BasicBaseString, UnorderedMap, and related utilities.
// Focus: edge cases, memory safety, move/copy semantics, stress testing.

#include <gtest/gtest.h>

#include <base/arch.h>
#include <base/containers/vector.h>
#include <base/containers/unordered_map.h>
#include <base/strings/base_string.h>
#include <base/containers/span.h>
#include <base/containers/array.h>
#include <base/containers/linked_list.h>
#include <base/containers/ring_buffer.h>
#include <base/containers/deque.h>
#include <base/optional.h>
#include <base/expected.h>
#include <base/functional/function.h>
#include <base/functional/function_ref.h>
#include <base/memory/unique_pointer.h>
#include <base/hashing/fnv1a.h>
#include <base/math/math_helpers.h>
#include <base/strings/string_ref.h>

#include <string>
#include <cstring>

namespace {
using namespace base;

// ============================================================================
// Helper: tracks constructor/destructor calls for leak/double-free detection
// ============================================================================
struct LifetimeTracker {
  static int alive_count;
  static int ctor_count;
  static int dtor_count;
  static int copy_count;
  static int move_count;

  static void Reset() {
    alive_count = ctor_count = dtor_count = copy_count = move_count = 0;
  }

  int value_;

  LifetimeTracker() : value_(0) {
    ++alive_count;
    ++ctor_count;
  }
  explicit LifetimeTracker(int v) : value_(v) {
    ++alive_count;
    ++ctor_count;
  }
  LifetimeTracker(const LifetimeTracker& o) : value_(o.value_) {
    ++alive_count;
    ++copy_count;
  }
  LifetimeTracker(LifetimeTracker&& o) noexcept : value_(o.value_) {
    o.value_ = -1;
    ++alive_count;
    ++move_count;
  }
  LifetimeTracker& operator=(const LifetimeTracker& o) {
    value_ = o.value_;
    ++copy_count;
    return *this;
  }
  LifetimeTracker& operator=(LifetimeTracker&& o) noexcept {
    value_ = o.value_;
    o.value_ = -1;
    ++move_count;
    return *this;
  }
  ~LifetimeTracker() {
    --alive_count;
    ++dtor_count;
  }

  bool operator==(const LifetimeTracker& o) const { return value_ == o.value_; }
  bool operator!=(const LifetimeTracker& o) const { return value_ != o.value_; }
};

int LifetimeTracker::alive_count = 0;
int LifetimeTracker::ctor_count = 0;
int LifetimeTracker::dtor_count = 0;
int LifetimeTracker::copy_count = 0;
int LifetimeTracker::move_count = 0;

// ============================================================================
// VECTOR TESTS
// ============================================================================

class VectorBugBashTest : public ::testing::Test {
 protected:
  void SetUp() override { LifetimeTracker::Reset(); }
  void TearDown() override { EXPECT_EQ(LifetimeTracker::alive_count, 0); }
};

TEST_F(VectorBugBashTest, DefaultConstructorIsEmpty) {
  base::Vector<i32> v;
  EXPECT_TRUE(v.empty());
  EXPECT_EQ(v.size(), 0u);
  EXPECT_EQ(v.capacity(), 0u);
  EXPECT_EQ(v.begin(), v.end());
  EXPECT_EQ(v.data(), nullptr);
}

TEST_F(VectorBugBashTest, SizeConstructor) {
  base::Vector<i32> v(5);
  EXPECT_EQ(v.size(), 5u);
  EXPECT_EQ(v.capacity(), 5u);
  for (mem_size i = 0; i < 5; ++i) {
    EXPECT_EQ(v[i], 0);
  }
}

TEST_F(VectorBugBashTest, SizeConstructorZero) {
  base::Vector<i32> v(0);
  EXPECT_TRUE(v.empty());
  EXPECT_EQ(v.data(), nullptr);
}

TEST_F(VectorBugBashTest, SizeConstructorComplex) {
  {
    base::Vector<LifetimeTracker> v(3);
    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(LifetimeTracker::alive_count, 3);
    for (mem_size i = 0; i < 3; ++i) {
      EXPECT_EQ(v[i].value_, 0);
    }
  }
  EXPECT_EQ(LifetimeTracker::alive_count, 0);
}

TEST_F(VectorBugBashTest, InitializerListConstruct) {
  base::Vector<i32> v = {10, 20, 30, 40, 50};
  EXPECT_EQ(v.size(), 5u);
  EXPECT_EQ(v[0], 10);
  EXPECT_EQ(v[4], 50);
}

TEST_F(VectorBugBashTest, CopyConstructor) {
  base::Vector<i32> v1 = {1, 2, 3};
  base::Vector<i32> v2(v1);
  EXPECT_EQ(v2.size(), v1.size());
  EXPECT_EQ(v2[0], 1);
  EXPECT_EQ(v2[2], 3);
  // Ensure deep copy - modifying v2 doesn't affect v1
  v2[0] = 99;
  EXPECT_EQ(v1[0], 1);
}

TEST_F(VectorBugBashTest, CopyConstructorComplex) {
  {
    base::Vector<LifetimeTracker> v1;
    v1.push_back(LifetimeTracker(10));
    v1.push_back(LifetimeTracker(20));

    base::Vector<LifetimeTracker> v2(v1);
    EXPECT_EQ(v2.size(), 2u);
    EXPECT_EQ(v2[0].value_, 10);
    EXPECT_EQ(v2[1].value_, 20);
  }
  // All elements destroyed
  EXPECT_EQ(LifetimeTracker::alive_count, 0);
}

TEST_F(VectorBugBashTest, CopyConstructorEmpty) {
  base::Vector<i32> v1;
  base::Vector<i32> v2(v1);
  EXPECT_TRUE(v2.empty());
}

TEST_F(VectorBugBashTest, MoveConstructor) {
  base::Vector<i32> v1 = {1, 2, 3};
  i32* old_data = v1.data();
  base::Vector<i32> v2(base::move(v1));
  EXPECT_EQ(v2.size(), 3u);
  EXPECT_EQ(v2.data(), old_data);
  EXPECT_TRUE(v1.empty());
  EXPECT_EQ(v1.data(), nullptr);
}

TEST_F(VectorBugBashTest, MoveConstructorComplex) {
  {
    base::Vector<LifetimeTracker> v1;
    v1.push_back(LifetimeTracker(42));
    int alive_before = LifetimeTracker::alive_count;

    base::Vector<LifetimeTracker> v2(base::move(v1));
    // Move constructor steals pointers, no element-level moves
    EXPECT_EQ(LifetimeTracker::alive_count, alive_before);
    EXPECT_EQ(v2.size(), 1u);
    EXPECT_EQ(v2[0].value_, 42);
  }
  EXPECT_EQ(LifetimeTracker::alive_count, 0);
}

TEST_F(VectorBugBashTest, CopyAssignment) {
  base::Vector<i32> v1 = {1, 2, 3};
  base::Vector<i32> v2 = {4, 5};
  v2 = v1;
  EXPECT_EQ(v2.size(), 3u);
  EXPECT_EQ(v2[0], 1);
}

TEST_F(VectorBugBashTest, CopyAssignmentSelf) {
  base::Vector<i32> v = {1, 2, 3};
  v = v;
  EXPECT_EQ(v.size(), 3u);
  EXPECT_EQ(v[0], 1);
}

TEST_F(VectorBugBashTest, MoveAssignment) {
  base::Vector<i32> v1 = {1, 2, 3};
  base::Vector<i32> v2 = {4, 5};
  v2 = base::move(v1);
  EXPECT_EQ(v2.size(), 3u);
  EXPECT_EQ(v2[0], 1);
  EXPECT_TRUE(v1.empty());
}

TEST_F(VectorBugBashTest, PushBackGrowth) {
  base::Vector<i32> v;
  for (i32 i = 0; i < 1000; ++i) {
    v.push_back(i);
    EXPECT_EQ(v.back(), i);
    EXPECT_EQ(v.size(), static_cast<mem_size>(i + 1));
  }
  for (i32 i = 0; i < 1000; ++i) {
    EXPECT_EQ(v[i], i);
  }
}

TEST_F(VectorBugBashTest, PushBackRvalue) {
  {
    base::Vector<LifetimeTracker> v;
    v.push_back(LifetimeTracker(42));
    EXPECT_EQ(v.back().value_, 42);
  }
  EXPECT_EQ(LifetimeTracker::alive_count, 0);
}

TEST_F(VectorBugBashTest, EmplaceBack) {
  {
    base::Vector<LifetimeTracker> v;
    auto& ref = v.emplace_back(42);
    EXPECT_EQ(ref.value_, 42);
    EXPECT_EQ(v.size(), 1u);
  }
  EXPECT_EQ(LifetimeTracker::alive_count, 0);
}

TEST_F(VectorBugBashTest, PopBack) {
  {
    base::Vector<LifetimeTracker> v;
    v.push_back(LifetimeTracker(1));
    v.push_back(LifetimeTracker(2));
    v.push_back(LifetimeTracker(3));
    int alive_before = LifetimeTracker::alive_count;

    v.pop_back();
    EXPECT_EQ(v.size(), 2u);
    EXPECT_EQ(LifetimeTracker::alive_count, alive_before - 1);
    EXPECT_EQ(v.back().value_, 2);
  }
  EXPECT_EQ(LifetimeTracker::alive_count, 0);
}

TEST_F(VectorBugBashTest, EraseFirstElement) {
  base::Vector<i32> v = {10, 20, 30, 40, 50};
  v.erase(static_cast<mem_size>(0));
  EXPECT_EQ(v.size(), 4u);
  EXPECT_EQ(v[0], 20);
  EXPECT_EQ(v[3], 50);
}

TEST_F(VectorBugBashTest, EraseLastElement) {
  base::Vector<i32> v = {10, 20, 30};
  v.erase(static_cast<mem_size>(2));
  EXPECT_EQ(v.size(), 2u);
  EXPECT_EQ(v[0], 10);
  EXPECT_EQ(v[1], 20);
}

TEST_F(VectorBugBashTest, EraseMiddleElement) {
  base::Vector<i32> v = {10, 20, 30, 40, 50};
  v.erase(static_cast<mem_size>(2));
  EXPECT_EQ(v.size(), 4u);
  EXPECT_EQ(v[0], 10);
  EXPECT_EQ(v[1], 20);
  EXPECT_EQ(v[2], 40);
  EXPECT_EQ(v[3], 50);
}

TEST_F(VectorBugBashTest, EraseByPointer) {
  base::Vector<i32> v = {10, 20, 30};
  v.erase(v.begin() + 1);
  EXPECT_EQ(v.size(), 2u);
  EXPECT_EQ(v[0], 10);
  EXPECT_EQ(v[1], 30);
}

TEST_F(VectorBugBashTest, EraseRange) {
  base::Vector<i32> v = {10, 20, 30, 40, 50};
  v.erase(v.begin() + 1, v.begin() + 4);
  EXPECT_EQ(v.size(), 2u);
  EXPECT_EQ(v[0], 10);
  EXPECT_EQ(v[1], 50);
}

TEST_F(VectorBugBashTest, EraseRangeComplex) {
  {
    base::Vector<LifetimeTracker> v;
    for (int i = 0; i < 5; ++i) v.push_back(LifetimeTracker(i));

    v.erase(v.begin() + 1, v.begin() + 4);
    EXPECT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0].value_, 0);
    EXPECT_EQ(v[1].value_, 4);
  }
  EXPECT_EQ(LifetimeTracker::alive_count, 0);
}

TEST_F(VectorBugBashTest, EraseEmptyRange) {
  base::Vector<i32> v = {10, 20, 30};
  auto* result = v.erase(v.begin() + 1, v.begin() + 1);
  EXPECT_EQ(v.size(), 3u);
  EXPECT_EQ(result, v.begin() + 1);
}

TEST_F(VectorBugBashTest, EraseOutOfBounds) {
  base::Vector<i32> v = {10, 20, 30};
  EXPECT_FALSE(v.erase(static_cast<mem_size>(5)));
  EXPECT_EQ(v.size(), 3u);
}

TEST_F(VectorBugBashTest, EraseComplexLifetime) {
  {
    base::Vector<LifetimeTracker> v;
    for (int i = 0; i < 5; ++i) v.push_back(LifetimeTracker(i));

    v.erase(static_cast<mem_size>(2));
    EXPECT_EQ(v.size(), 4u);
    EXPECT_EQ(v[0].value_, 0);
    EXPECT_EQ(v[1].value_, 1);
    EXPECT_EQ(v[2].value_, 3);
    EXPECT_EQ(v[3].value_, 4);
  }
  EXPECT_EQ(LifetimeTracker::alive_count, 0);
}

TEST_F(VectorBugBashTest, InsertAtBegin) {
  base::Vector<i32> v = {20, 30, 40};
  v.insert(v.begin(), 10);
  EXPECT_EQ(v.size(), 4u);
  EXPECT_EQ(v[0], 10);
  EXPECT_EQ(v[1], 20);
  EXPECT_EQ(v[3], 40);
}

TEST_F(VectorBugBashTest, InsertAtEnd) {
  base::Vector<i32> v = {10, 20, 30};
  v.insert(v.end(), 40);
  EXPECT_EQ(v.size(), 4u);
  EXPECT_EQ(v[3], 40);
}

TEST_F(VectorBugBashTest, InsertMiddle) {
  base::Vector<i32> v = {10, 30, 40};
  v.insert(v.begin() + 1, 20);
  EXPECT_EQ(v.size(), 4u);
  EXPECT_EQ(v[0], 10);
  EXPECT_EQ(v[1], 20);
  EXPECT_EQ(v[2], 30);
  EXPECT_EQ(v[3], 40);
}

TEST_F(VectorBugBashTest, InsertMultipleCopies) {
  base::Vector<i32> v = {1, 5};
  v.insert(v.begin() + 1, static_cast<mem_size>(3), 99);
  EXPECT_EQ(v.size(), 5u);
  EXPECT_EQ(v[0], 1);
  EXPECT_EQ(v[1], 99);
  EXPECT_EQ(v[2], 99);
  EXPECT_EQ(v[3], 99);
  EXPECT_EQ(v[4], 5);
}

TEST_F(VectorBugBashTest, InsertTriggerRealloc) {
  base::Vector<i32> v;
  v.reserve(2);
  v.push_back(1);
  v.push_back(2);
  EXPECT_EQ(v.capacity(), 2u);
  v.insert(v.begin(), 0);
  EXPECT_EQ(v.size(), 3u);
  EXPECT_EQ(v[0], 0);
  EXPECT_EQ(v[1], 1);
  EXPECT_EQ(v[2], 2);
}

TEST_F(VectorBugBashTest, InsertRangeFromOtherContainer) {
  base::Vector<i32> v = {1, 5};
  base::Vector<i32> to_insert = {2, 3, 4};
  v.insert(v.begin() + 1, to_insert.begin(), to_insert.end());
  EXPECT_EQ(v.size(), 5u);
  for (i32 i = 0; i < 5; ++i) {
    EXPECT_EQ(v[i], i + 1);
  }
}

TEST_F(VectorBugBashTest, InsertEmptyRange) {
  base::Vector<i32> v = {1, 2, 3};
  base::Vector<i32> empty;
  v.insert(v.begin(), empty.begin(), empty.end());
  EXPECT_EQ(v.size(), 3u);
}

TEST_F(VectorBugBashTest, InsertZeroCopies) {
  base::Vector<i32> v = {1, 2, 3};
  v.insert(v.begin(), static_cast<mem_size>(0), 99);
  EXPECT_EQ(v.size(), 3u);
}

TEST_F(VectorBugBashTest, ResizeGrow) {
  base::Vector<i32> v = {1, 2, 3};
  v.resize(6);
  EXPECT_EQ(v.size(), 6u);
  EXPECT_EQ(v[0], 1);
  EXPECT_EQ(v[2], 3);
  EXPECT_EQ(v[3], 0);
  EXPECT_EQ(v[5], 0);
}

TEST_F(VectorBugBashTest, ResizeShrink) {
  base::Vector<i32> v = {1, 2, 3, 4, 5};
  v.resize(2);
  EXPECT_EQ(v.size(), 2u);
  EXPECT_EQ(v[0], 1);
  EXPECT_EQ(v[1], 2);
}

TEST_F(VectorBugBashTest, ResizeShrinkComplex) {
  {
    base::Vector<LifetimeTracker> v;
    for (int i = 0; i < 5; ++i) v.push_back(LifetimeTracker(i));
    int alive_before = LifetimeTracker::alive_count;

    v.resize(2);
    EXPECT_EQ(v.size(), 2u);
    EXPECT_EQ(LifetimeTracker::alive_count, alive_before - 3);
    EXPECT_EQ(v[0].value_, 0);
    EXPECT_EQ(v[1].value_, 1);
  }
  EXPECT_EQ(LifetimeTracker::alive_count, 0);
}

TEST_F(VectorBugBashTest, ResizeToZero) {
  base::Vector<i32> v = {1, 2, 3};
  v.resize(0);
  EXPECT_TRUE(v.empty());
}

TEST_F(VectorBugBashTest, ResizeWithValue) {
  base::Vector<i32> v = {1, 2};
  v.resize(5, 42);
  EXPECT_EQ(v.size(), 5u);
  EXPECT_EQ(v[0], 1);
  EXPECT_EQ(v[1], 2);
  EXPECT_EQ(v[2], 42);
  EXPECT_EQ(v[4], 42);
}

TEST_F(VectorBugBashTest, ResizeSameSize) {
  base::Vector<i32> v = {1, 2, 3};
  v.resize(3);
  EXPECT_EQ(v.size(), 3u);
}

TEST_F(VectorBugBashTest, ReserveGrow) {
  base::Vector<i32> v;
  v.reserve(100);
  EXPECT_GE(v.capacity(), 100u);
  EXPECT_EQ(v.size(), 0u);
}

TEST_F(VectorBugBashTest, ReserveZero) {
  base::Vector<i32> v;
  v.reserve(0);
  EXPECT_EQ(v.capacity(), 0u);
}

TEST_F(VectorBugBashTest, ReserveSmallerThanCurrent) {
  base::Vector<i32> v;
  v.reserve(100);
  auto old_cap = v.capacity();
  v.reserve(50);
  EXPECT_EQ(v.capacity(), old_cap);  // should not shrink
}

TEST_F(VectorBugBashTest, ShrinkToFit) {
  base::Vector<i32> v;
  v.reserve(100);
  for (i32 i = 0; i < 10; ++i) v.push_back(i);
  EXPECT_GT(v.capacity(), v.size());

  v.shrink_to_fit();
  EXPECT_EQ(v.capacity(), v.size());
  for (i32 i = 0; i < 10; ++i) EXPECT_EQ(v[i], i);
}

TEST_F(VectorBugBashTest, ShrinkToFitEmpty) {
  base::Vector<i32> v;
  v.reserve(100);
  v.shrink_to_fit();
  EXPECT_EQ(v.capacity(), 0u);
  EXPECT_TRUE(v.empty());
}

TEST_F(VectorBugBashTest, ShrinkToFitComplex) {
  {
    base::Vector<LifetimeTracker> v;
    for (int i = 0; i < 10; ++i) v.push_back(LifetimeTracker(i));
    v.resize(3);

    v.shrink_to_fit();
    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v.capacity(), 3u);
    EXPECT_EQ(v[0].value_, 0);
    EXPECT_EQ(v[1].value_, 1);
    EXPECT_EQ(v[2].value_, 2);
  }
  EXPECT_EQ(LifetimeTracker::alive_count, 0);
}

TEST_F(VectorBugBashTest, ClearKeepsCapacity) {
  base::Vector<i32> v = {1, 2, 3, 4, 5};
  auto old_cap = v.capacity();
  v.clear();
  EXPECT_TRUE(v.empty());
  EXPECT_EQ(v.capacity(), old_cap);
}

TEST_F(VectorBugBashTest, ClearComplex) {
  {
    base::Vector<LifetimeTracker> v;
    for (int i = 0; i < 5; ++i) v.push_back(LifetimeTracker(i));

    v.clear();
    EXPECT_TRUE(v.empty());
  }
  EXPECT_EQ(LifetimeTracker::alive_count, 0);
}

TEST_F(VectorBugBashTest, ResetFreesMemory) {
  base::Vector<i32> v = {1, 2, 3};
  v.reset();
  EXPECT_TRUE(v.empty());
  EXPECT_EQ(v.capacity(), 0u);
  EXPECT_EQ(v.data(), nullptr);
}

TEST_F(VectorBugBashTest, AssignValueCount) {
  base::Vector<i32> v = {1, 2, 3};
  v.assign(static_cast<mem_size>(5), 42);
  EXPECT_EQ(v.size(), 5u);
  for (mem_size i = 0; i < 5; ++i) EXPECT_EQ(v[i], 42);
}

TEST_F(VectorBugBashTest, AssignRange) {
  base::Vector<i32> src = {10, 20, 30};
  base::Vector<i32> v = {1, 2};
  v.assign(src.begin(), src.end());
  EXPECT_EQ(v.size(), 3u);
  EXPECT_EQ(v[0], 10);
  EXPECT_EQ(v[2], 30);
}

TEST_F(VectorBugBashTest, AtBoundsCheck) {
  base::Vector<i32> v = {10, 20, 30};
  EXPECT_NE(v.at(0), nullptr);
  EXPECT_EQ(*v.at(0), 10);
  EXPECT_NE(v.at(2), nullptr);
  EXPECT_EQ(*v.at(2), 30);
  EXPECT_EQ(v.at(3), nullptr);
  EXPECT_EQ(v.at(100), nullptr);
}

TEST_F(VectorBugBashTest, FindAndContains) {
  base::Vector<i32> v = {10, 20, 30, 40, 50};
  EXPECT_NE(v.find(30), nullptr);
  EXPECT_EQ(*v.find(30), 30);
  EXPECT_EQ(v.find(99), nullptr);
  EXPECT_TRUE(v.Contains(30));
  EXPECT_FALSE(v.Contains(99));
}

TEST_F(VectorBugBashTest, FindIfPredicate) {
  base::Vector<i32> v = {1, 2, 3, 4, 5};
  auto* result = v.FindIf([](i32 x) { return x > 3; });
  ASSERT_NE(result, nullptr);
  EXPECT_EQ(*result, 4);

  auto* not_found = v.FindIf([](i32 x) { return x > 10; });
  EXPECT_EQ(not_found, nullptr);
}

TEST_F(VectorBugBashTest, ForEach) {
  base::Vector<i32> v = {1, 2, 3};
  i32 sum = 0;
  v.ForEach([&sum](i32 x) { sum += x; });
  EXPECT_EQ(sum, 6);
}

TEST_F(VectorBugBashTest, EqualityOperator) {
  base::Vector<i32> v1 = {1, 2, 3};
  base::Vector<i32> v2 = {1, 2, 3};
  base::Vector<i32> v3 = {1, 2, 4};
  base::Vector<i32> v4 = {1, 2};

  EXPECT_TRUE(v1 == v2);
  EXPECT_FALSE(v1 == v3);
  EXPECT_FALSE(v1 == v4);
  EXPECT_TRUE(v1 != v3);
}

TEST_F(VectorBugBashTest, IteratorLoop) {
  base::Vector<i32> v = {10, 20, 30};
  i32 expected = 10;
  for (auto val : v) {
    EXPECT_EQ(val, expected);
    expected += 10;
  }
}

TEST_F(VectorBugBashTest, StressTestPushPopCycle) {
  {
    base::Vector<LifetimeTracker> v;
    for (int cycle = 0; cycle < 10; ++cycle) {
      for (int i = 0; i < 100; ++i) {
        v.push_back(LifetimeTracker(i));
      }
      while (!v.empty()) {
        v.pop_back();
      }
    }
  }
  EXPECT_EQ(LifetimeTracker::alive_count, 0);
}

TEST_F(VectorBugBashTest, LargeVector) {
  base::Vector<i32> v;
  constexpr int N = 100000;
  for (i32 i = 0; i < N; ++i) v.push_back(i);
  EXPECT_EQ(v.size(), static_cast<mem_size>(N));
  for (i32 i = 0; i < N; ++i) EXPECT_EQ(v[i], i);
}

TEST_F(VectorBugBashTest, InsertComplexLifetime) {
  {
    base::Vector<LifetimeTracker> v;
    v.push_back(LifetimeTracker(1));
    v.push_back(LifetimeTracker(3));

    LifetimeTracker to_insert(2);
    v.insert(v.begin() + 1, to_insert);
    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0].value_, 1);
    EXPECT_EQ(v[1].value_, 2);
    EXPECT_EQ(v[2].value_, 3);
  }
  EXPECT_EQ(LifetimeTracker::alive_count, 0);
}

TEST_F(VectorBugBashTest, InsertAtEndComplex) {
  {
    base::Vector<LifetimeTracker> v;
    v.push_back(LifetimeTracker(1));
    v.push_back(LifetimeTracker(2));

    LifetimeTracker to_insert(3);
    v.insert(v.end(), to_insert);
    EXPECT_EQ(v.size(), 3u);
    EXPECT_EQ(v[2].value_, 3);
  }
  EXPECT_EQ(LifetimeTracker::alive_count, 0);
}

TEST_F(VectorBugBashTest, ReservePolicyForPushback) {
  base::Vector<i32> v(10, VectorReservePolicy::kForPushback);
  EXPECT_EQ(v.size(), 0u);
  EXPECT_EQ(v.capacity(), 10u);
  for (i32 i = 0; i < 10; ++i) v.push_back(i);
  EXPECT_EQ(v.size(), 10u);
}

TEST_F(VectorBugBashTest, ReservePolicyForData) {
  base::Vector<i32> v(10, VectorReservePolicy::kForData);
  EXPECT_EQ(v.size(), 10u);
  EXPECT_EQ(v.capacity(), 10u);
  for (mem_size i = 0; i < 10; ++i) EXPECT_EQ(v[i], 0);
}

TEST_F(VectorBugBashTest, EraseAllOneByOne) {
  {
    base::Vector<LifetimeTracker> v;
    for (int i = 0; i < 10; ++i) v.push_back(LifetimeTracker(i));

    while (!v.empty()) {
      v.erase(static_cast<mem_size>(0));
    }
  }
  EXPECT_EQ(LifetimeTracker::alive_count, 0);
}

// ============================================================================
// STRING TESTS
// ============================================================================

using String = base::BasicBaseString<char>;

class StringBugBashTest : public ::testing::Test {};

TEST_F(StringBugBashTest, DefaultConstructor) {
  String s;
  EXPECT_TRUE(s.empty());
  EXPECT_EQ(s.size(), 0u);
  EXPECT_STREQ(s.c_str(), "");
}

TEST_F(StringBugBashTest, ConstructFromCString) {
  String s("hello");
  EXPECT_EQ(s.size(), 5u);
  EXPECT_STREQ(s.c_str(), "hello");
}

TEST_F(StringBugBashTest, ConstructFromNullptr) {
  String s(static_cast<const char*>(nullptr));
  EXPECT_TRUE(s.empty());
  EXPECT_STREQ(s.c_str(), "");
}

TEST_F(StringBugBashTest, ConstructFromCStringWithLength) {
  String s("hello world", 5);
  EXPECT_EQ(s.size(), 5u);
  EXPECT_STREQ(s.c_str(), "hello");
}

TEST_F(StringBugBashTest, ConstructFromRange) {
  const char* str = "hello world";
  String s(str, str + 5);
  EXPECT_EQ(s.size(), 5u);
  EXPECT_STREQ(s.c_str(), "hello");
}

TEST_F(StringBugBashTest, FillConstructor) {
  String s(10, 'x');
  EXPECT_EQ(s.size(), 10u);
  EXPECT_STREQ(s.c_str(), "xxxxxxxxxx");
}

TEST_F(StringBugBashTest, FillConstructorZero) {
  String s(0, 'x');
  EXPECT_TRUE(s.empty());
}

TEST_F(StringBugBashTest, SSOSmallString) {
  // kSmallCapacity = (24-1)/1 - 1 = 22 for char
  String s("tiny");
  EXPECT_EQ(s.size(), 4u);
  EXPECT_LE(s.size(), 22u);  // fits in SSO
  EXPECT_STREQ(s.c_str(), "tiny");
}

TEST_F(StringBugBashTest, SSOExactFit) {
  // Exactly kSmallCapacity (22 chars)
  String s("1234567890123456789012");
  EXPECT_EQ(s.size(), 22u);
  EXPECT_STREQ(s.c_str(), "1234567890123456789012");
}

TEST_F(StringBugBashTest, SSOTransitionToLarge) {
  // 23 chars - exceeds SSO
  String s("12345678901234567890123");
  EXPECT_EQ(s.size(), 23u);
  EXPECT_STREQ(s.c_str(), "12345678901234567890123");
}

TEST_F(StringBugBashTest, CopyConstructorSmall) {
  String s1("hello");
  String s2(s1);
  EXPECT_EQ(s2.size(), s1.size());
  EXPECT_STREQ(s2.c_str(), "hello");
  // Ensure deep copy
  EXPECT_NE(s1.data(), s2.data());
}

TEST_F(StringBugBashTest, CopyConstructorLarge) {
  std::string large(100, 'a');
  String s1(large.c_str());
  String s2(s1);
  EXPECT_EQ(s2.size(), s1.size());
  EXPECT_STREQ(s2.c_str(), large.c_str());
}

TEST_F(StringBugBashTest, CopyConstructorEmpty) {
  String s1;
  String s2(s1);
  EXPECT_TRUE(s2.empty());
  EXPECT_STREQ(s2.c_str(), "");
}

TEST_F(StringBugBashTest, MoveConstructor) {
  String s1("hello world this is a long string beyond SSO");
  const char* old_data = s1.c_str();
  String s2(base::move(s1));
  EXPECT_STREQ(s2.c_str(), "hello world this is a long string beyond SSO");
  EXPECT_TRUE(s1.empty());
  // Should have stolen the pointer
  EXPECT_EQ(s2.c_str(), old_data);
}

TEST_F(StringBugBashTest, MoveConstructorSmall) {
  String s1("tiny");
  String s2(base::move(s1));
  EXPECT_STREQ(s2.c_str(), "tiny");
  EXPECT_TRUE(s1.empty());
}

TEST_F(StringBugBashTest, CopyAssignment) {
  String s1("hello");
  String s2("world");
  s2 = s1;
  EXPECT_STREQ(s2.c_str(), "hello");
}

TEST_F(StringBugBashTest, CopyAssignmentSelf) {
  String s("hello");
  s = s;
  EXPECT_STREQ(s.c_str(), "hello");
}

TEST_F(StringBugBashTest, MoveAssignment) {
  String s1("hello world this is a very long string");
  String s2("short");
  s2 = base::move(s1);
  EXPECT_STREQ(s2.c_str(), "hello world this is a very long string");
  EXPECT_TRUE(s1.empty());
}

TEST_F(StringBugBashTest, AssignCString) {
  String s;
  s.assign("hello");
  EXPECT_STREQ(s.c_str(), "hello");
  EXPECT_EQ(s.size(), 5u);
}

TEST_F(StringBugBashTest, AssignWithLength) {
  String s;
  s.assign("hello world", 5);
  EXPECT_STREQ(s.c_str(), "hello");
}

TEST_F(StringBugBashTest, AssignNull) {
  String s("hello");
  s.assign(static_cast<const char*>(nullptr), static_cast<String::size_type>(0));
  EXPECT_TRUE(s.empty());
}

TEST_F(StringBugBashTest, AssignEmptyString) {
  String s("hello");
  s.assign(static_cast<const char*>(""), static_cast<String::size_type>(0));
  EXPECT_TRUE(s.empty());
}

TEST_F(StringBugBashTest, AssignSmallToLarge) {
  String s("tiny");
  std::string large(100, 'b');
  s.assign(large.c_str(), large.size());
  EXPECT_EQ(s.size(), 100u);
  EXPECT_STREQ(s.c_str(), large.c_str());
}

TEST_F(StringBugBashTest, AssignLargeToSmall) {
  std::string large(100, 'b');
  String s(large.c_str());
  s.assign("tiny");
  EXPECT_EQ(s.size(), 4u);
  EXPECT_STREQ(s.c_str(), "tiny");
}

TEST_F(StringBugBashTest, AppendBasic) {
  String s("hello");
  s.append(" world", 6);
  EXPECT_STREQ(s.c_str(), "hello world");
}

TEST_F(StringBugBashTest, AppendCString) {
  String s("hello");
  s.append(" world");
  EXPECT_STREQ(s.c_str(), "hello world");
}

TEST_F(StringBugBashTest, AppendEmpty) {
  String s("hello");
  s.append("", 0);
  EXPECT_EQ(s.size(), 5u);
  EXPECT_STREQ(s.c_str(), "hello");
}

TEST_F(StringBugBashTest, AppendTriggerRealloc) {
  String s("short");
  std::string to_append(200, 'x');
  s.append(to_append.c_str(), to_append.size());
  EXPECT_EQ(s.size(), 5 + 200);
  EXPECT_EQ(memcmp(s.c_str(), "short", 5), 0);
  EXPECT_EQ(s.c_str()[5], 'x');
}

TEST_F(StringBugBashTest, PushBackChar) {
  String s;
  for (int i = 0; i < 100; ++i) {
    s.push_back('a');
  }
  EXPECT_EQ(s.size(), 100u);
  for (mem_size i = 0; i < 100; ++i) {
    EXPECT_EQ(s[i], 'a');
  }
}

TEST_F(StringBugBashTest, PushBackSSOTransition) {
  String s;
  for (int i = 0; i < 24; ++i) {
    s.push_back('a' + (i % 26));
  }
  EXPECT_EQ(s.size(), 24u);
  EXPECT_EQ(s[0], 'a');
}

TEST_F(StringBugBashTest, OperatorPlusEquals) {
  String s("hello");
  s += " ";
  s += "world";
  EXPECT_STREQ(s.c_str(), "hello world");
}

TEST_F(StringBugBashTest, OperatorPlusEqualsChar) {
  String s("ab");
  s += 'c';
  EXPECT_STREQ(s.c_str(), "abc");
}

TEST_F(StringBugBashTest, InsertChars) {
  String s("hello world");
  s.insert(5, 3, '!');
  EXPECT_STREQ(s.c_str(), "hello!!! world");
}

TEST_F(StringBugBashTest, InsertAtStart) {
  String s("world");
  s.insert(0, 5, '_');
  EXPECT_EQ(s.size(), 10u);
  EXPECT_STREQ(s.c_str(), "_____world");
}

TEST_F(StringBugBashTest, EraseSubstring) {
  String s("hello world");
  s.erase(5, 6);
  EXPECT_STREQ(s.c_str(), "hello");
}

TEST_F(StringBugBashTest, EraseFromStart) {
  String s("hello world");
  s.erase(0, 6);
  EXPECT_STREQ(s.c_str(), "world");
}

TEST_F(StringBugBashTest, EraseNpos) {
  String s("hello world");
  s.erase(5);
  EXPECT_STREQ(s.c_str(), "hello");
}

TEST_F(StringBugBashTest, EraseSingleChar) {
  String s("hello");
  s.erase(s.c_str() + 2);  // erase 'l' at position 2
  EXPECT_EQ(s.size(), 4u);
  EXPECT_STREQ(s.c_str(), "helo");
}

TEST_F(StringBugBashTest, EraseRange) {
  String s("hello world");
  s.erase(s.begin() + 5, s.begin() + 6);
  EXPECT_STREQ(s.c_str(), "helloworld");
}

TEST_F(StringBugBashTest, RemoveSuffix) {
  String s("hello world");
  s.remove_suffix(6);
  EXPECT_STREQ(s.c_str(), "hello");
}

TEST_F(StringBugBashTest, Substr) {
  String s("hello world");
  String sub = s.substr(6, 5);
  EXPECT_STREQ(sub.c_str(), "world");
}

TEST_F(StringBugBashTest, SubstrNpos) {
  String s("hello world");
  String sub = s.substr(6);
  EXPECT_STREQ(sub.c_str(), "world");
}

TEST_F(StringBugBashTest, SubstrFromStart) {
  String s("hello");
  String sub = s.substr(0, 3);
  EXPECT_STREQ(sub.c_str(), "hel");
}

TEST_F(StringBugBashTest, FindChar) {
  String s("hello world");
  EXPECT_EQ(s.find('w'), 6u);
  EXPECT_EQ(s.find('z'), String::npos);
  EXPECT_EQ(s.find('h'), 0u);
}

TEST_F(StringBugBashTest, FindCharWithPos) {
  String s("hello world");
  EXPECT_EQ(s.find('l', 4), 9u);
  EXPECT_EQ(s.find('l', 0), 2u);
}

TEST_F(StringBugBashTest, FindSubstring) {
  String s("hello world");
  EXPECT_EQ(s.find("world"), 6u);
  EXPECT_EQ(s.find("xyz"), String::npos);
  EXPECT_EQ(s.find(""), 0u);
}

TEST_F(StringBugBashTest, FindLastOf) {
  String s("hello world");
  EXPECT_EQ(s.find_last_of('l'), 9u);
  EXPECT_EQ(s.find_last_of('h'), 0u);
  EXPECT_EQ(s.find_last_of('z'), String::npos);
}

TEST_F(StringBugBashTest, CompareBasic) {
  String s1("abc");
  String s2("abc");
  String s3("abd");
  String s4("ab");

  EXPECT_EQ(s1.compare(s2), 0);
  EXPECT_LT(s1.compare(s3), 0);
  EXPECT_GT(s3.compare(s1), 0);
  EXPECT_GT(s1.compare(s4), 0);
  EXPECT_LT(s4.compare(s1), 0);
}

TEST_F(StringBugBashTest, EqualityOperators) {
  String s1("hello");
  String s2("hello");
  String s3("world");

  EXPECT_TRUE(s1 == s2);
  EXPECT_FALSE(s1 == s3);
  EXPECT_TRUE(s1 != s3);
  EXPECT_TRUE(s1 == "hello");
  EXPECT_FALSE(s1 == "world");
}

TEST_F(StringBugBashTest, ComparisonOperators) {
  String s1("abc");
  String s2("abd");

  EXPECT_TRUE(s1 < s2);
  EXPECT_TRUE(s1 <= s2);
  EXPECT_TRUE(s2 > s1);
  EXPECT_TRUE(s2 >= s1);
  EXPECT_TRUE(s1 <= s1);
  EXPECT_TRUE(s1 >= s1);
}

TEST_F(StringBugBashTest, ConcatenationOperator) {
  String s1("hello");
  String s2(" world");
  String result = s1 + s2;
  EXPECT_STREQ(result.c_str(), "hello world");
}

TEST_F(StringBugBashTest, ConcatenationWithCString) {
  String s("hello");
  String result = s + " world";
  EXPECT_STREQ(result.c_str(), "hello world");

  String result2 = "prefix " + s;
  EXPECT_STREQ(result2.c_str(), "prefix hello");
}

TEST_F(StringBugBashTest, ReserveSmallToLarge) {
  String s("tiny");
  s.reserve(100);
  EXPECT_GE(s.capacity(), 100u);
  EXPECT_STREQ(s.c_str(), "tiny");
}

TEST_F(StringBugBashTest, ReserveLargeToLarger) {
  std::string initial(50, 'a');
  String s(initial.c_str());
  s.reserve(200);
  EXPECT_GE(s.capacity(), 200u);
  EXPECT_STREQ(s.c_str(), initial.c_str());
}

TEST_F(StringBugBashTest, ResizeGrow) {
  String s("hello");
  s.resize(10);
  EXPECT_EQ(s.size(), 10u);
  EXPECT_EQ(memcmp(s.c_str(), "hello", 5), 0);
  for (mem_size i = 5; i < 10; ++i) {
    EXPECT_EQ(s[i], '\0');
  }
}

TEST_F(StringBugBashTest, ResizeShrink) {
  String s("hello world");
  s.resize(5);
  EXPECT_EQ(s.size(), 5u);
  EXPECT_STREQ(s.c_str(), "hello");
}

TEST_F(StringBugBashTest, ClearString) {
  String s("hello");
  s.clear();
  EXPECT_TRUE(s.empty());
  EXPECT_STREQ(s.c_str(), "");
}

TEST_F(StringBugBashTest, ClearLargeString) {
  std::string large(100, 'a');
  String s(large.c_str());
  s.clear();
  EXPECT_TRUE(s.empty());
  EXPECT_STREQ(s.c_str(), "");
}

TEST_F(StringBugBashTest, ShrinkToFitLargeToSmall) {
  std::string initial(50, 'a');
  String s(initial.c_str());
  s.erase(5);  // now 5 chars, fits in SSO
  s.shrink_to_fit();
  EXPECT_EQ(s.size(), 5u);
  EXPECT_EQ(memcmp(s.c_str(), "aaaaa", 5), 0);
}

TEST_F(StringBugBashTest, ShrinkToFitLargeToLarger) {
  std::string initial(100, 'a');
  String s(initial.c_str());
  s.erase(50);  // now 50 chars, still large
  auto old_cap = s.capacity();
  s.shrink_to_fit();
  EXPECT_EQ(s.size(), 50u);
  EXPECT_LE(s.capacity(), old_cap);
}

TEST_F(StringBugBashTest, IteratorBasic) {
  String s("abc");
  int i = 0;
  const char expected[] = {'a', 'b', 'c'};
  for (auto c : s) {
    EXPECT_EQ(c, expected[i++]);
  }
  EXPECT_EQ(i, 3);
}

TEST_F(StringBugBashTest, BackAccess) {
  String s("hello");
  EXPECT_EQ(s.back(), 'o');
}

TEST_F(StringBugBashTest, IndexAccess) {
  String s("hello");
  EXPECT_EQ(s[0], 'h');
  EXPECT_EQ(s[4], 'o');
}

TEST_F(StringBugBashTest, AtAccess) {
  String s("hello");
  EXPECT_EQ(s.at(0), 'h');
  EXPECT_EQ(s.at(4), 'o');
}

TEST_F(StringBugBashTest, StressPushBackTransitions) {
  String s;
  for (int i = 0; i < 1000; ++i) {
    s.push_back('a' + (i % 26));
  }
  EXPECT_EQ(s.size(), 1000u);
  for (int i = 0; i < 1000; ++i) {
    EXPECT_EQ(s[i], 'a' + (i % 26));
  }
}

TEST_F(StringBugBashTest, RepeatedClearAppend) {
  String s;
  for (int cycle = 0; cycle < 100; ++cycle) {
    s.clear();
    for (int i = 0; i < 50; ++i) {
      s.push_back('x');
    }
    EXPECT_EQ(s.size(), 50u);
  }
}

TEST_F(StringBugBashTest, NullTerminationAfterOperations) {
  String s("hello");
  s.erase(3);
  EXPECT_EQ(s.c_str()[s.size()], '\0');

  s.append(" world");
  EXPECT_EQ(s.c_str()[s.size()], '\0');

  s.resize(3);
  EXPECT_EQ(s.c_str()[s.size()], '\0');

  s.push_back('!');
  EXPECT_EQ(s.c_str()[s.size()], '\0');

  s.clear();
  EXPECT_EQ(s.c_str()[s.size()], '\0');
}

TEST_F(StringBugBashTest, AssignFlagCorruption) {
  // Test the assign() flag corruption bug:
  // When transitioning small→large, assign() uses |= kLargeFlag
  // instead of = kLargeFlag, which could leave garbage in the flag byte.
  String s("tiny");  // starts small
  std::string large(100, 'b');
  s.assign(large.c_str(), large.size());
  EXPECT_EQ(s.size(), 100u);
  EXPECT_STREQ(s.c_str(), large.c_str());

  // Verify the string is still functional after the transition
  s.append("!");
  EXPECT_EQ(s.size(), 101u);
  s.clear();
  EXPECT_TRUE(s.empty());
}

TEST_F(StringBugBashTest, CompareWithCString) {
  String s("hello");
  EXPECT_EQ(s.compare("hello"), 0);
  EXPECT_LT(s.compare("world"), 0);
  EXPECT_GT(s.compare("abc"), 0);
}

TEST_F(StringBugBashTest, FindInEmptyString) {
  String s;
  EXPECT_EQ(s.find('a'), String::npos);
  EXPECT_EQ(s.find("hello"), String::npos);
}

TEST_F(StringBugBashTest, FindLastOfInEmptyString) {
  String s;
  EXPECT_EQ(s.find_last_of('a'), String::npos);
}

// ============================================================================
// UNORDERED MAP TESTS
// ============================================================================

class UnorderedMapBugBashTest : public ::testing::Test {
 protected:
  void SetUp() override { LifetimeTracker::Reset(); }
  void TearDown() override { EXPECT_EQ(LifetimeTracker::alive_count, 0); }
};

TEST_F(UnorderedMapBugBashTest, DefaultConstructor) {
  base::UnorderedMap<i32, i32> m;
  EXPECT_TRUE(m.empty());
  EXPECT_EQ(m.size(), 0u);
}

TEST_F(UnorderedMapBugBashTest, InsertAndFind) {
  base::UnorderedMap<i32, i32> m;
  auto [ptr, inserted] = m.insert(42, 100);
  EXPECT_TRUE(inserted);
  EXPECT_NE(ptr, nullptr);
  EXPECT_EQ(*ptr, 100);

  auto* found = m.find(42);
  ASSERT_NE(found, nullptr);
  EXPECT_EQ(*found, 100);
}

TEST_F(UnorderedMapBugBashTest, InsertDuplicate) {
  base::UnorderedMap<i32, i32> m;
  m.insert(42, 100);
  auto [ptr, inserted] = m.insert(42, 200);
  EXPECT_FALSE(inserted);
  EXPECT_EQ(*ptr, 100);  // original value preserved
  EXPECT_EQ(m.size(), 1u);
}

TEST_F(UnorderedMapBugBashTest, OperatorBracket) {
  base::UnorderedMap<i32, i32> m;
  m[1] = 10;
  m[2] = 20;
  m[3] = 30;
  EXPECT_EQ(m.size(), 3u);
  EXPECT_EQ(m[1], 10);
  EXPECT_EQ(m[2], 20);
  EXPECT_EQ(m[3], 30);
}

TEST_F(UnorderedMapBugBashTest, OperatorBracketCreatesDefault) {
  base::UnorderedMap<i32, i32> m;
  i32& val = m[42];
  EXPECT_EQ(val, 0);
  EXPECT_EQ(m.size(), 1u);
}

TEST_F(UnorderedMapBugBashTest, Contains) {
  base::UnorderedMap<i32, i32> m;
  m.insert(1, 10);
  EXPECT_TRUE(m.contains(1));
  EXPECT_FALSE(m.contains(2));
}

TEST_F(UnorderedMapBugBashTest, FindNonExistent) {
  base::UnorderedMap<i32, i32> m;
  EXPECT_EQ(m.find(42), nullptr);
}

TEST_F(UnorderedMapBugBashTest, FindInEmptyMap) {
  base::UnorderedMap<i32, i32> m;
  EXPECT_EQ(m.find(0), nullptr);
  EXPECT_FALSE(m.contains(0));
}

TEST_F(UnorderedMapBugBashTest, Erase) {
  base::UnorderedMap<i32, i32> m;
  m.insert(1, 10);
  m.insert(2, 20);
  m.insert(3, 30);

  EXPECT_TRUE(m.erase(2));
  EXPECT_EQ(m.size(), 2u);
  EXPECT_EQ(m.find(2), nullptr);
  EXPECT_NE(m.find(1), nullptr);
  EXPECT_NE(m.find(3), nullptr);
}

TEST_F(UnorderedMapBugBashTest, EraseNonExistent) {
  base::UnorderedMap<i32, i32> m;
  m.insert(1, 10);
  EXPECT_FALSE(m.erase(99));
  EXPECT_EQ(m.size(), 1u);
}

TEST_F(UnorderedMapBugBashTest, EraseFromEmpty) {
  base::UnorderedMap<i32, i32> m;
  EXPECT_FALSE(m.erase(1));
}

TEST_F(UnorderedMapBugBashTest, Clear) {
  base::UnorderedMap<i32, i32> m;
  for (i32 i = 0; i < 100; ++i) m.insert(i, i * 10);
  EXPECT_EQ(m.size(), 100u);
  m.clear();
  EXPECT_EQ(m.size(), 0u);
  EXPECT_TRUE(m.empty());
}

TEST_F(UnorderedMapBugBashTest, CopyConstructor) {
  base::UnorderedMap<i32, i32> m1;
  m1.insert(1, 10);
  m1.insert(2, 20);
  m1.insert(3, 30);

  base::UnorderedMap<i32, i32> m2(m1);
  EXPECT_EQ(m2.size(), 3u);
  EXPECT_NE(m2.find(1), nullptr);
  EXPECT_EQ(*m2.find(1), 10);
  EXPECT_NE(m2.find(2), nullptr);
  EXPECT_EQ(*m2.find(2), 20);
  EXPECT_NE(m2.find(3), nullptr);
  EXPECT_EQ(*m2.find(3), 30);
}

TEST_F(UnorderedMapBugBashTest, CopyConstructorEmpty) {
  base::UnorderedMap<i32, i32> m1;
  base::UnorderedMap<i32, i32> m2(m1);
  EXPECT_TRUE(m2.empty());
}

TEST_F(UnorderedMapBugBashTest, MoveConstructor) {
  base::UnorderedMap<i32, i32> m1;
  m1.insert(1, 10);
  m1.insert(2, 20);

  base::UnorderedMap<i32, i32> m2(base::move(m1));
  EXPECT_EQ(m2.size(), 2u);
  EXPECT_TRUE(m1.empty());
  EXPECT_EQ(*m2.find(1), 10);
}

TEST_F(UnorderedMapBugBashTest, CopyAssignment) {
  base::UnorderedMap<i32, i32> m1;
  m1.insert(1, 10);

  base::UnorderedMap<i32, i32> m2;
  m2.insert(99, 99);
  m2 = m1;
  EXPECT_EQ(m2.size(), 1u);
  EXPECT_EQ(*m2.find(1), 10);
  EXPECT_EQ(m2.find(99), nullptr);
}

TEST_F(UnorderedMapBugBashTest, CopyAssignmentSelf) {
  base::UnorderedMap<i32, i32> m;
  m.insert(1, 10);
  m = m;
  EXPECT_EQ(m.size(), 1u);
  EXPECT_EQ(*m.find(1), 10);
}

TEST_F(UnorderedMapBugBashTest, MoveAssignment) {
  base::UnorderedMap<i32, i32> m1;
  m1.insert(1, 10);

  base::UnorderedMap<i32, i32> m2;
  m2.insert(99, 99);
  m2 = base::move(m1);
  EXPECT_EQ(m2.size(), 1u);
  EXPECT_EQ(*m2.find(1), 10);
  EXPECT_TRUE(m1.empty());
}

TEST_F(UnorderedMapBugBashTest, Emplace) {
  base::UnorderedMap<i32, i32> m;
  auto [ptr, ok] = m.emplace(1, 42);
  EXPECT_TRUE(ok);
  EXPECT_EQ(*ptr, 42);

  auto [ptr2, ok2] = m.emplace(1, 99);
  EXPECT_FALSE(ok2);
  EXPECT_EQ(*ptr2, 42);
}

TEST_F(UnorderedMapBugBashTest, InsertRvalue) {
  base::UnorderedMap<i32, i32> m;
  auto [ptr, ok] = m.insert(1, i32(42));
  EXPECT_TRUE(ok);
  EXPECT_EQ(*ptr, 42);
}

TEST_F(UnorderedMapBugBashTest, Rehash) {
  base::UnorderedMap<i32, i32> m;
  for (i32 i = 0; i < 100; ++i) {
    m.insert(i, i * 10);
  }
  EXPECT_EQ(m.size(), 100u);
  for (i32 i = 0; i < 100; ++i) {
    auto* found = m.find(i);
    ASSERT_NE(found, nullptr) << "Missing key " << i;
    EXPECT_EQ(*found, i * 10);
  }
}

TEST_F(UnorderedMapBugBashTest, EraseAndReinsert) {
  base::UnorderedMap<i32, i32> m;
  for (i32 i = 0; i < 50; ++i) m.insert(i, i);
  for (i32 i = 0; i < 50; ++i) m.erase(i);
  EXPECT_EQ(m.size(), 0u);

  // Reinsert - tombstones shouldn't prevent finding
  for (i32 i = 0; i < 50; ++i) m.insert(i, i * 100);
  EXPECT_EQ(m.size(), 50u);
  for (i32 i = 0; i < 50; ++i) {
    auto* found = m.find(i);
    ASSERT_NE(found, nullptr) << "Missing key " << i;
    EXPECT_EQ(*found, i * 100);
  }
}

TEST_F(UnorderedMapBugBashTest, TombstoneStress) {
  base::UnorderedMap<i32, i32> m;
  // Insert and erase repeatedly to create many tombstones
  for (int cycle = 0; cycle < 10; ++cycle) {
    for (i32 i = 0; i < 100; ++i) {
      m.insert(i + cycle * 1000, i);
    }
    for (i32 i = 0; i < 100; ++i) {
      m.erase(i + cycle * 1000);
    }
  }
  EXPECT_EQ(m.size(), 0u);

  // Still functional
  m.insert(999, 42);
  EXPECT_EQ(*m.find(999), 42);
}

TEST_F(UnorderedMapBugBashTest, IteratorBasic) {
  base::UnorderedMap<i32, i32> m;
  m.insert(1, 10);
  m.insert(2, 20);
  m.insert(3, 30);

  int count = 0;
  i32 key_sum = 0, val_sum = 0;
  for (auto [key, value] : m) {
    key_sum += key;
    val_sum += value;
    ++count;
  }
  EXPECT_EQ(count, 3);
  EXPECT_EQ(key_sum, 6);
  EXPECT_EQ(val_sum, 60);
}

TEST_F(UnorderedMapBugBashTest, IteratorEmpty) {
  base::UnorderedMap<i32, i32> m;
  int count = 0;
  for (auto [key, value] : m) {
    (void)key;
    (void)value;
    ++count;
  }
  EXPECT_EQ(count, 0);
}

TEST_F(UnorderedMapBugBashTest, ConstIterator) {
  base::UnorderedMap<i32, i32> m;
  m.insert(1, 10);
  m.insert(2, 20);

  const auto& cm = m;
  int count = 0;
  for (auto [key, value] : cm) {
    (void)key;
    (void)value;
    ++count;
  }
  EXPECT_EQ(count, 2);
}

TEST_F(UnorderedMapBugBashTest, ForEach) {
  base::UnorderedMap<i32, i32> m;
  m.insert(1, 10);
  m.insert(2, 20);
  m.insert(3, 30);

  i32 key_sum = 0, val_sum = 0;
  m.ForEach([&](const i32& k, const i32& v) {
    key_sum += k;
    val_sum += v;
  });
  EXPECT_EQ(key_sum, 6);
  EXPECT_EQ(val_sum, 60);
}

TEST_F(UnorderedMapBugBashTest, ComplexValueType) {
  {
    base::UnorderedMap<i32, LifetimeTracker> m;
    m.emplace(1, 10);
    m.emplace(2, 20);
    m.emplace(3, 30);
    EXPECT_EQ(m.size(), 3u);

    auto* found = m.find(2);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->value_, 20);

    m.erase(2);
    EXPECT_EQ(m.size(), 2u);
  }
  EXPECT_EQ(LifetimeTracker::alive_count, 0);
}

TEST_F(UnorderedMapBugBashTest, StringKeys) {
  base::UnorderedMap<String, i32> m;
  m.insert(String("hello"), 1);
  m.insert(String("world"), 2);

  EXPECT_EQ(m.size(), 2u);
  auto* v1 = m.find(String("hello"));
  ASSERT_NE(v1, nullptr);
  EXPECT_EQ(*v1, 1);

  auto* v2 = m.find(String("world"));
  ASSERT_NE(v2, nullptr);
  EXPECT_EQ(*v2, 2);
}

TEST_F(UnorderedMapBugBashTest, LargeKeySpace) {
  base::UnorderedMap<i32, i32> m;
  for (i32 i = 0; i < 10000; ++i) {
    m.insert(i, i);
  }
  EXPECT_EQ(m.size(), 10000u);
  for (i32 i = 0; i < 10000; ++i) {
    auto* found = m.find(i);
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(*found, i);
  }
}

TEST_F(UnorderedMapBugBashTest, ComplexClearLifetime) {
  {
    base::UnorderedMap<i32, LifetimeTracker> m;
    for (i32 i = 0; i < 50; ++i) {
      m.emplace(i, i);
    }

    m.clear();
    EXPECT_EQ(m.size(), 0u);
  }
  EXPECT_EQ(LifetimeTracker::alive_count, 0);
}

TEST_F(UnorderedMapBugBashTest, UpdateExistingValue) {
  base::UnorderedMap<i32, i32> m;
  m[1] = 10;
  m[1] = 20;
  EXPECT_EQ(m.size(), 1u);
  EXPECT_EQ(m[1], 20);
}

TEST_F(UnorderedMapBugBashTest, NegativeKeys) {
  base::UnorderedMap<i32, i32> m;
  m.insert(-1, 10);
  m.insert(-100, 20);
  m.insert(0, 30);
  EXPECT_EQ(m.size(), 3u);
  EXPECT_EQ(*m.find(-1), 10);
  EXPECT_EQ(*m.find(-100), 20);
  EXPECT_EQ(*m.find(0), 30);
}

TEST_F(UnorderedMapBugBashTest, U64Keys) {
  base::UnorderedMap<u64, i32> m;
  m.insert(0ULL, 0);
  m.insert(UINT64_MAX, 1);
  m.insert(UINT64_MAX / 2, 2);
  EXPECT_EQ(m.size(), 3u);
  EXPECT_EQ(*m.find(0ULL), 0);
  EXPECT_EQ(*m.find(UINT64_MAX), 1);
  EXPECT_EQ(*m.find(UINT64_MAX / 2), 2);
}

// ============================================================================
// OPTIONAL TESTS
// ============================================================================

TEST(OptionalBugBash, DefaultEmpty) {
  base::Optional<i32> o;
  EXPECT_FALSE(o.has_value());
}

TEST(OptionalBugBash, WithValue) {
  base::Optional<i32> o(42);
  EXPECT_TRUE(o.has_value());
  EXPECT_EQ(o.value(), 42);
}

TEST(OptionalBugBash, Reset) {
  base::Optional<i32> o(42);
  o.reset();
  EXPECT_FALSE(o.has_value());
}

TEST(OptionalBugBash, CopyConstruct) {
  base::Optional<i32> o1(42);
  base::Optional<i32> o2(o1);
  EXPECT_TRUE(o2.has_value());
  EXPECT_EQ(o2.value(), 42);
}

TEST(OptionalBugBash, MoveConstruct) {
  base::Optional<i32> o1(42);
  base::Optional<i32> o2(base::move(o1));
  EXPECT_TRUE(o2.has_value());
  EXPECT_EQ(o2.value(), 42);
}

// ============================================================================
// FUNCTION TESTS
// ============================================================================

TEST(FunctionBugBash, EmptyFunction) {
  base::Function<void()> f;
  // Should not crash when empty
}

TEST(FunctionBugBash, Lambda) {
  base::Function<i32(i32, i32)> f = [](i32 a, i32 b) { return a + b; };
  EXPECT_EQ(f(3, 4), 7);
}

TEST(FunctionBugBash, CapturingLambda) {
  i32 captured = 42;
  base::Function<i32()> f = [captured]() { return captured; };
  EXPECT_EQ(f(), 42);
}

TEST(FunctionBugBash, MutableLambda) {
  i32 counter = 0;
  base::Function<void()> f = [&counter]() { ++counter; };
  f();
  f();
  EXPECT_EQ(counter, 2);
}

TEST(FunctionBugBash, MoveOnly) {
  auto f1 = base::Function<i32()>([](){ return 42; });
  auto f2 = base::move(f1);
  EXPECT_EQ(f2(), 42);
}

// ============================================================================
// HASH FUNCTION TESTS
// ============================================================================

TEST(HashBugBash, FNV1aConsistency) {
  const char* data = "hello";
  auto h1 = base::fnv1a(reinterpret_cast<const u8*>(data), 5);
  auto h2 = base::fnv1a(reinterpret_cast<const u8*>(data), 5);
  EXPECT_EQ(h1, h2);
}

TEST(HashBugBash, FNV1aDifferentInputs) {
  const char* d1 = "hello";
  const char* d2 = "world";
  auto h1 = base::fnv1a(reinterpret_cast<const u8*>(d1), 5);
  auto h2 = base::fnv1a(reinterpret_cast<const u8*>(d2), 5);
  EXPECT_NE(h1, h2);
}

TEST(HashBugBash, U32Hash) {
  base::Hash<u32> h;
  // Different inputs should produce different hashes
  EXPECT_NE(h(0), h(1));
  EXPECT_NE(h(1), h(2));
  // Same input should produce same hash
  EXPECT_EQ(h(42), h(42));
}

TEST(HashBugBash, U64Hash) {
  base::Hash<u64> h;
  EXPECT_NE(h(0ULL), h(1ULL));
  EXPECT_EQ(h(42ULL), h(42ULL));
}

TEST(HashBugBash, StringHash) {
  base::Hash<String> h;
  String s1("hello");
  String s2("hello");
  String s3("world");
  EXPECT_EQ(h(s1), h(s2));
  EXPECT_NE(h(s1), h(s3));
}

// ============================================================================
// MATH HELPERS TESTS
// ============================================================================

TEST(MathBugBash, IsPowerOf2) {
  EXPECT_TRUE(base::IsPowerOf2(1u));
  EXPECT_TRUE(base::IsPowerOf2(2u));
  EXPECT_TRUE(base::IsPowerOf2(4u));
  EXPECT_TRUE(base::IsPowerOf2(1024u));
  EXPECT_FALSE(base::IsPowerOf2(0u));
  EXPECT_FALSE(base::IsPowerOf2(3u));
  EXPECT_FALSE(base::IsPowerOf2(6u));
}

TEST(MathBugBash, NextPowerOf2) {
  // Note: NextPowerOf2(0) is UB (shift overflow) - don't test it
  EXPECT_EQ(base::NextPowerOf2(1u), 1u);
  EXPECT_EQ(base::NextPowerOf2(3u), 4u);
  EXPECT_EQ(base::NextPowerOf2(5u), 8u);
  EXPECT_EQ(base::NextPowerOf2(1023u), 1024u);
  EXPECT_EQ(base::NextPowerOf2(1024u), 1024u);
}

TEST(MathBugBash, CountLeadingZeros) {
  EXPECT_EQ(base::CountLeadingZeros(1u), 31u);
  EXPECT_EQ(base::CountLeadingZeros(2u), 30u);
  EXPECT_EQ(base::CountLeadingZeros(0x80000000u), 0u);
}

TEST(MathBugBash, Log2) {
  EXPECT_EQ(base::Log2(1u), 0u);
  EXPECT_EQ(base::Log2(2u), 1u);
  EXPECT_EQ(base::Log2(4u), 2u);
  EXPECT_EQ(base::Log2(1024u), 10u);
}

// ============================================================================
// VECTOR + STRING INTEGRATION
// ============================================================================

TEST(IntegrationBugBash, VectorOfStrings) {
  base::Vector<String> v;
  v.push_back(String("hello"));
  v.push_back(String("world"));
  v.push_back(String("this is a long string that exceeds SSO capacity"));

  EXPECT_EQ(v.size(), 3u);
  EXPECT_STREQ(v[0].c_str(), "hello");
  EXPECT_STREQ(v[1].c_str(), "world");
  EXPECT_STREQ(v[2].c_str(), "this is a long string that exceeds SSO capacity");

  // Copy the vector
  base::Vector<String> v2(v);
  EXPECT_EQ(v2.size(), 3u);
  EXPECT_STREQ(v2[2].c_str(), "this is a long string that exceeds SSO capacity");

  // Clear original
  v.clear();
  EXPECT_TRUE(v.empty());
  // Copy should be unaffected
  EXPECT_STREQ(v2[0].c_str(), "hello");
}

TEST(IntegrationBugBash, VectorOfStringsRealloc) {
  base::Vector<String> v;
  for (int i = 0; i < 100; ++i) {
    char buf[64];
    snprintf(buf, sizeof(buf), "string_%d_with_extra_padding_for_length", i);
    v.push_back(String(buf));
  }
  EXPECT_EQ(v.size(), 100u);
  for (int i = 0; i < 100; ++i) {
    char buf[64];
    snprintf(buf, sizeof(buf), "string_%d_with_extra_padding_for_length", i);
    EXPECT_STREQ(v[i].c_str(), buf);
  }
}

TEST(IntegrationBugBash, UnorderedMapOfStrings) {
  base::UnorderedMap<String, String> m;
  m.insert(String("key1"), String("value1"));
  m.insert(String("key2"), String("value2 is a bit longer to test heap"));

  auto* v = m.find(String("key1"));
  ASSERT_NE(v, nullptr);
  EXPECT_STREQ(v->c_str(), "value1");

  auto* v2 = m.find(String("key2"));
  ASSERT_NE(v2, nullptr);
  EXPECT_STREQ(v2->c_str(), "value2 is a bit longer to test heap");
}

TEST(IntegrationBugBash, MapWithVectorValues) {
  base::UnorderedMap<i32, base::Vector<i32>> m;
  m[1] = base::Vector<i32>{10, 20, 30};
  m[2] = base::Vector<i32>{40, 50};

  auto* v1 = m.find(1);
  ASSERT_NE(v1, nullptr);
  EXPECT_EQ(v1->size(), 3u);
  EXPECT_EQ((*v1)[0], 10);
}

TEST(IntegrationBugBash, EraseInsertCycle) {
  base::Vector<i32> v;
  // Build up, tear down, rebuild - tests for memory corruption
  for (int cycle = 0; cycle < 50; ++cycle) {
    v.clear();
    for (i32 i = 0; i < 100; ++i) {
      v.push_back(i);
    }
    // Erase every other element
    for (mem_size i = 0; i < v.size(); i += 1) {
      v.erase(i);
    }
  }
}

// ============================================================================
// EDGE CASE REGRESSION TESTS
// ============================================================================

TEST(RegressionBugBash, VectorInsertCausesRealloc) {
  // Regression: insert at begin when at capacity should not corrupt data
  base::Vector<i32> v;
  v.reserve(4);
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);
  v.push_back(4);
  // Now at capacity
  EXPECT_EQ(v.capacity(), 4u);

  v.insert(v.begin(), 0);
  EXPECT_EQ(v.size(), 5u);
  EXPECT_EQ(v[0], 0);
  EXPECT_EQ(v[1], 1);
  EXPECT_EQ(v[2], 2);
  EXPECT_EQ(v[3], 3);
  EXPECT_EQ(v[4], 4);
}

TEST(RegressionBugBash, VectorInsertMultipleCopiesCausesRealloc) {
  base::Vector<i32> v = {1, 2, 3};
  v.insert(v.begin() + 1, static_cast<mem_size>(10), 42);
  EXPECT_EQ(v.size(), 13u);
  EXPECT_EQ(v[0], 1);
  for (mem_size i = 1; i < 11; ++i) EXPECT_EQ(v[i], 42);
  EXPECT_EQ(v[11], 2);
  EXPECT_EQ(v[12], 3);
}

TEST(RegressionBugBash, StringRepeatedSmallLargeTransition) {
  String s;
  for (int i = 0; i < 100; ++i) {
    // Build up past SSO
    for (int j = 0; j < 30; ++j) s.push_back('a');
    EXPECT_EQ(s.size(), static_cast<mem_size>(30));
    EXPECT_STREQ(s.c_str(), "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    // Clear back to empty
    s.clear();
    EXPECT_TRUE(s.empty());
    EXPECT_STREQ(s.c_str(), "");
  }
}

TEST(RegressionBugBash, MapEraseAndLookup) {
  // Regression: after erase, tombstones should not prevent finding existing keys
  base::UnorderedMap<i32, i32> m;
  m.insert(1, 100);
  m.insert(2, 200);
  m.insert(3, 300);

  m.erase(2);
  // Keys 1 and 3 should still be findable
  ASSERT_NE(m.find(1), nullptr);
  EXPECT_EQ(*m.find(1), 100);
  ASSERT_NE(m.find(3), nullptr);
  EXPECT_EQ(*m.find(3), 300);
}

TEST(RegressionBugBash, StringCopyAfterMove) {
  String s1("hello world this is a long string");
  String s2(base::move(s1));
  // s1 should be valid empty string
  String s3(s1);  // copy from moved-from should work
  EXPECT_TRUE(s3.empty());
}

TEST(RegressionBugBash, VectorMoveAssignToNonEmpty) {
  base::Vector<i32> v1 = {1, 2, 3};
  base::Vector<i32> v2 = {4, 5, 6, 7, 8};
  v2 = base::move(v1);
  EXPECT_EQ(v2.size(), 3u);
  EXPECT_TRUE(v1.empty());
  // v2's old elements should be properly destroyed
}

// ============================================================================
// EXPECTED TESTS
// ============================================================================

TEST(ExpectedBugBash, ConstructWithValue) {
  enum class MyError { kBad };
  base::Expected<i32, MyError> e(42);
  EXPECT_FALSE(e.has_error());
  EXPECT_EQ(e.value(), 42);
}

TEST(ExpectedBugBash, ConstructWithError) {
  enum class MyError { kBad };
  base::Expected<i32, MyError> e(MyError::kBad);
  EXPECT_TRUE(e.has_error());
  EXPECT_EQ(e.error(), MyError::kBad);
}

TEST(ExpectedBugBash, CopyConstructValue) {
  base::Expected<i32, float> e1(42);
  base::Expected<i32, float> e2(e1);
  EXPECT_FALSE(e2.has_error());
  EXPECT_EQ(e2.value(), 42);
}

TEST(ExpectedBugBash, CopyConstructError) {
  base::Expected<i32, float> e1(3.14f);
  base::Expected<i32, float> e2(e1);
  EXPECT_TRUE(e2.has_error());
}

TEST(ExpectedBugBash, AssignmentPreservesErrorState) {
  // This test caught a bug: operator= was assigning expected_value_ to has_error_
  base::Expected<i32, float> e1(42);
  base::Expected<i32, float> e2(1.0f);
  e2 = e1;
  EXPECT_FALSE(e2.has_error());
  EXPECT_EQ(e2.value(), 42);
}

// ============================================================================
// SPAN TESTS
// ============================================================================

TEST(SpanBugBash, EmptySpan) {
  base::Span<i32> s(static_cast<const i32*>(nullptr), 0);
  EXPECT_TRUE(s.empty());
  EXPECT_EQ(s.size(), 0u);
}

TEST(SpanBugBash, NonNullButZeroLength) {
  i32 arr[] = {1, 2, 3};
  base::Span<i32> s(arr, 0);
  EXPECT_TRUE(s.empty());  // Previously returned false (wrong!)
  EXPECT_EQ(s.size(), 0u);
}

TEST(SpanBugBash, BasicAccess) {
  i32 arr[] = {10, 20, 30};
  base::Span<i32> s(arr, 3);
  EXPECT_EQ(s.size(), 3u);
  EXPECT_FALSE(s.empty());
  EXPECT_EQ(s[0], 10);
  EXPECT_EQ(s[2], 30);
}

TEST(SpanBugBash, FrontAndBack) {
  i32 arr[] = {10, 20, 30};
  base::Span<i32> s(arr, 3);
  EXPECT_EQ(s.front(), 10);
  EXPECT_EQ(s.back(), 30);
}

TEST(SpanBugBash, FromVector) {
  base::Vector<i32> v = {1, 2, 3, 4, 5};
  auto s = base::MakeSpan(v);
  EXPECT_EQ(s.size(), 5u);
  EXPECT_EQ(s[0], 1);
  EXPECT_EQ(s[4], 5);
}

TEST(SpanBugBash, IterateSpan) {
  i32 arr[] = {10, 20, 30};
  base::Span<i32> s(arr, 3);
  i32 sum = 0;
  for (auto val : s) {
    sum += val;
  }
  EXPECT_EQ(sum, 60);
}

TEST(SpanBugBash, FromArray) {
  i32 arr[] = {1, 2, 3, 4};
  base::Span<i32> s(arr);
  EXPECT_EQ(s.size(), 4u);
  EXPECT_EQ(s[3], 4);
}

// ============================================================================
// STRING REF TESTS
// ============================================================================

TEST(StringRefBugBash, LexicographicComparison) {
  // Previously operator< only compared lengths, not content
  base::BasicStringRef<char> a("abc");
  base::BasicStringRef<char> b("abd");
  EXPECT_TRUE(a < b);
  EXPECT_FALSE(b < a);
}

TEST(StringRefBugBash, SameLengthDifferentContent) {
  base::BasicStringRef<char> a("aaa");
  base::BasicStringRef<char> b("zzz");
  EXPECT_TRUE(a < b);
  EXPECT_FALSE(b < a);
}

TEST(StringRefBugBash, EqualStringsNotLessThan) {
  base::BasicStringRef<char> a("hello");
  base::BasicStringRef<char> b("hello");
  EXPECT_FALSE(a < b);
  EXPECT_FALSE(b < a);
  EXPECT_TRUE(a <= b);
  EXPECT_TRUE(a >= b);
}

TEST(StringRefBugBash, PrefixComparison) {
  base::BasicStringRef<char> a("abc");
  base::BasicStringRef<char> b("abcdef");
  EXPECT_TRUE(a < b);   // shorter prefix comes first
  EXPECT_FALSE(b < a);
}

}  // namespace
