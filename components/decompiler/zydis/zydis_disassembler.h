// Copyright (C) 2022 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Simple cxx wrapper around a zydis instance.
#pragma once

#include <zydis/Decoder.h>
#include <decompiler/disassembler.h>

namespace decompiler {

class ZydisDisassembler final : public Disassembler {
 public:
  ZydisDisassembler(MachineType);

  bool SetUp(MachineType) override;
  void Teardown() override;

  // Ask the engine to process given data.
  bool Process(const base::Span<byte> data, u64 vaddress_base, size_t subset) override;

  // TODO: Accessors for the specific instances.
 private:
  ZydisDecoder decoder_{};
  ZydisDecodedInstruction instruction_{};
  ZydisDecodedOperand operands_[ZYDIS_MAX_OPERAND_COUNT_VISIBLE]{};
};
}  // namespace decompiler