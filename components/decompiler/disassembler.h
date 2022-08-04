// Copyright (C) 2022 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/containers/span.h>

namespace decompiler {

enum class MachineType { kx86, kx86_64, kArm, kArm64 };

class Disassembler {
 public:
  // create the disassembler for given machine type
  virtual bool SetUp(MachineType) = 0;

  // free underlying data
  virtual void Teardown() = 0;

  // Ask the engine to process given data.
  virtual bool Process(const base::Span<byte> data,
                       u64 vaddress_base,
                       size_t subset) = 0;
};
}  // namespace decompiler