
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// IDA implementation of the signature generator.
// 
// Warning: Please consider always running this in release mode unless needed,
//          as it's very compute intensive.

#include "Pch.h"
#include <bytes.hpp>

#include "pattern_provider.h"
#include "pattern_provider_config.h"

#include <base/check.h>
#include <base/scoped_profile.h>

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

uint8_t GetOpCodeSize(const insn_t& instruction) {
  for (uint32_t i = 0; i < UA_MAXOP; i++) {
    if (instruction.ops[i].type == o_void)
      return 0;
    if (instruction.ops[i].offb != 0)
      return static_cast<uint8_t>(instruction.ops[i].offb);
  }
  return 0;
}

uint32_t GetAddressFlags(const ea_t ea) {
  return (get_flags(ea) & MS_CLS);
}

bool CheckAddressTypeAllowed(const ea_t ea) {
  uint32_t flags = (get_flags(ea) & MS_CLS);
  switch (flags) {
    case FF_TAIL:
    case FF_UNK: {
      return false;
    }
    default:
      return true;
  }
}

bool ValidateWildcardCount(const std::string& pattern, bool with_spacing) {
  return true;

  size_t count = 0;
  size_t i = with_spacing ? 3 : 1;
  size_t offset = with_spacing ? 2 : 1;

  for (; i < pattern.length(); i++) {
    // count patterns in a row
    if (pattern[i] == '?' && pattern[i - offset] == '?') {
      count++;

      // We only allow 4 patterns in a row e.g
      // E9 ? ? ? ?
      if (count > 3)
        return false;
    }
    if (pattern[i] != '?' && pattern[i - offset] == '?') {
      count = 0;
    }
  }

  return true;
}
}  // namespace

PatternProvider::PatternProvider() = default;

const char* const PatternProvider::TranslateStatus(Status result) noexcept {
  switch (result) {
    case Status::kSuccess:
      return "Success";
    case Status::kDecodeError:
      return "Failed to decode";
    case Status::kLengthExceeded:
      return "Length exceeded";
    case Status::kNoReferences:
      return "No references found";
    case Status::kRefLimitReached:
      return "Ref Limit reached";
    case Status::kEmpty:
      return "Empty";
    default:
      TK_IMPOSSIBLE;
      return "<unknown>";
  }
}

