// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// ImGui render layer implementation on top of SKIA.
// Largely based on
// https://github.com/google/skia/blob/main/tools/viewer/ImGuiLayer.cpp

#include <base/check.h>
#include <core/SkCanvas.h>
#include <core/SkGraphics.h>
#include <core/SkVertices.h>
#include <core/SkImage.h>
#include <core/SkSwizzle.h>

#include <private/SkTDArray.h>
#include <private/SkTPin.h>
#include "ui/imgui/imgui_context.h"
#include "ui/skia/layer/imgui_layer.h"
#include <imgui.h>
#include <imgui_internal.h>

namespace ui {

namespace {
static void BuildImFontAtlas(ImFontAtlas& atlas, SkPaint& fontPaint) {
  int w, h;
  unsigned char* pixels;
  atlas.GetTexDataAsAlpha8(&pixels, &w, &h);
  SkImageInfo info = SkImageInfo::MakeA8(w, h);
  SkPixmap pmap(info, pixels, info.minRowBytes());
  SkMatrix localMatrix = SkMatrix::Scale(1.0f / w, 1.0f / h);
  auto fontImage = SkImage::MakeFromRaster(pmap, nullptr, nullptr);
  auto fontShader = fontImage->makeShader(
      SkSamplingOptions(SkFilterMode::kLinear), localMatrix);
  fontPaint.setShader(fontShader);
  fontPaint.setColor(SK_ColorWHITE);
  atlas.TexID = &fontPaint;
}
}  // namespace

ImguiSkiaLayer::ImguiSkiaLayer(ui::DearImGuiContext& ctx) : im_data_(ctx) {
  auto *fonts = ctx.IO().Fonts;
  BuildImFontAtlas(*fonts, font_paint_);
}

void ImguiSkiaLayer::SetScaleFactor(float factor) {
  im_data_.style().ScaleAllSizes(factor);
  ImFontAtlas& atlas = *im_data_.IO().Fonts;
  atlas.Clear();

  ImFontConfig cfg;
  cfg.SizePixels = 13 * factor;

  atlas.AddFontDefault(&cfg)->Scale = factor * 0.5f;
  BuildImFontAtlas(atlas, font_paint_);
}

void ImguiSkiaLayer::Draw(SkCanvas* canvas) {
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
}  // namespace ui