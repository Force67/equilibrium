// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace base {
template <typename T>
struct DefaultDeleter {
  static void Delete(T* raw_pointer) { delete raw_pointer; }
};
}  // namespace base