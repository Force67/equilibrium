// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "pch.h"
#include "toolbox.h"
#include "utils/Opt.h"

namespace {
// Somethin
utils::Opt<bool> ToolCache{false, "Nd_PatternCache"};
}  // namespace

// s_mToolBox;

ToolBox::ToolBox() {}

ToolBox::~ToolBox() {}

void ToolBox::RegisterPattern(const std::string& pat) {
  bool shouldCache = ToolCache;
  if (!shouldCache)
    return;
}