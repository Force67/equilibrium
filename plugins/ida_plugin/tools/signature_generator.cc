// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"

#include <QVector>
#include <array>
#include "signature_generator.h"

namespace tools {
namespace {
// how many instructions the generator will try to build patterns
constexpr int kInstructionLimit = 100;

class SignatureGenerator {
 public:
  enum class Result {
    kSuccess,
    kDecodeError,
    kLengthExceeded,
    kEmpty,
  };

  static const char* const ResultToString(Result) noexcept;

  Result Generate() { return Result::kSuccess; }

  std::string ToString() { return ""; }

 private:
  qvector<uchar> bytes_;
  qvector<uchar> mask_;
  size_t trimmed_size_;
};

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

SignatureGenerator::Result GenerateSignatureInternal(ea_t address,
                                                     std::string& out_pattern) {
  qvector<uchar> bytes;
  qvector<uchar> mask;

  ea_t current_address = address;

  // Analyze up to k bytes.
  for (int i = 0; i < kInstructionLimit; i++) {
    insn_t instruction{};
    if (decode_insn(&instruction, current_address) == 0) {
      LOG_ERROR("Failed to decode isntruction at {:x}", current_address);
      return SignatureGenerator::Result::kDecodeError;
    }

    // Add the bytes of the instruction to the bytes of our pattern.
    for (int j = 0; j < instruction.size; ++j) {
      const uchar byte = get_byte(instruction.ea + j);
      bytes.add(byte);
    }

    // todo: yeet
    QVector<uchar> instructionMask(instruction.size, 1);

    for (const auto& op : instruction.ops) {
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
              std::back_inserter(mask));

    // Search from the min to the most address
    if (bin_search2(inf_get_min_ea(), address, bytes.begin(), mask.begin(),
                    bytes.size(), BIN_SEARCH_FORWARD) == BADADDR) {
      // Search from the address we're making the pattern for + current
      // pattern size to the max-most address.
      if (bin_search2(address + bytes.size(), inf_get_max_ea(), bytes.begin(),
                      mask.begin(), bytes.size(),
                      BIN_SEARCH_FORWARD) == BADADDR) {
        break;
      }
    }

    // continue seeking
    current_address += instruction.size;
  }

  for (int i = 0; i < bytes.size(); ++i) {
    if (mask[i] == 1) {
      std::array<char, 3> byte{};

      sprintf_s(byte.data(), byte.size(), "%02X", bytes[i]);

      out_pattern += byte.data();
      out_pattern += " ";
    } else {
      out_pattern += "? ";
    }
  }

  if (out_pattern.empty()) {
    return SignatureGenerator::Result::kEmpty;
  }

  if (out_pattern.length() > 256) {
    return SignatureGenerator::Result::kLengthExceeded;
  }

  out_pattern = out_pattern.substr(0, out_pattern.length() - 1);
  return SignatureGenerator::Result::kSuccess;
}
}  // namespace

void GenerateSignature(ea_t target_address) {
  const flags_t flags = get_flags(target_address);

  SignatureGenerator::Result result{SignatureGenerator::Result::kSuccess};
  insn_t instruction{};

  // if the user wants to generate a pattern towards a data type
  // we allow that by searching through xrefs and giving ref address
  // + displacement
  if (is_data(flags)) {
    LOG_TRACE("Creating data signature for {:x} ...", target_address);

    // iterate through references to data.
    for (auto ref_address = get_first_dref_to(target_address);
         ref_address != BADADDR;
         ref_address = get_next_dref_to(target_address, ref_address)) {
      if (target_address == ref_address)
        continue;

      if (decode_insn(&instruction, ref_address) == 0)
        continue;

      // try to generate, if not, go to the next reference
      std::string pattern;
      if ((result = GenerateSignatureInternal(ref_address, pattern)) ==
          SignatureGenerator::Result::kSuccess) {
        if (instruction.size > 0)
          LOG_INFO("Data signature: {:x} = {} + {}", ref_address, pattern,
                   instruction.size);
        else
          LOG_INFO("Data signature: {:x} = {}", ref_address, pattern);
        return;
      }
    }

    LOG_ERROR("Failed to generate reference to data {:x} with error: {}",
              target_address, SignatureGenerator::ResultToString(result));
  } else if (is_code(flags)) {
    LOG_TRACE("Creating code signature for {:x} ...", target_address);

    if (!can_decode(target_address)) {
      LOG_ERROR("Please select a valid address");
      return;
    }

    std::string pattern;
    if ((result = GenerateSignatureInternal(target_address, pattern)) ==
        SignatureGenerator::Result::kSuccess) {
      LOG_INFO("Code signature: {:x} = {}", target_address, pattern);
      return;
    }

    // iterate through references to code.
    for (auto ref_address = get_first_cref_to(target_address);
         ref_address != BADADDR;
         ref_address = get_next_cref_to(target_address, ref_address)) {
      if (target_address == ref_address)
        continue;

      if (decode_insn(&instruction, ref_address) == 0)
        continue;

      if ((result = GenerateSignatureInternal(ref_address, pattern)) ==
          SignatureGenerator::Result::kSuccess) {
        if (instruction.size > 0)
          LOG_INFO("Code signature: {:x} = {} + {}", ref_address, pattern,
                   instruction.size);
        else
          LOG_INFO("Code signature: {:x} = {}", ref_address, pattern);
        return;
      }
    }

    LOG_ERROR("Failed to generate reference to code {:x} with error: {}",
              target_address, SignatureGenerator::ResultToString(result));
  }
}
}  // namespace tools