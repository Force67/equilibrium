// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"

#include <QVector>
#include <bytes.hpp>
#include <array>
#include "signature_generator.h"

// too tired to fix this
#ifndef MS_CLS
#define MS_CLS 0x00000600LU   ///< Mask for typing
#define FF_CODE 0x00000600LU  ///< Code ?
#define FF_DATA 0x00000400LU  ///< Data ?
#define FF_TAIL 0x00000200LU  ///< Tail ?
#define FF_UNK 0x00000000LU   ///< Unknown ?
//@}
#endif

namespace tools {
namespace {
// how many instructions the generator will try to build patterns
constexpr size_t kInstructionLimit = 100;
// originally 256, but thats a bit extreme.
constexpr size_t kSignatureLengthLimit = 70;

uint8_t GetOpCodeSize(const insn_t& instruction) {
  for (unsigned int i = 0; i < UA_MAXOP; i++) {
    if (instruction.ops[i].type == o_void)
      return 0;
    if (instruction.ops[i].offb != 0)
      return static_cast<uint8_t>(instruction.ops[i].offb);
  }
  return 0;
}

}  // namespace

SignatureGenerator::SignatureGenerator(Toolbox* toolbox) : toolbox_(toolbox) {}

const char* const SignatureGenerator::ResultToString(Result result) noexcept {
  switch (result) {
    case Result::kSuccess:
      return "Success";
    case Result::kDecodeError:
      return "Failed to decode";
    case Result::kLengthExceeded:
      return "Length exceeded";
    case Result::kEmpty:
      return "Empty";
    default:
      // TK_IMPOSSIBLE;
      return "<unknown>";
  }
}



SignatureGenerator::Result SignatureGenerator::GenerateSignatureInternal_2(
    ea_t address,
    std::string& out_pattern) {
  bytes_.clear();
  masks_.clear();

  ea_t current_address = address;
  out_pattern = "";

  // Analyze up to k bytes.
  for (int i = 0; i < kInstructionLimit; i++) {
    insn_t instruction{};
    if (decode_insn(&instruction, current_address) == 0) {
      return SignatureGenerator::Result::kDecodeError;
    }

    // Add the bytes of the instruction to the bytes of our pattern.
    for (int j = 0; j < instruction.size; ++j) {
      const uchar byte = get_byte(instruction.ea + j);
      bytes_.add(byte);
    }

    // todo: yeet
    QVector<uchar> instructionMask(instruction.size, 1);

    for (const op_t& op : instruction.ops) {
      if (op.type == o_void) {
        break;
      }

      // If the operand isn't one of these types, fill the remaining portion
      // of the mask with 0.
      if (op.type != o_reg && op.type != o_phrase) {
        for (int k = op.offb; k < instruction.size; ++k) {
          instructionMask[k] = 0;
        }

        break;
      }
    }

    // Copy this instructions mask to our pattern's mask.
    std::copy(instructionMask.begin(), instructionMask.end(),
              std::back_inserter(masks_));

    // optimization opportunity: bin search only within code seg.
    //
    // Search from the min to the most address
    if (bin_search2(inf_get_min_ea(), address, bytes_.begin(), masks_.begin(),
                    bytes_.size(), BIN_SEARCH_FORWARD) == BADADDR) {
      // Search from the address we're making the pattern for + current
      // pattern size to the max-most address.
      if (bin_search2(address + bytes_.size(), inf_get_max_ea(), bytes_.begin(),
                      masks_.begin(), bytes_.size(),
                      BIN_SEARCH_FORWARD) == BADADDR) {
        break;
      }
    }

    // continue seeking
    current_address += instruction.size;
  }

  if (bytes_.empty() || masks_.empty())
    return Result::kEmpty;

  constexpr char kHexChars[] = "0123456789ABCDEF";
  for (size_t i = 0; i < bytes_.size(); ++i) {
    if (i)
      out_pattern += ' ';

    const uchar mask = masks_[i];
    const uchar value = bytes_[i];

    if (mask != 0x00) {
      out_pattern += kHexChars[static_cast<std::size_t>(value >> 4)];
      out_pattern += kHexChars[static_cast<std::size_t>(value & 0xF)];
    } else {
      out_pattern += '?';
    }
  }

  // chop off remaining spaces or question marks
  // this still saves us time in the long run, as we might not
  // have to rerun the pattern to get a shorter pattern
  for (size_t i = out_pattern.size(); i-- != 0;) {
    if (out_pattern[i] != ' ' && out_pattern[i] != '?') {
      out_pattern.resize(i + 1);
      break;
    }
  }

  if (out_pattern.length() > kSignatureLengthLimit) {
    return SignatureGenerator::Result::kLengthExceeded;
  }

  return SignatureGenerator::Result::kSuccess;
}

SignatureGenerator::Result SignatureGenerator::UniqueDataPattern(
    ea_t target_address,
    std::string& out_pattern,
    size_t& out_offset) {
  insn_t instruction{};
  Result result{Result::kEmpty};
  out_pattern = "";

  // iterate through references to data.
  for (auto ref_address = get_first_dref_to(target_address);
       ref_address != BADADDR;
       ref_address = get_next_dref_to(target_address, ref_address)) {
    if (target_address == ref_address)
      continue;

    // we only care about function references. go to the start of the function
    // and make a pattern from there.
    if (const func_t* func = get_func(ref_address)) {
      if ((result = GenerateSignatureInternal_2(func->start_ea, out_pattern)) ==
          SignatureGenerator::Result::kSuccess) {
        if (decode_insn(&instruction, ref_address) > 0 &&
            instruction.size > 0) {
          out_offset =
              (ref_address - func->start_ea) + GetOpCodeSize(instruction);
        }

        return result;
      }
    }
  }

  return result;
}

SignatureGenerator::Result SignatureGenerator::UniqueCodePattern(
    ea_t target_address,
    std::string& out_pattern,
    size_t& out_offset) {
  insn_t instruction{};
  Result result{Result::kEmpty};

  if (!can_decode(target_address)) {
    return Result::kDecodeError;
  }

  // try the direct way first.
  if ((result = GenerateSignatureInternal_2(target_address, out_pattern)) ==
      SignatureGenerator::Result::kSuccess) {
    return result;
  }

  // if locating fails for the direct function body, we start iterating the
  // references towards the function
  for (auto ref_address = get_first_cref_to(target_address);
       ref_address != BADADDR;
       ref_address = get_next_cref_to(target_address, ref_address)) {
    if (target_address == ref_address)
      continue;

    // go to the start of the function calling our function
    // and make a pattern from there.
    if (const func_t* func = get_func(ref_address)) {
      if ((result = GenerateSignatureInternal_2(func->start_ea, out_pattern)) ==
          SignatureGenerator::Result::kSuccess) {
        if (decode_insn(&instruction, ref_address) > 0 &&
            instruction.size > 0) {
          out_offset =
              (ref_address - func->start_ea) + GetOpCodeSize(instruction);
        }

        return result;
      }
    }
  }

  return result;
}

std::string SignatureGenerator::UniquePattern(ea_t target_address,
                                              bool mute_log) {
  uint32_t flags = (get_flags(target_address) & MS_CLS);
  // disallow unsupported address types so it can only be code or data
  // going forward
  switch (flags) {
    case FF_TAIL:
    case FF_UNK: {
      LOG_ERROR("Unsupported address flag {}", flags);
      return "";
    }
    default:
      break;
  }

  const bool is_address_data = flags == FF_DATA;
  const char* type_name = is_address_data ? "data" : "code";

  if (!mute_log) {
    LOG_TRACE("Creating {} signature for {:x}", type_name, target_address);
  }

  std::string result_pattern;
  size_t result_offset = 0;

  const Result result =
      is_address_data
          ? UniqueDataPattern(target_address, result_pattern, result_offset)
          : UniqueCodePattern(target_address, result_pattern, result_offset);

  if (result == Result::kSuccess) {
    if (!mute_log) {
      if (result_offset > 0) {
        LOG_INFO("{} signature: {:x} = {} + {}", type_name, target_address,
                 result_pattern, result_offset);
      } else {
        LOG_INFO("{} signature: {:x} = {}", type_name, target_address,
                 result_pattern);
      }
    }

    return result_pattern;
  }

  if (!mute_log) {
    LOG_ERROR("Failed to generate reference to {} {:x} with error: {}",
              type_name, target_address, ResultToString(result));
  }

  return "";
}
}  // namespace tools