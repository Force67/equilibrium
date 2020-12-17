// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace tools {
class Toolbox {
 public:
  Toolbox();
  ~Toolbox();

  enum class FeatureCode {
	kNone,
	kSignature
  };

  void TriggerFeature(FeatureCode);

  void RegisterPattern(const std::string&);
};
}