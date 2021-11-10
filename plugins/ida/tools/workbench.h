// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "ui/plugin_ui.h"
#include "tools/signature_maker.h"
#include "tools/reflection_provider.h"
#include "idb/workbench_data.h"

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
 private:
  WorkbenchData data_;
  FeatureIndex last_feature_{FeatureIndex::kSignatureMaker};
};

inline Workbench* work_bench();