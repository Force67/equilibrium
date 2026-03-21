// Copyright (C) 2024 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// STL-free algorithm replacements: sort, find_if, fill, copy, swap, clamp, etc.
#pragma once

#include <base/arch.h>
#include <base/memory/move.h>
#include <base/math/value_bounds.h>

namespace base {

// ── Swap ────────────────────────────────────────────────────────────
template <typename T>
inline void Swap(T& a, T& b) {
  T tmp = base::move(a);
  a = base::move(b);
  b = base::move(tmp);
}

// ── Clamp ───────────────────────────────────────────────────────────
template <typename T>
inline T Clamp(T value, T lo, T hi) {
  return base::Max(lo, base::Min(value, hi));
}

// ── Fill ────────────────────────────────────────────────────────────
template <typename Iter, typename T>
inline void Fill(Iter first, Iter last, const T& value) {
  for (; first != last; ++first) {
    *first = value;
  }
}

// ── Copy ────────────────────────────────────────────────────────────
template <typename InputIt, typename OutputIt>
inline OutputIt Copy(InputIt first, InputIt last, OutputIt dest) {
  for (; first != last; ++first, ++dest) {
    *dest = *first;
  }
  return dest;
}

// ── Find ────────────────────────────────────────────────────────────
template <typename Iter, typename T>
inline Iter Find(Iter first, Iter last, const T& value) {
  for (; first != last; ++first) {
    if (*first == value) return first;
  }
  return last;
}

// ── FindIf ──────────────────────────────────────────────────────────
template <typename Iter, typename Pred>
inline Iter FindIf(Iter first, Iter last, Pred pred) {
  for (; first != last; ++first) {
    if (pred(*first)) return first;
  }
  return last;
}

// ── AnyOf ───────────────────────────────────────────────────────────
template <typename Iter, typename Pred>
inline bool AnyOf(Iter first, Iter last, Pred pred) {
  for (; first != last; ++first) {
    if (pred(*first)) return true;
  }
  return false;
}

// ── AllOf ───────────────────────────────────────────────────────────
template <typename Iter, typename Pred>
inline bool AllOf(Iter first, Iter last, Pred pred) {
  for (; first != last; ++first) {
    if (!pred(*first)) return false;
  }
  return true;
}

// ── Count ───────────────────────────────────────────────────────────
template <typename Iter, typename T>
inline mem_size Count(Iter first, Iter last, const T& value) {
  mem_size n = 0;
  for (; first != last; ++first) {
    if (*first == value) ++n;
  }
  return n;
}

// ── CountIf ─────────────────────────────────────────────────────────
template <typename Iter, typename Pred>
inline mem_size CountIf(Iter first, Iter last, Pred pred) {
  mem_size n = 0;
  for (; first != last; ++first) {
    if (pred(*first)) ++n;
  }
  return n;
}

// ── RemoveIf (unstable - swaps to end) ──────────────────────────────
template <typename Iter, typename Pred>
inline Iter RemoveIf(Iter first, Iter last, Pred pred) {
  Iter result = first;
  for (; first != last; ++first) {
    if (!pred(*first)) {
      if (result != first) {
        *result = base::move(*first);
      }
      ++result;
    }
  }
  return result;
}

// ── Iota ────────────────────────────────────────────────────────────
template <typename Iter, typename T>
inline void Iota(Iter first, Iter last, T value) {
  for (; first != last; ++first, ++value) {
    *first = value;
  }
}

// ── Accumulate ──────────────────────────────────────────────────────
template <typename Iter, typename T>
inline T Accumulate(Iter first, Iter last, T init) {
  for (; first != last; ++first) {
    init = init + *first;
  }
  return init;
}

// ── Sort (introsort: quicksort + insertion sort for small ranges) ──
namespace detail {

template <typename T>
inline void InsertionSort(T* first, T* last) {
  for (T* i = first + 1; i < last; ++i) {
    T key = base::move(*i);
    T* j = i - 1;
    while (j >= first && key < *j) {
      *(j + 1) = base::move(*j);
      --j;
    }
    *(j + 1) = base::move(key);
  }
}

template <typename T, typename Comp>
inline void InsertionSortComp(T* first, T* last, Comp comp) {
  for (T* i = first + 1; i < last; ++i) {
    T key = base::move(*i);
    T* j = i - 1;
    while (j >= first && comp(key, *j)) {
      *(j + 1) = base::move(*j);
      --j;
    }
    *(j + 1) = base::move(key);
  }
}

template <typename T>
inline T* Partition(T* first, T* last) {
  T* pivot = last - 1;
  T* i = first;
  for (T* j = first; j < pivot; ++j) {
    if (*j < *pivot) {
      base::Swap(*i, *j);
      ++i;
    }
  }
  base::Swap(*i, *pivot);
  return i;
}

template <typename T, typename Comp>
inline T* PartitionComp(T* first, T* last, Comp comp) {
  T* pivot = last - 1;
  T* i = first;
  for (T* j = first; j < pivot; ++j) {
    if (comp(*j, *pivot)) {
      base::Swap(*i, *j);
      ++i;
    }
  }
  base::Swap(*i, *pivot);
  return i;
}

template <typename T>
inline void IntroSortImpl(T* first, T* last, int depth) {
  while (last - first > 16) {
    if (depth == 0) {
      // Fallback to insertion sort to avoid O(n^2) worst case
      InsertionSort(first, last);
      return;
    }
    --depth;
    T* pivot = Partition(first, last);
    // Recurse on smaller partition, iterate on larger
    if (pivot - first < last - pivot) {
      IntroSortImpl(first, pivot, depth);
      first = pivot + 1;
    } else {
      IntroSortImpl(pivot + 1, last, depth);
      last = pivot;
    }
  }
  InsertionSort(first, last);
}

template <typename T, typename Comp>
inline void IntroSortImplComp(T* first, T* last, Comp comp, int depth) {
  while (last - first > 16) {
    if (depth == 0) {
      InsertionSortComp(first, last, comp);
      return;
    }
    --depth;
    T* pivot = PartitionComp(first, last, comp);
    if (pivot - first < last - pivot) {
      IntroSortImplComp(first, pivot, comp, depth);
      first = pivot + 1;
    } else {
      IntroSortImplComp(pivot + 1, last, comp, depth);
      last = pivot;
    }
  }
  InsertionSortComp(first, last, comp);
}

inline int Log2(mem_size n) {
  int k = 0;
  while (n > 1) { n >>= 1; ++k; }
  return k;
}

}  // namespace detail

// Sort a range [first, last) using operator<
template <typename T>
inline void Sort(T* first, T* last) {
  if (last - first <= 1) return;
  int maxDepth = 2 * detail::Log2(last - first);
  detail::IntroSortImpl(first, last, maxDepth);
}

// Sort a range [first, last) using a custom comparator
template <typename T, typename Comp>
inline void Sort(T* first, T* last, Comp comp) {
  if (last - first <= 1) return;
  int maxDepth = 2 * detail::Log2(last - first);
  detail::IntroSortImplComp(first, last, comp, maxDepth);
}

// ── Reverse ─────────────────────────────────────────────────────────
template <typename T>
inline void Reverse(T* first, T* last) {
  while (first < last) {
    --last;
    base::Swap(*first, *last);
    ++first;
  }
}

// ── Unique (removes consecutive duplicates) ─────────────────────────
template <typename T>
inline T* Unique(T* first, T* last) {
  if (first == last) return last;
  T* result = first;
  while (++first != last) {
    if (!(*result == *first)) {
      ++result;
      if (result != first)
        *result = base::move(*first);
    }
  }
  return ++result;
}

// ── LowerBound (binary search) ──────────────────────────────────────
template <typename T>
inline T* LowerBound(T* first, T* last, const T& value) {
  while (first < last) {
    T* mid = first + (last - first) / 2;
    if (*mid < value)
      first = mid + 1;
    else
      last = mid;
  }
  return first;
}

}  // namespace base
