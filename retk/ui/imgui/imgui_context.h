// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// ImGui context wrapper.
#pragma once

struct ImGuiContext;

namespace ui {
class DearImGuiContext {
 public:
  DearImGuiContext();
  ~DearImGuiContext();

 private:
  void SetKeyBindings();

 private:
  ImGuiContext* im_context_;
};
}  // namespace ui