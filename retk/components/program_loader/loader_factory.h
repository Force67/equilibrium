// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <vector>
#include <program_loader/program_loader.h>
#include <program_loader/file_classifier.h>

namespace program_loader {
// The purpose of this class is to automatically select the best matching file
// format and create a representation

class LoaderSink {
 public:
  virtual ~LoaderSink() = default;

  virtual void AddSegment() = 0;
};

class LoaderFactory {
 public:
  LoaderFactory();

  void FindApplicableCanidates(const FileClassificationInfo&,
                               std::vector<const ProgramLoadDescriptor*>& out);

  // in an ideal world this returns multiple close matches
  std::unique_ptr<ProgramLoader> CreateLoader(const ProgramLoadDescriptor&,
                                              const FileClassificationInfo&);

  // Search for user created loaders.
  void CollectCustomLoaders();

 private:
  std::vector<ProgramLoadDescriptor*> known_loaders_;
};
}  // namespace program_loader