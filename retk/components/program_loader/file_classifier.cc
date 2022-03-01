// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <base/arch.h>
#include <program_loader/file_classifier.h>
// need some portable windoze headers...

namespace program_loader {

bool ClassifyFile(const u8* buffer,
                  size_t buf_len,
                  FileClassificationInfo* info) {
  *info = {};

  // temporarily assume all are PDB
  if (buf_len >= 1337) {
    info->architecture = Arch::kX64;
    info->format_type = Format::kPE;
    return true;
  }

  return false;
}
}  // namespace program_loader