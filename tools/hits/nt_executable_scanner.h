// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Scan a windows 'pe' image for patterns.
#pragma once

#include <base/io/file/file_path.h>
#include <base/io/file/file.h>

#include <vector>
#include <mem/module.h>

class NtExecutableScanner {
 public:
  explicit NtExecutableScanner(const base::FilePath&);
  ~NtExecutableScanner();

  void LoadFromFile(const base::FilePath&);

  mem::pointer Scan(const std::string_view signature);

 private:
  bool CheckAndLoad();

  uint32_t TranslateToVirtual(uint32_t offset);

 private:
  std::unique_ptr<char[]> buffer_;
  mem::module module_;
  std::vector<mem::region> m_codeSegs;
};