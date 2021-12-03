// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// IDA implementation of a pattern generation engine.
// TODO: consider if this should be named PatternGenerator instead.
#pragma once

#include "diffing/pattern.h"

class PatternProvider {
 public:
  enum class Status {
    kSuccess,
    kUnsupportedType,
    kDecodeError,
    kNoReferences,
    kRefLimitReached,
    kLengthExceeded,
    kEmpty,
  };
  static const char* const TranslateStatus(Status);

  PatternProvider();

  Status CreateSignature(const ea_t target_address, diffing::Pattern& out);
  bool CreatePrintSignature(const ea_t ea);
 private:
  Status ComposeSignature(const ea_t in_address, std::string& out_pattern);
};

class SignatureMaker {
 public:
  explicit SignatureMaker();

  static const char* const ResultToString(Result) noexcept;

  Result CreateSignature(const ea_t target_address,
                         std::string& out_pattern,
                         int8_t& out_offset,
                         bool& is_data,
                         ReferenceType& ref_type);

  bool CreateAndPrintSignature(const ea_t target_ea);

 private:
  Result GenerateSignatureInternal_2(ea_t address, std::string& out_pattern);

  Result UniqueDataSignature(ea_t target_address,
                             std::string& out_pattern,
                             int8_t& out_offset);

  Result UniqueCodeSignature(ea_t target_address,
                             std::string& out_pattern,
                             int8_t& out_offset,
                             bool& very_dumb_flag,
                             ReferenceType& ref_type);

  Result GenerateFunctionReference(ea_t target_address,
                                   std::string& out_pattern,
                                   int8_t& out_offset);

 private:
  qvector<uchar> bytes_;
  qvector<uchar> masks_;
};