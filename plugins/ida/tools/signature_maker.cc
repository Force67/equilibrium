
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"

#include <QVector>
#include <bytes.hpp>
#include <array>
#include "signature_maker.h"

#include <chrono>

namespace cn = std::chrono;

struct ScopedProfile {
  cn::high_resolution_clock::time_point delta;
  const char* const name;

  ScopedProfile(const char* const name)
      : name(name), delta(cn::high_resolution_clock::now()) {}

  ~ScopedProfile(void) {
    auto now = std::chrono::high_resolution_clock::now();
    auto nanos =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - delta)
            .count();
    auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(now - delta).count();

    LOG_INFO("~ScopedTimer() : <{}> : {} ms ({} s)", name, nanos, seconds);
  }
};

// too tired to fix this
#ifndef MS_CLS
#define MS_CLS 0x00000600LU   ///< Mask for typing
#define FF_CODE 0x00000600LU  ///< Code ?
#define FF_DATA 0x00000400LU  ///< Data ?
#define FF_TAIL 0x00000200LU  ///< Tail ?
#define FF_UNK 0x00000000LU   ///< Unknown ?
//@}
#endif

namespace {
// how many instructions the generator can try to use to build a pattern
constexpr size_t kInstructionLimit = 100u;
// max length of a signature, including spaces and question marks
constexpr size_t kSignatureMaxLength = 75u;
// min length of a signature in bytes
constexpr size_t kMinSignatureByteLength = 5u;
// max displacement size in bytes
constexpr size_t kDisplacementStepSize = 100u;
// max reference count to be considered this is done to limit at
// runtime where it scales up pretty badly if we have lots of references :(
constexpr size_t kMaxRefCountAnalysisDepth = 10u;

// Does this even make sense?
static_assert(kDisplacementStepSize <= INT8_MAX,
              "Displacement bounds exceeded");

uint8_t GetOpCodeSize(const insn_t& instruction) {
  for (uint32_t i = 0; i < UA_MAXOP; i++) {
    if (instruction.ops[i].type == o_void)
      return 0;
    if (instruction.ops[i].offb != 0)
      return static_cast<uint8_t>(instruction.ops[i].offb);
  }
  return 0;
}
}  // namespace

SignatureMaker::SignatureMaker() = default;

const char* const SignatureMaker::ResultToString(Result result) noexcept {
  switch (result) {
    case Result::kSuccess:
      return "Success";
    case Result::kDecodeError:
      return "Failed to decode";
    case Result::kLengthExceeded:
      return "Length exceeded";
    case Result::kNoReferences:
      return "No references found";
    case Result::kRefLimitReached:
      return "Ref Limit reached";
    case Result::kEmpty:
      return "Empty";
    default:
      // TK_IMPOSSIBLE;
      return "<unknown>";
  }
}

