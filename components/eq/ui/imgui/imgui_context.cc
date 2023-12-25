// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// ImGui implementation.

#include <imgui.h>
#include <imgui_internal.h>
#include <ui/keycode.inl>
#include "imgui_context.h"

namespace ui {
DearImGuiContext::DearImGuiContext() {
  IMGUI_CHECKVERSION();
  im_context_ = ImGui::CreateContext();
  // don't save pos to an ini
  im_context_->IO.IniFilename = nullptr;

  SetKeyBindings();
  ImGui::StyleColorsDark();

  // TODO: configure our own dpi scaling...
}

DearImGuiContext::~DearImGuiContext() {
  ImGui::DestroyContext(im_context_);
}

void DearImGuiContext::SetKeyBindings() {
  ImGuiIO& io = im_context_->IO;

}
}  // namespace ui