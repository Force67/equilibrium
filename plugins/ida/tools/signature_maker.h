// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

class SignatureMaker {
 public:
  explicit SignatureMaker();

  enum class Result {
    kSuccess,
    kDecodeError,
    kNoReferences,
    kRefLimitReached,
    kLengthExceeded,
    kEmpty,
  };
  static const char* const ResultToString(Result) noexcept;

  Result CreateUniqueSignature(const ea_t target_address,
                               std::string& out_pattern,
                               ptrdiff_t& out_offset,
                               bool mute_log,
                               bool& is_data);

 private:
  Result GenerateSignatureInternal_2(ea_t address, std::string& out_pattern);

  Result UniqueDataSignature(ea_t target_address,
                             std::string& out_pattern,
                             ptrdiff_t& out_offset);

  Result UniqueCodeSignature(ea_t target_address,
                             std::string& out_pattern,
                             ptrdiff_t& out_offset,
                             bool& very_dumb_flag);

  Result GenerateFunctionReference(ea_t target_address,
                                   std::string& out_pattern,
                                   ptrdiff_t& out_offset);

 private:
  qvector<uchar> bytes_;
  qvector<uchar> masks_;
};