SignatureMaker::Result SignatureMaker::GenerateSignatureInternal_2(
    ea_t address,
    std::string& out_pattern) {
  ScopedProfile profile_frame("GenerateSignatureInternal");
  (void)profile_frame;

  bytes_.clear();
  masks_.clear();
  out_pattern.clear();

  ea_t current_address = address;
  bool done_flag = false;

  // Analyze up to k bytes.
  for (size_t i = 0; i < kInstructionLimit; i++) {
    insn_t instruction{};
    if (decode_insn(&instruction, current_address) == 0) {
      return Result::kDecodeError;
    }

    // Add the bytes of the instruction to the bytes of our pattern.
    for (uint16_t j = 0; j < instruction.size; ++j) {
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
    // get_ph -> this gets the buffer!
    // pack_dd();
    // pack_dd
    // show_auto -> allows setting IDB BUSY
    // search only within code.
    // get_byte
    /*
    *      v21 = getinf(57i64);
            v22 = get_compiler_name(v21);
            v23 = get_compiler_name(*((_BYTE *)v14 + 36));
    */

    for (const segment_t* seg = get_first_seg(); seg != nullptr;
         seg = get_next_seg(seg->start_ea)) {
      // patterns may only exist within code segments.
      if (seg->type != SEG_CODE)
        continue;
      // search from the segment start to address.
      if (bin_search2(seg->start_ea, address, bytes_.begin(), masks_.begin(),
                      bytes_.size(),
                      seg->start_ea <= address
                          ? BIN_SEARCH_FORWARD
                          : BIN_SEARCH_BACKWARD) == BADADDR) {
        // Search from the address we're making the pattern for + current
        // pattern size to the max-most address.
        const ea_t start = address + bytes_.size();
        if (bin_search2(start, seg->end_ea, bytes_.begin(), masks_.begin(),
                        bytes_.size(),
                        start <= seg->end_ea
                            ? BIN_SEARCH_FORWARD
                            : BIN_SEARCH_BACKWARD) == BADADDR) {
          done_flag = true;
          break;
        }
      }
    }

    if (done_flag)
      break;

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
      out_pattern += kHexChars[static_cast<size_t>(value >> 4)];
      out_pattern += kHexChars[static_cast<size_t>(value & 0xF)];
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

  if (out_pattern.length() > kSignatureMaxLength) {
    return Result::kLengthExceeded;
  }

  return Result::kSuccess;
}

SignatureMaker::Result SignatureMaker::GenerateFunctionReference(
    ea_t ref_ea,
    std::string& out_pattern,
    ptrdiff_t& out_offset) {
  Result result{Result::kEmpty};
  insn_t instruction{};
  // jump to start of the function and search from there
  if (const func_t* func = get_func(ref_ea)) {
    if (decode_insn(&instruction, ref_ea) == 0)
      return Result::kDecodeError;

    out_offset = GetOpCodeSize(instruction);
    // shitty optimization #1: if delta is low, skip the rest
    // TODO: Is this optimization ever triggered????
    if ((ref_ea - kDisplacementStepSize) <= kDisplacementStepSize) {
      if ((result = GenerateSignatureInternal_2(func->start_ea, out_pattern)) ==
          Result::kSuccess) {
        // again, 10/10 code.
        out_offset += (ref_ea - func->start_ea);
        return result;
      }
    }

    ea_t ea_step = ref_ea - kDisplacementStepSize;
    // search downwards, to reduce displacement we go from ref_ea.
    while (ea_step >= func->start_ea) {
      if ((result = GenerateSignatureInternal_2(ea_step, out_pattern)) ==
          Result::kSuccess) {
        // again, 10/10 code.
        out_offset += (ref_ea - ea_step);
        return result;
      }
      ea_step -= kDisplacementStepSize;
    }
    // now upwards.
    ea_step = ref_ea;
    while (ea_step <= func->end_ea) {
      if ((result = GenerateSignatureInternal_2(ea_step, out_pattern)) ==
          Result::kSuccess) {
        // need to subtract offset.
        out_offset -= (ea_step - ref_ea);
        return result;
      }
      ea_step += kDisplacementStepSize;
    }

    // fallback case: maximum displacement, very bad!
    if (result != Result::kSuccess) {
      result = GenerateSignatureInternal_2(func->start_ea, out_pattern);
      // again, 10/10 code.
      out_offset += (ref_ea - func->start_ea);
      return result;
    }
  }

  return result;
}

SignatureMaker::Result SignatureMaker::UniqueDataSignature(
    ea_t target_ea,
    std::string& out_pattern,
    ptrdiff_t& out_offset) {
  Result result{Result::kNoReferences};
  // iterate through references to data.

  // ordered map based by function size
  // sorted by key, which is our function size
  std::map<ea_t, ea_t> refs;
  for (auto ref_ea = get_first_dref_to(target_ea); ref_ea != BADADDR;
       ref_ea = get_next_dref_to(target_ea, ref_ea)) {
    if (target_ea == ref_ea)
      continue;

    if (const func_t* func = get_func(ref_ea)) {
      refs.insert(std::make_pair(func->end_ea - func->start_ea, ref_ea));
    }
  }

  LOG_INFO("DREF COUNT: {}", refs.size());

  int counter = 0;
  for (const auto& pair : refs) {
    if (counter > kMaxRefCountAnalysisDepth)
      return Result::kRefLimitReached;

    LOG_TRACE("Dref: {:x} {}/{}", pair.second, counter, refs.size());
    result = GenerateFunctionReference(pair.second, out_pattern, out_offset);
    if (result == Result::kSuccess)
      return result;

    counter++;
  }

  return result;
}

SignatureMaker::Result SignatureMaker::UniqueCodeSignature(
    ea_t target_ea,
    std::string& out_pattern,
    ptrdiff_t& out_offset,
    bool& very_dumb_flag) {
  // valid code?
  if (!can_decode(target_ea)) {
    return Result::kDecodeError;
  }

  // try the direct way first.
  Result result{Result::kNoReferences};
  if ((result = GenerateSignatureInternal_2(target_ea, out_pattern)) ==
      Result::kSuccess) {
    out_offset = 0;
    // direct:
    very_dumb_flag = false;
    return result;
  }

  very_dumb_flag = true;

  // ordered map based by function size
  // sorted by key, which is our function size
  std::map<ea_t, ea_t> refs;
  for (auto ref_ea = get_first_cref_to(target_ea); ref_ea != BADADDR;
       ref_ea = get_next_cref_to(target_ea, ref_ea)) {
    if (target_ea == ref_ea)
      continue;

    if (const func_t* func = get_func(ref_ea)) {
      refs.insert(std::make_pair(func->end_ea - func->start_ea, ref_ea));
    }
  }

  LOG_INFO("CREF COUNT: {}", refs.size());

  int counter = 0;
  for (const auto& pair : refs) {
    // temp shit, we only look at the kMaxRefCountAnalysisDepth best samples
    if (counter > kMaxRefCountAnalysisDepth)
      return Result::kRefLimitReached;

    LOG_TRACE("Cref: {:x} {}/{}", pair.second, counter, refs.size());
    result = GenerateFunctionReference(pair.second, out_pattern, out_offset);
    if (result == Result::kSuccess)
      return result;

    counter++;
  }

  return result;
}

SignatureMaker::Result SignatureMaker::CreateUniqueSignature(
    const ea_t in_ea,
    std::string& out_pattern,
    ptrdiff_t& out_offset,
    bool mute_log,
    bool& is_data) {
  // disallow unsupported address types so it can only be code or data
  // going forward
  uint32_t flags = (get_flags(in_ea) & MS_CLS);
  switch (flags) {
    case FF_TAIL:
    case FF_UNK: {
      LOG_ERROR("Unsupported address flag {}", flags);
      return Result::kDecodeError;
    }
    default:
      break;
  }

  const bool is_address_data = flags == FF_DATA;
  const char* type_name = is_address_data ? "data" : "code";

  is_data = is_address_data;

  if (!mute_log) {
    LOG_TRACE("Creating {} signature for {:x}", type_name, in_ea);
  }

  std::string result_pattern{};
  ptrdiff_t result_offset = 0;

  const Result result =
      is_address_data
          ? UniqueDataSignature(in_ea, result_pattern, result_offset)
          : UniqueCodeSignature(in_ea, result_pattern, result_offset, is_data);

  if (result == Result::kSuccess) {
    if (!mute_log) {
      if (result_offset > 0) {
        LOG_INFO("{} signature: {:x} = {} + {}", type_name, in_ea,
                 result_pattern, result_offset);
      } else {
        LOG_INFO("{} signature: {:x} = {}", type_name, in_ea, result_pattern);
      }
    }

    out_pattern = std::move(result_pattern);
    out_offset = result_offset;
    return result;
  }

  if (!mute_log) {
    LOG_ERROR("Failed to generate reference to {} {:x} with error: {}",
              type_name, in_ea, ResultToString(result));
  }

  out_pattern = "";
  out_offset = 0;

  return Result::kEmpty;
}