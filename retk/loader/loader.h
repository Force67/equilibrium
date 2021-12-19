// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/filesystem/file.h>

namespace loader {

struct ImageDataBase {
  int64_t len;
};

class Loader {
 public:
  virtual ~Loader() = default;
  // Query if we want to be considered for loading given file
  virtual bool CreateModule(const base::File&) { return false; }

  virtual std::unique_ptr<ImageDataBase> Load() = 0;
};
}  // namespace loader