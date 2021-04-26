// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace tools {
class Toolbox;

class SignatureGenerator {
 public:
  enum class Result {
    kSuccess,
    kDecodeError,
    kNoReferences,
    kRefLimitReached,
    kLengthExceeded,
    kEmpty,
  };
  static const char* const ResultToString(Result) noexcept;

  explicit SignatureGenerator(Toolbox* toolbox);

  std::string UniqueSignature(ea_t target_address, bool mute_log);

 private:
  Result GenerateSignatureInternal_2(ea_t address, std::string& out_pattern);

  Result UniqueDataSignature(ea_t target_address,
                             std::string& out_pattern,
                             size_t& out_offset);
  Result UniqueCodeSignature(ea_t target_address,
                             std::string& out_pattern,
                             size_t& out_offset);

  Result GenerateFunctionReference(ea_t target_address,
                                   std::string& out_pattern,
                                   size_t& out_offset);

 private:
  Toolbox* toolbox_;
  qvector<uchar> bytes_;
  qvector<uchar> masks_;
};
}  // namespace tools