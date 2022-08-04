// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Simple cxx wrapper around a capstone instance.
#pragma once

#include <decompiler/disassembler.h>

struct cs_insn;

namespace decompiler {
using namespace arch_types;

enum class MachineType { kx86, kx86_64, kArm, kArm64 };

class CapstoneDissasembler final : public Disassembler {
 public:
  CapstoneDissasembler() = default;
  CapstoneDissasembler(MachineType);
  ~CapstoneDissasembler();

  bool SetUp(MachineType) override;
  void Teardown() override;

  bool Process(const base::Span<byte> data, u64 vaddress_base, size_t subset);

 private:
  size_t handle_ = 0;
  size_t di_count_ = 0;
  cs_insn* insn_ = nullptr;
};
}  // namespace decompiler