// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// ImGui context wrapper.
#pragma once

#include <imgui.h>
#include <imgui_internal.h>

namespace ui {
class DearImGuiContext {
 public:
  DearImGuiContext();
  ~DearImGuiContext();

  inline ImGuiContext* context() { return im_context_; }
  inline ImGuiIO& IO() { return im_context_->IO; }
  inline ImGuiStyle& style() { return im_context_->Style; }

 private:
  void SetKeyBindings();

 private:
  ImGuiContext* im_context_;
};
}  // namespace ui