// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "tools/signature_generator.h"

namespace tools {
class Toolbox {
 public:
  Toolbox();
  ~Toolbox();

  enum class ActionCode { kNone, kSignature };

  void InvokeAction(ActionCode);
  void RegisterPattern(const std::string&);

  SignatureGenerator& signature_generator() { return generator_; }

 private:
  SignatureGenerator generator_;
};

Toolbox* toolbox() noexcept;
}  // namespace tools