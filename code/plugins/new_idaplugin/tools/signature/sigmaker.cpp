// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Sigmaker.h"
#include "Pch.h"
#include "utils/Logger.h"

#include <QVector>
#include <array>

namespace features {

namespace {
constexpr int kInstructionLimit = 100;
}

void GenerateSignature() {
  const ea_t address = get_screen_ea();

  if (!can_decode(address)) {
    LOG_ERROR("Please select a valid address");
    return;
  }

  LOG_TRACE("Creating Signature for {:x} ...", address);

  QVector<uchar> bytes;
  QVector<uchar> mask;

  ea_t current_address = address;

  // Analyze up to k bytes.
  for (int i = 0; i < kInstructionLimit; i++) {
    insn_t instruction{};
    if (decode_insn(&instruction, current_address) == 0) {
      LOG_ERROR("Failed to decode isntruction at {:x}", current_address);
      return;
    }

    // Add the bytes of the instruction to the bytes of our pattern.
    for (int j = 0; j < instruction.size; ++j) {
      const uchar byte = get_byte(instruction.ea + j);
      bytes.append(byte);
    }

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
    if (bin_search2(inf.min_ea, address, bytes.data(), mask.data(),
                    bytes.size(), BIN_SEARCH_FORWARD) == BADADDR) {
      // Search from the address we're making the pattern for + current pattern
      // size to the max-most address.
      if (bin_search2(address + bytes.size(), inf.max_ea, bytes.data(),
                      mask.data(), bytes.size(),
                      BIN_SEARCH_FORWARD) == BADADDR) {
        break;
      }
    }

    // continue seeking
    current_address += instruction.size;
  }

  // Create a pattern from the bytes and mask.
  std::string pat{};

  for (int i = 0; i < bytes.size(); ++i) {
    if (mask[i] == 1) {
      std::array<char, 3> byte{};

      sprintf_s(byte.data(), byte.size(), "%02X", bytes[i]);

      pat += byte.data();
      pat += " ";
    } else {
      pat += "? ";
    }
  }

  if (pat.empty()) {
    LOG_ERROR("Failed to create pattern");
    return;
  }

  if (pat.length() > 256) {
    LOG_ERROR("Pattern too long");
    return;
  }

  pat = pat.substr(0, pat.length() - 1);
  LOG_INFO("Generated Pattern: {}", pat);
}
}  // namespace features