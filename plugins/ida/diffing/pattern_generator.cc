
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// IDA implementation of the signature generator.
//
// Warning: Please consider always running this in release mode unless needed,
//          as it's very compute intensive.

#include "Pch.h"
#include <bytes.hpp>

#include "pattern_generator.h"
#include "pattern_generator_config.h"

#include <base/check.h>
#include <base/arch.h>
#include <base/scoped_profile.h>
using namespace arch_types;

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
}  // namespace

PatternGenerator::PatternGenerator() = default;

const char* const PatternGenerator::TranslateStatus(Status result) {
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
      IMPOSSIBLE;
      return "<unknown>";
  }
}

std::string PatternGenerator::ByteRepresentationToString(
    const byte_set_t& bytes,
    const byte_set_t& masks) {
  std::string str_pattern{};

  static constexpr char kHexMask[] = "0123456789ABCDEF";
  for (size_t i = 0; i < bytes.size(); ++i) {
    if (i)
      str_pattern += ' ';

    const uchar mask = masks[i];
    const uchar value = bytes[i];

    if (mask != 0x00) {
      str_pattern += kHexMask[static_cast<size_t>(value >> 4)];
      str_pattern += kHexMask[static_cast<size_t>(value & 0xF)];
    } else {
      str_pattern += '?';
    }
  }

  // trip remaining spaces or question marks at the end
  // this saves us time in the long run, as we might not
  // have to rerun the pattern to get a shorter pattern
  for (size_t i = str_pattern.size(); i-- != 0;) {
    if (str_pattern[i] != ' ' && str_pattern[i] != '?') {
      str_pattern.resize(i + 1);
      break;
    }
  }

  return str_pattern;
}

PatternGenerator::Status PatternGenerator::ComposeSignature(
    const ea_t in_address,
    std::string& out_pattern) {
  // enter timed scope
  base::ScopedProfile profile_frame("ComposeSignature");
  (void)profile_frame;

  byte_set_t bytes;
  byte_set_t masks;

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
        if (bin_search2(
                start, seg->end_ea, bytes.data(), masks.data(), bytes.size(),
                start <= seg->end_ea ? BIN_SEARCH_FORWARD
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

  out_pattern = ByteRepresentationToString(bytes, masks);
  if (out_pattern.length() > pattern_config::kSignatureMaxLength ||
      // patterns must not contain excessive spacing
      // since too many wild-cards will inevitably lead to breakage
      // if ABI changes occur
      !diffing::ValidateWildcardCount(out_pattern, true)) {
    return Status::kLengthExceeded;
  }

  return Status::kSuccess;
}

// Too lazy to write an iterator adapeter for this.
void PatternGenerator::CollectCodeReferences(
    const ea_t ea,
    ref_collection_t& refs) {
  for (auto cur_ea = get_first_cref_to(ea); cur_ea != BADADDR;
       cur_ea = get_next_cref_to(ea, cur_ea)) {
    if (ea == cur_ea)
      continue;
    if (const func_t* func = get_func(cur_ea)) {
      refs.insert(std::make_pair(func->size(), cur_ea));
    }
  }
}

void PatternGenerator::CollectDataReferences(
    const ea_t ea,
    ref_collection_t& refs) {
  for (auto cur_ea = get_first_dref_to(ea); cur_ea != BADADDR;
       cur_ea = get_next_dref_to(ea, cur_ea)) {
    if (ea == cur_ea)
      continue;
    if (const func_t* func = get_func(cur_ea)) {
      refs.insert(std::make_pair(func->size(), cur_ea));
    }
  }
}

PatternGenerator::Status PatternGenerator::CreateSignature(
    const ea_t target_ea,
    diffing::Pattern& out) {
  // Must be within code
  if (!CheckAddressTypeAllowed(target_ea))
    return Status::kUnsupportedType;

  out.is_data = GetAddressFlags(target_ea) == FF_DATA;
  out.type = diffing::Pattern::ReferenceType::kRef3;

  Status s{Status::kNoReferences};
  if (!out.is_data) {
    // is it code?
    if (!can_decode(target_ea))
      return Status::kDecodeError;
    // try the direct way first.
    if ((s = ComposeSignature(target_ea, out.bytes)) == Status::kSuccess) {
      // This is a direct offset, we have zero levels of indirection
      out.lv0_offset = out.lv1_offset = 0;
      out.type = diffing::Pattern::ReferenceType::kDirect;
      return s;
    }
  }

  //func_item_iterator_t 

  ref_collection_t refs;
  out.is_data ? CollectDataReferences(target_ea, refs)
              : CollectCodeReferences(target_ea, refs);

  // we now iterate through the refs, starting from the best
  // offset going to the worst
  for (size_t i = 0; i < refs.size(); i++) {
    // avoid really long taking loops
    if (i > pattern_config::kMaxRefCountAnalysisDepth)
      return Status::kRefLimitReached;

    const auto& pair = *std::next(refs.begin(), i);
    LOG_TRACE("Ref: {:x} {}/{}", pair.second, i, refs.size());
    s = MakeReference(pair.second, out);
    if (s == Status::kSuccess)
      return s;
  }

  return s;
}

PatternGenerator::Status PatternGenerator::MakeReference(
    const ea_t ref_ea,
    diffing::Pattern& out) {
  const func_t* func = get_func(ref_ea);
  if (!func)
    return Status::kDecodeError;

  insn_t insn{};
  if (decode_insn(&insn, ref_ea) == 0) {
    return Status::kDecodeError;
  }

  // the needle is in the middle
  // we go down.
  ea_t needle = ref_ea + GetOpCodeSize(insn);
  while (needle >= func->start_ea) {
    if (decode_insn(&insn, needle) == 0)
      return Status::kDecodeError;

    if (ComposeSignature(needle, out.bytes) == Status::kSuccess) {
      // set the offset of the second indirection
      BUGCHECK(ref_ea - needle < INT8_MAX);

      out.lv1_offset = ref_ea - needle;
      return Status::kSuccess;
    }

    needle -= GetOpCodeSize(insn);
  }

  // the same dance again, upwards
  needle = ref_ea + GetOpCodeSize(insn);
  while (needle <= func->end_ea) {
    if (decode_insn(&insn, needle) == 0)
      return Status::kDecodeError;

    if (ComposeSignature(needle, out.bytes) == Status::kSuccess) {
      BUGCHECK(ref_ea - needle < INT8_MAX);
      // set the offset of the second indirection
      out.lv1_offset = needle - ref_ea;
      return Status::kSuccess;
    }

    needle += GetOpCodeSize(insn);
  }

  return Status::kEmpty;
}

bool PatternGenerator::CreatePrintSignature(const ea_t ea) {
  LOG_TRACE("Creating signature for {:x}", ea);

  diffing::Pattern pattern{};
  const Status s = CreateSignature(ea, pattern);

  if (s == Status::kSuccess) {
    const auto pretty_print = diffing::PrettyPrintPattern(pattern, ea);
    LOG_INFO(pretty_print.c_str());
    return true;
  }

  LOG_ERROR("Failed to generate reference to {} {:x} with error: {}",
            diffing::TypeToString(pattern), ea, TranslateStatus(s));
  return false;
}