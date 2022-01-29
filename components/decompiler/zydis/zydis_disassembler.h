// Copyright (C) 2022 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Simple cxx wrapper around a zydis instance.
#pragma once

#include <base/arch.h>
#include <zydis/Decoder.h>

namespace decompiler {

// Make sure to never change the order of this.
enum class MachineType { kx86, kx86_64, kArm, kArm64 };

class ZydisDisassembler {
 public:
  ZydisDisassembler(MachineType);

  void SetUp(MachineType);
  void Teardown();

  // Ask the engine to process given data.
  bool Process(const u8* data, size_t len, u64 vaddress_base, size_t subset);

  // TODO: Accessors for the specific instances.
 private:
  ZydisDecoder decoder_{};
  ZydisDecodedInstruction instruction_{};
  ZydisDecodedOperand operands_[ZYDIS_MAX_OPERAND_COUNT_VISIBLE]{}; 
};
}  // namespace decompiler