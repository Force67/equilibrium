// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <base/arch.h>
#include <program_loader/file_reader.h>
#include <program_loader/file_classifier.h>

#include "inbuilt/loader_pe.h"
#include "inbuilt/loader_elf.h"
#include "inbuilt/loader_mach-o.h"

namespace program_loader {

// rather temporary fix?
static FileClassifier* kClassifiers[]{
    LoaderPE::classifier(), LoaderELF::classifier(), LoaderMachO::classifier()};

bool ClassifyFile(base::Span<byte> buffer, FileClassificationInfo* info) {
  *info = {};

  for (const FileClassifier* f : kClassifiers) {
    if (!f->classify(buffer, *info)) {
      __debugbreak();
    }
  }

  return false;
}
}  // namespace program_loader