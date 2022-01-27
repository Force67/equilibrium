// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <vector>
#include <loader/loader.h>
#include <loader/file_classifier.h>

namespace loader {
// The purpose of this class is to automatically select the best matching file
// format and create a representation

struct Load_Entry {};

struct Data {};

class LoaderSink {
 public:
  virtual void AddSegment() = 0;
};

class LoaderFactory {
 public:
  LoaderFactory() = default;

  // in an ideal world this returns multiple close matches
  std::unique_ptr<Data> LoadFile(const FileClassificationInfo& type_info);
};
}  // namespace loader