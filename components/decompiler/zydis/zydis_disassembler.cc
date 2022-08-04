// Copyright (C) 2022 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Simple cxx wrapper around a zydis instance.

#include <base/check.h>
#include <base/logging.h>

#include <decompiler/zydis/zydis_disassembler.h>

namespace decompiler {
struct TargetInfo {
  ZydisMachineMode mode;
  ZydisStackWidth stack_width;
};

static TargetInfo kMachineMap[] = {
    {ZydisMachineMode::ZYDIS_MACHINE_MODE_LONG_COMPAT_32,
     ZydisStackWidth::ZYDIS_STACK_WIDTH_32},
    {ZydisMachineMode::ZYDIS_MACHINE_MODE_LONG_64,
     ZydisStackWidth::ZYDIS_STACK_WIDTH_64}};

static TargetInfo* GetZydisTargetInfo(MachineType mt) {
  // In order to satisfy intellisense...
  if (mt == MachineType::kx86 || mt == MachineType::kx86_64)
    return &kMachineMap[static_cast<size_t>(mt)];
  return nullptr;
}

ZydisDisassembler::ZydisDisassembler(MachineType mt) {
  SetUp(mt);
}

bool ZydisDisassembler::SetUp(MachineType mt) {
  if (TargetInfo* info = GetZydisTargetInfo(mt)) {
    const auto result = ZydisDecoderInit(&decoder_, info->mode, info->stack_width);
    return ZYAN_SUCCESS(result);
  }

  BUGCHECK(false);
  return false;
}

void ZydisDisassembler::Teardown() {
  // no-op
}

bool ZydisDisassembler::Process(const base::Span<byte> data,
                                u64 vaddress_base,
                                size_t subset) {
  return ZYAN_SUCCESS(ZydisDecoderDecodeFull(
      &decoder_, static_cast<const void*>(data.data()), data.length(), &instruction_,
      operands_, ZYDIS_MAX_OPERAND_COUNT_VISIBLE,
      ZYDIS_DFLAG_VISIBLE_OPERANDS_ONLY));
}

}  // namespace decompiler