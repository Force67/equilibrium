// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "ui/plugin_ui.h"
#include "tools/usi_maker.h"
#include "tools/signature_maker.h"

// 'QuickRunner' is responsible for making quick links to features available.
//	>Features are either exposed through the RunDialog ui component or through hot-keys
//	>TODO: make user customizable
class Workbench final : public QObject {
 public:
  Workbench(Plugin&);
  ~Workbench();

  // do ensure that feature index in ui component
  // is always same as this!
  enum class FeatureIndex {
	  kSignatureMaker = 1,
	  kUSIMaker,
	  kCount,
  };

  void RunFeature(int data);

  USIMaker& usi_maker() { return usi_factory_; }

 private:
  FeatureIndex last_feature_{FeatureIndex::kSignatureMaker};
  USIMaker usi_factory_;
};

inline Workbench* work_bench();