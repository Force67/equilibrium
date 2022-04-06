// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <base/arch.h>
#include <base/win/minwin.h>

#include <program_loader/file_reader.h>
#include <program_loader/file_classifier.h>
// need some portable windoze headers...

namespace program_loader {

namespace {


// https://github.com/llvm/llvm-project/blob/d480f968ad8b56d3ee4a6b6df5532d485b0ad01e/llvm/include/llvm/Object/ELFTypes.h

// https://github.com/llvm/llvm-project/blob/d480f968ad8b56d3ee4a6b6df5532d485b0ad01e/llvm/include/llvm/Object/COFF.h

// https://github.com/llvm/llvm-project/blob/d480f968ad8b56d3ee4a6b6df5532d485b0ad01e/llvm/include/llvm/Object/ELFTypes.h


// https://github.com/llvm/llvm-project/blob/d480f968ad8b56d3ee4a6b6df5532d485b0ad01e/llvm/include/llvm/BinaryFormat/ELF.h
}  // namespace


bool ClassifyFile(base::Span<byte> buffer, FileClassificationInfo* info) {
  *info = {};




  return false;
}
}  // namespace program_loader