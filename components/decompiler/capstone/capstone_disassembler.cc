// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Simple cxx wrapper around a capstone instance.

#include <base/check.h>
#include <base/logging.h>

#include <capstone/capstone.h>
#include "decompiler/capstone/capstone_disassembler.h"

namespace decompiler {

namespace {
const char* const CsErrorToString(cs_err err) {
  switch (err) {
    case CS_ERR_OK:
      return "OK";
    case CS_ERR_MEM:
      return "Out-Of-Memory error: cs_open(), cs_disasm(),"
             "cs_disasm_iter() ";
    case CS_ERR_ARCH:
      return "Unsupported architecture: cs_open()";
    case CS_ERR_HANDLE:
      return "Invalid handle: cs_op_count(), cs_op_index()";
    case CS_ERR_CSH:
      return "Invalid csh argument: cs_close(), cs_errno(),"
             "cs_option()";
    case CS_ERR_MODE:
      return "Invalid/unsupported mode: cs_open()";
    case CS_ERR_OPTION:
      return "Invalid/unsupported option: cs_option()";
    case CS_ERR_DETAIL:
      return "Information is unavailable because detail option is OFF";
    case CS_ERR_MEMSETUP:
      return "Dynamic memory management uninitialized (see CS_OPT_MEM)";
    case CS_ERR_VERSION:
      return "Unsupported version (bindings)";
    case CS_ERR_DIET:
      return "Access irrelevant data in \"diet\" engine";
    case CS_ERR_SKIPDATA:
      return "Access irrelevant data for \"data\" instruction in SKIPDATA mode";
    case CS_ERR_X86_ATT:
      return "X86 AT&T syntax is unsupported (opt-out at compile time)";
    case CS_ERR_X86_INTEL:
      return "X86 Intel syntax is unsupported (opt-out at compile time)";
    case CS_ERR_X86_MASM:
      return "X86 Masm syntax is unsupported (opt-out at compile time)";
    default:
      IMPOSSIBLE;
      return "";
  }
}

cs_arch TranslateMachineType(MachineType mt) {
  switch (mt) {
    case MachineType::kx86_64:
    case MachineType::kx86:
      return CS_ARCH_X86;
    case MachineType::kArm:
      return CS_ARCH_ARM;
    case MachineType::kArm64:
      return CS_ARCH_ARM64;
    default:
      IMPOSSIBLE;
      return CS_ARCH_ALL;
  }
}
}  // namespace

CapstoneDissasembler::CapstoneDissasembler(MachineType mt) {
  SetUp(mt);
}

CapstoneDissasembler::~CapstoneDissasembler() {
  Teardown();
}

bool CapstoneDissasembler::SetUp(MachineType type) {
  const cs_arch arch = TranslateMachineType(type);
  const cs_err err = cs_open(arch, CS_MODE_64, &handle_);
  if (err != CS_ERR_OK) {
    LOG_ERROR("Capstone error: {}", CsErrorToString(err));
    return false;
  }

  // enable detailled instructions
  cs_option(handle_, CS_OPT_DETAIL, CS_OPT_ON);
  return false;
}

void CapstoneDissasembler::Teardown() {
  if (handle_) {
    cs_free(insn_, di_count_);
    cs_close(&handle_);
  }
}

bool CapstoneDissasembler::Process(const base::Span<byte> data,
                                   u64 vaddress_base,
                                   size_t subset) {
  di_count_ =
      cs_disasm(handle_, data.data(), data.length(), vaddress_base, subset, &insn_);

  if (di_count_ > 0)
    return true;

  LOG_ERROR("Capstone failed to disassemble");
  return false;
}
}  // namespace decompiler