// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <imgui.h>
#include <imgui_internal.h>

#include <ui/keycode.inl>
#include <ui/layer/imgui_skia_backend.h>
#include <base/logging.h>

#include <core/SkCanvas.h>
#include <core/SkGraphics.h>
#include <core/SkVertices.h>
#include <core/SkSwizzle.h>

#include <private/SkTDArray.h>
#include <private/SkTPin.h>

namespace uikit {

ImguiSkiaBackend::ImguiSkiaBackend() {
#if defined(TK_DBG)
  IMGUI_CHECKVERSION();
#endif

  ImGuiContext *im_context = ImGui::CreateContext();
  ImGuiIO& io = im_context->IO;
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

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
}

ImguiSkiaBackend::~ImguiSkiaBackend() {
  ImGui::DestroyContext();
}

void ImguiSkiaBackend::Draw(SkCanvas* canvas) {
  ImGui::Render();

  // Then we fetch the most recent data, and convert it so we can render with
  // Skia
  const ImDrawData* drawData = ImGui::GetDrawData();
  SkTDArray<SkPoint> pos;
  SkTDArray<SkPoint> uv;
  SkTDArray<SkColor> color;

  for (int i = 0; i < drawData->CmdListsCount; ++i) {
    const ImDrawList* drawList = drawData->CmdLists[i];

    // De-interleave all vertex data (sigh), convert to Skia types
    pos.rewind();
    uv.rewind();
    color.rewind();
    for (int j = 0; j < drawList->VtxBuffer.size(); ++j) {
      const ImDrawVert& vert = drawList->VtxBuffer[j];
      pos.push_back(SkPoint::Make(vert.pos.x, vert.pos.y));
      uv.push_back(SkPoint::Make(vert.uv.x, vert.uv.y));
      color.push_back(vert.col);
    }
    // ImGui colors are RGBA
    SkSwapRB(color.begin(), color.begin(), color.count());

    int indexOffset = 0;

    // Draw everything with canvas.drawVertices...
    for (int j = 0; j < drawList->CmdBuffer.size(); ++j) {
      const ImDrawCmd* drawCmd = &drawList->CmdBuffer[j];

      SkAutoCanvasRestore acr(canvas, true);

      // TODO: Find min/max index for each draw, so we know how many vertices
      // (sigh)
      if (drawCmd->UserCallback) {
        drawCmd->UserCallback(drawList, drawCmd);
      } else {
          SkPaint* paint = static_cast<SkPaint*>(drawCmd->TextureId);
          TK_DCHECK(paint);

          canvas->clipRect(
              SkRect::MakeLTRB(drawCmd->ClipRect.x, drawCmd->ClipRect.y,
                               drawCmd->ClipRect.z, drawCmd->ClipRect.w));
          auto vertices = SkVertices::MakeCopy(
              SkVertices::kTriangles_VertexMode, drawList->VtxBuffer.size(),
              pos.begin(), uv.begin(), color.begin(), drawCmd->ElemCount,
              drawList->IdxBuffer.begin() + indexOffset);
          canvas->drawVertices(vertices, SkBlendMode::kModulate, *paint);
        }
        indexOffset += drawCmd->ElemCount;
      }
  }
}
}  // namespace uikit