PatternProvider::Status PatternProvider::ComposeSignature(
    const ea_t in_address,
    std::string& out_pattern) {
  // enter timed scope
  base::ScopedProfile profile_frame("ComposeSignature");
  (void)profile_frame;

  std::vector<uchar> bytes;
  std::vector<uchar> masks;

  ea_t current_ea = in_address;
  bool done_flag = false;

  // Analyze up to k bytes.
  for (size_t i = 0; i < pattern_config::kInstructionLimit; i++) {
      // gather information about the current info
    insn_t instruction{};
    if (decode_insn(&instruction, current_ea) == 0) {
      return Status::kDecodeError;
    }

    // Add the bytes of the instruction to the bytes of our pattern.
    for (uint16_t j = 0; j < instruction.size; ++j) {
      const uchar byte = get_byte(instruction.ea + j);
      bytes.push_back(byte);
    }

    std::vector<uchar> instructionMask(instruction.size, 1);

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
              std::back_inserter(masks));

    for (const segment_t* seg = get_first_seg(); seg != nullptr;
         seg = get_next_seg(seg->start_ea)) {
      // patterns may only exist within code segments.
      if (seg->type != SEG_CODE)
        continue;
      // search from the segment start to address.
      if (bin_search2(seg->start_ea, in_address, bytes.data(), masks.data(),
                      bytes.size(),
                      seg->start_ea <= in_address
                          ? BIN_SEARCH_FORWARD
                          : BIN_SEARCH_BACKWARD) == BADADDR) {
        // Search from the address we're making the pattern for + current
        // pattern size to the max-most address.
        const ea_t start = in_address + bytes.size();
        if (bin_search2(start, seg->end_ea, bytes.data(), masks.data(),
                        bytes.size(),
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
    current_ea += instruction.size;
  }

  if (bytes.empty() || masks.empty())
    return Status::kEmpty;

  constexpr char kHexChars[] = "0123456789ABCDEF";
  for (size_t i = 0; i < bytes.size(); ++i) {
    if (i)
      out_pattern += ' ';

    const uchar mask = masks[i];
    const uchar value = bytes[i];

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

  if (out_pattern.length() > pattern_config::kSignatureMaxLength ||
      // patterns must not contain excessive spacing
      // since too many wild-cards will inevitably lead to breakage
      // if ABI changes occur
      !ValidateWildcardCount(out_pattern, true)) {
    return Status::kLengthExceeded;
  }

  return Status::kSuccess;
}

PatternProvider::Status PatternProvider::CreateSignature(
    const ea_t target_address,
    diffing::Pattern& out) {
  // Must be within code
  if (!CheckAddressTypeAllowed(target_address))
    return Status::kUnsupportedType;

  out.is_data = GetAddressFlags(target_address) == FF_DATA;
  out.type = diffing::Pattern::ReferenceType::kRef3;

  const Status s =
      is_data ? UniqueDataSignature(in_ea, out_pattern, out_offset)
              : UniqueCodeSignature(in_ea, out_pattern, out_offset, is_data,
                                    ref_type);

  return result;
}

bool PatternProvider::CreatePrintSignature(const ea_t ea) {
  LOG_TRACE("Creating signature for {:x}", ea);

  diffing::Pattern pattern{};
  const Status s = CreateSignature(ea, pattern);

  // pretty format the name data
  const char* type_name = pattern.is_data ? "data" : "code";
  const char* ref_name =
      pattern.type == diffing::Pattern::ReferenceType::kDirect ? "direct"
                                                               : "reference";

  if (s == Status::kSuccess) {
    // offsets may also be negative
    if (pattern.function_offset != 0) {
      LOG_INFO("{} {} signature: {:x} = {}@{}", type_name, ref_name, ea,
               pattern, offset);
    } else {
      LOG_INFO("{} {} signature: {:x} = {}", type_name, ref_name, ea, pattern);
    }

    return true;
  }

  LOG_ERROR("Failed to generate reference to {} {:x} with error: {}", type_name,
            ea, TranslateStatus(s));
}

PatternProvider::Result PatternProvider::GenerateFunctionReference(
    ea_t ref_ea,
    std::string& out_pattern,
    int8_t& out_offset) {
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

          // MUST not subtract this difference, by rather we need to go DO ref_ea + opcodesize - start_ea
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

PatternProvider::Result PatternProvider::UniqueDataSignature(
    ea_t target_ea,
    std::string& out_pattern,
    int8_t& out_offset) {
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

// This function allows manipulation of the data_flag parameter since
// we might also use a data reference to point to code as a last resort
// in the future.
PatternProvider::Result PatternProvider::UniqueCodeSignature(
    ea_t target_ea,
    std::string& out_pattern,
    int8_t& out_offset,
    bool& data_flag,
    ReferenceType& ref_type) {

  // valid code?
  if (!can_decode(target_ea))
    return Result::kDecodeError;

  data_flag = false;

  // try the direct way first.
  Result result{Result::kNoReferences};
  if ((result = GenerateSignatureInternal_2(target_ea, out_pattern)) ==
      Result::kSuccess) {
    out_offset = 0;
    ref_type = ReferenceType::kDirect;
    return result;
  }

  // ordered map based by function size
  // sorted by key, which is our function size
  std::map<uint32_t, ea_t> refs;
  for (auto ref_ea = get_first_cref_to(target_ea); ref_ea != BADADDR;
       ref_ea = get_next_cref_to(target_ea, ref_ea)) {
    if (target_ea == ref_ea)
      continue;

    // collect functional references
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