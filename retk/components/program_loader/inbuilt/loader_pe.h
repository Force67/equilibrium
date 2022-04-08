// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <program_loader/program_loader.h>
#include <program_loader/file_classifier.h>

namespace program_loader {

class LoaderPE final : public ProgramLoader {
 public:
  bool Accept(const base::Span<byte> data,
              const FileClassificationInfo& in) override;

  // Inherited via ProgramLoader
  bool Parse(const base::Span<byte> data,
             const FileClassificationInfo& intel,
             ProgramData& out) override;

  static ProgramLoadDescriptor* descriptor();

  // TODO(Vince): Move in the future.
  static FileClassifier* classifier();
};
}  // namespace program_loader