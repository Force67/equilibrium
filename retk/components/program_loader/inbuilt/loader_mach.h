// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <program_loader/program_loader.h>

namespace program_loader {

class LoaderMach final : public ProgramLoader {
 public:
  bool Accept(const base::Span<byte> data, FileClassificationInfo&) override;

  // Inherited via ProgramLoader
  bool Parse(const base::Span<byte> data,
             const FileClassificationInfo& intel,
             ProgramData& out) override;

  static ProgramLoadDescriptor* descriptor();
};
}  // namespace program_loader