// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Simple cxx wrapper around a capstone instance.
#pragma once

#include <base/arch.h>

struct cs_insn;

namespace decompiler {
using namespace arch_types;

enum class MachineType { kx86, kx86_64, kArm, kArm64 };

class CapstoneDissasembler {
 public:
  CapstoneDissasembler() = default;
  CapstoneDissasembler(MachineType);
  ~CapstoneDissasembler();

  bool Create(MachineType);
  void Destroy();

  bool Process(const u8* data, size_t len, u64 vaddress_base, size_t subset);

 private:
  size_t handle_ = 0;
  size_t di_count_ = 0;
  cs_insn* insn_ = nullptr;
};
}  // namespace decompiler