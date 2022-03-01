// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <program_loader/program_loader.h>

namespace program_loader {

class LoaderPE final : public ProgramLoader {
 public:
  bool Accept(const base::File&) override { return true; }

  // Inherited via ProgramLoader
  bool Parse(const FileClassificationInfo& intel,
             ProgramData& out,
             const u8* ptr) override;

  static ProgramLoadDescriptor* descriptor();
};
}  // namespace program_loader