// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "loader_pe.h"

namespace program_loader {
static constinit ProgramLoadDescriptor kDescriptor{
    "Portable Executeable", "Load Windows Programs", Format::kPE,
    Arch::kX64 | Arch::kX86, ProgramLoadDescriptor::Flags::kInternal};

ProgramLoadDescriptor* LoaderPE::descriptor() {
  return &kDescriptor;
}

bool LoaderPE::Parse(const FileClassificationInfo& intel,
                     ProgramData& out,
                     const u8* ptr) {
  return false;
}

}  // namespace program_loader
