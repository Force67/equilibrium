// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once
#include "ui/plugin_ui.h"
#include "tools/signature_maker.h"

// 'QuickRunner' is responsible for making quick links to features available.
//	>Features are either exposed through the RunDialog ui component or through hot-keys
//	>TODO: make user customizable
class QuickRunner final : public QObject {
 public:
  QuickRunner(Plugin&);
  ~QuickRunner();

  // do ensure that feature index in ui component
  // is always same as this!
  enum class FeatureIndex : int {
	  kSignatureMaker = 1,
	  kUSIMaker,
	  kCount,
  };

  void DispatchFeature(int data);

 private:
  FeatureIndex last_feature_{FeatureIndex::kSignatureMaker};
};