// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "pch.h"
#include "plugin.h"
#include "quick_runner.h"

QuickRunner::QuickRunner(Plugin& plugin) {
  // glue UI request to runner.
  connect(&plugin.Ui(), &PluginUi::RequestFeature, this,
          &QuickRunner::InvokeFeature);
}

QuickRunner::~QuickRunner() {}

void QuickRunner::InvokeFeature(int data) {
  const FeatureCode code = static_cast<FeatureCode>(data);
  switch (code) {
    case FeatureCode::SIGNATURE:
      // TODO: maybe execute binding?

    default:
      LOG_ERROR("??? : {}", data);
  }
}