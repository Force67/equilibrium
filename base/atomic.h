// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <atomic>

namespace base {

// this is one of the most complex classes

// for now.
template <typename T>
using Atomic = std::atomic<T>;
}  // namespace base