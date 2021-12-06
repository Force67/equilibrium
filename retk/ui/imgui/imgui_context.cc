// Copyright (C) 2021 Force67 <github.com/Force67>.
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
  io.KeyMap[ImGuiKey_Tab] = static_cast<int>(Key::kTab);
  io.KeyMap[ImGuiKey_LeftArrow] = static_cast<int>(Key::kLeft);
  io.KeyMap[ImGuiKey_RightArrow] = static_cast<int>(Key::kRight);
  io.KeyMap[ImGuiKey_UpArrow] = static_cast<int>(Key::kUp);
  io.KeyMap[ImGuiKey_DownArrow] = static_cast<int>(Key::kDown);
  io.KeyMap[ImGuiKey_PageUp] = static_cast<int>(Key::kPageUp);
  io.KeyMap[ImGuiKey_PageDown] = static_cast<int>(Key::kPageDown);
  io.KeyMap[ImGuiKey_Home] = static_cast<int>(Key::kHome);
  io.KeyMap[ImGuiKey_End] = static_cast<int>(Key::kEnd);
  io.KeyMap[ImGuiKey_Delete] = static_cast<int>(Key::kDelete);
  io.KeyMap[ImGuiKey_Backspace] = static_cast<int>(Key::kBack);
  io.KeyMap[ImGuiKey_Enter] = static_cast<int>(Key::kOK);
  io.KeyMap[ImGuiKey_Escape] = static_cast<int>(Key::kEscape);
  io.KeyMap[ImGuiKey_A] = static_cast<int>(Key::kA);
  io.KeyMap[ImGuiKey_C] = static_cast<int>(Key::kC);
  io.KeyMap[ImGuiKey_V] = static_cast<int>(Key::kV);
  io.KeyMap[ImGuiKey_X] = static_cast<int>(Key::kX);
  io.KeyMap[ImGuiKey_Y] = static_cast<int>(Key::kY);
  io.KeyMap[ImGuiKey_Z] = static_cast<int>(Key::kZ);
}
}  // namespace ui