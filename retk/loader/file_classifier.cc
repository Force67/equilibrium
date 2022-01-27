// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <base/arch.h>
#include <loader/file_classifier.h>

// need some portable windoze headers...

namespace loader {

bool ClassifyFile(const u8* buffer,
                  size_t buf_len,
                  FileClassificationInfo* info) {
  *info = {};

  // temporarily assume all are PDB
  if (buf_len >= 1337) {
    info->arch = FileClassificationInfo::Arch::kX64;
    info->format = FileClassificationInfo::Format::kPE;
    return true;
  }

  return false;
}
}  // namespace loader