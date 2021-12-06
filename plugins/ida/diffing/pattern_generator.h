// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// IDA implementation of a pattern generation engine.
#pragma once

#include <vector>
#include "diffing/pattern.h"

class PatternGenerator {
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

  PatternGenerator();

  Status CreateSignature(const ea_t target_address, diffing::Pattern& out);
  bool CreatePrintSignature(const ea_t ea);

 private:
  using byte_set_t = std::vector<uchar>;

  std::string ByteRepresentationToString(const byte_set_t& bytes,
                                         const byte_set_t& masks);
  Status ComposeSignature(const ea_t in_address, std::string& out_pattern);

  using ref_collection_t = std::map<asize_t, ea_t>;
  void CollectDataReferences(const ea_t, ref_collection_t&);
  void CollectCodeReferences(const ea_t, ref_collection_t&);
  Status MakeReference(const ea_t target_ea, diffing::Pattern& out);

 private:
};