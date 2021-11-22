// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <core/SkCanvas.h>
#include <core/SkFont.h>
#include <core/SkGraphics.h>
#include <core/SkSurface.h>
#include <core/SkColorSpace.h>
#include <gpu/GrDirectContext.h>
#include <gpu/GrBackendSurface.h>
#include <effects/SkGradientShader.h>
#include <imgui.h>

int g_THE_POS_X = 0;
int g_THE_POS_Y = 30;

void RenderImGuiThisFrame(SkCanvas* c) {
  static float f = 0.0f;
  static int counter = 0;

  ImGui::Begin("Ui Debug Stats");

  ImGui::Text("Local bounds: %f, %f", c->getLocalClipBounds().width(),
              c->getLocalClipBounds().height());
  ImGui::Text("Device bounds: %f, %f", c->getDeviceClipBounds().width(),
              c->getDeviceClipBounds().height());

  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

  ImGui::SliderInt("Scale Pos X", &g_THE_POS_X, 0,
                   c->getLocalClipBounds().width());
  ImGui::SliderInt("Scale Pos Y", &g_THE_POS_Y, 0,
                   c->getLocalClipBounds().height());
  ImGui::End();
}

void DrawButton(SkCanvas* c, const char* text, SkFont& font) {
  // todo: test if button has 'round' attribute
  SkRect r = SkRect::MakeXYWH(400, 100, 50, 20);

  SkPaint p;
  p.setColor(SK_ColorBLUE);
  c->drawRoundRect(r, 10, 10, p);

  SkPaint textCol;
  textCol.setColor(SK_ColorMAGENTA);
  c->drawSimpleText(text, strlen(text), SkTextEncoding::kUTF8,
                    r.centerX() - (r.width() / 4.f),
                    r.centerY() + (r.height() / 4.f), font, textCol);
}

void DrawToggleButton(SkCanvas* c, bool checked) {
  SkRect r = SkRect::MakeXYWH(600, 100, 30, 20);

  SkPaint pBg;
  pBg.setColor(SK_ColorDKGRAY);
  c->drawRoundRect(r, 10, 10, pBg);

  float margin = r.height() / 4.f;

  SkPaint lcol;
  lcol.setColor(SK_ColorWHITE);

  SkPoint circle_pos =
      checked
          ? SkPoint::Make((r.centerX() + (r.width() / 4.f) - (margin / 2.f)),
                          r.centerY())
          : SkPoint::Make((r.centerX() - (r.width() / 4.f)) + (margin / 2.f),
                          r.centerY());

  c->drawCircle(circle_pos, ((r.height() - margin) / 2), lcol);
}

// widgets/uikit
void DrawGroupBox(SkCanvas* c, const SkPaint& p, SkFont& font) {
  const char* g_textRows[] = {"Allgemein", "Mitteilungen", "Töne", "Fokus",
                              "Bildschirmzeit"};

  const SkRect bounds = SkRect::MakeXYWH(200, 200, 300, 300);
  const size_t count = sizeof(g_textRows) / sizeof(const char*);
  SkScalar row_height = bounds.height() / count;

  c->drawRoundRect(bounds, 10, 10, p);

  SkPaint lcol;
  lcol.setColor(SK_ColorBLACK);
  for (size_t i = 1; i < count; i++) {
    c->drawLine({bounds.x(), bounds.y() + row_height * i},
                {bounds.fRight, (bounds.y() + row_height * i)}, lcol);
  }

  for (size_t i = 0; i < count; i++) {
    uint8_t storage[][5] = {{0xCA, 0xDA, 0xCA, 0xC9, 0xA3},
                            {0xAC, 0xA8, 0x89, 0xA7, 0x87},
                            {0x9B, 0xB5, 0xE5, 0x95, 0x46},
                            {0x90, 0x81, 0xC5, 0x71, 0x33},
                            {0x75, 0x55, 0x44, 0x40, 0x30}};

    // THIS TAKES WIDTH AND HEIGHT FOR NOW
    float img_width = bounds.width() / 8.f;

    // TODO: collide image... so we get difference

    SkImageInfo imageInfo =
        SkImageInfo::Make(5, 5, kGray_8_SkColorType, kOpaque_SkAlphaType);
    SkPixmap pixmap(imageInfo, storage[0], sizeof(storage) / 5);

    SkBitmap bitmap;
    bitmap.installPixels(pixmap);

    SkRect img_bounds = SkRect::MakeXYWH(
        bounds.x(), bounds.y() + i * row_height, img_width, img_width);
    c->drawImageRect(bitmap.asImage(), img_bounds, SkSamplingOptions());

    c->drawSimpleText(
        g_textRows[i], strlen(g_textRows[i]), SkTextEncoding::kUTF8, bounds.x(),
        (bounds.y() + i * row_height) + (row_height / 4.f), font, lcol);
  }
}

void DrawMenubar(SkCanvas* c, SkFont& font) {
  const char* g_textRows[] = {"File",    "Edit",    "Jump",
                              "Search",  "View",    "Debugger",
                              "Options", "Windows", "Help"};
  const size_t count = sizeof(g_textRows) / sizeof(const char*);

  const auto bounds = SkRect::MakeXYWH(0, 0, 1000.f, 30.f);

  SkPaint p;
  p.setColor(SK_ColorMAGENTA);
  c->drawRect(bounds, p);

  SkPaint lcol;
  lcol.setColor(SK_ColorBLACK);

  // calculate average text height for all rows
  SkRect text_bounds{};
  SkScalar avg_text_height = 1.f;

  // easy and cheap workaround of doing this
  font.measureText("A", 1, SkTextEncoding::kUTF8, &text_bounds);
  avg_text_height = text_bounds.height();

  SkScalar x_offset = bounds.fLeft + 5.f;
  for (size_t i = 0; i < count; i++) {
    const auto text_ptr = g_textRows[i];

    const size_t text_length = strlen(text_ptr);
    font.measureText(text_ptr, text_length, SkTextEncoding::kUTF8,
                     &text_bounds);

    // Why are we using avg_text_height instead of text_bounds.width()?
    // Well, we need to calculate the average text height since for instance
    // the total x height will be affected by the presence of a p or q since
    // they go below the start pos so the total height will
    // appear larger than it should be.
    const SkScalar pos_x = x_offset;
    const SkScalar pos_y = bounds.centerY() + (avg_text_height / 2.f);

    // move offset
    x_offset += text_bounds.width() + 15.f;
    c->drawSimpleText(text_ptr, text_length, SkTextEncoding::kUTF8, pos_x,
                      pos_y, font, lcol);
  }
}

void DrawStatusBar(SkCanvas* c, SkFont& font) {
  const char* g_textRows[] = {"Status: Busy"};
  const size_t count = sizeof(g_textRows) / sizeof(const char*);

  const auto bounds = SkRect::MakeXYWH(
      0, c->getLocalClipBounds().height() - 30.f, 1000.f, 30.f);

  SkPaint p;
  p.setColor(SK_ColorMAGENTA);
  c->drawRect(bounds, p);

  SkPaint lcol;
  lcol.setColor(SK_ColorBLACK);

  // calculate average text height for all rows
  SkRect text_bounds{};
  SkScalar avg_text_height = 1.f;

  // easy and cheap workaround of doing this
  font.measureText("A", 1, SkTextEncoding::kUTF8, &text_bounds);
  avg_text_height = text_bounds.height();

  SkScalar x_offset = bounds.fLeft + 5.f;
  for (size_t i = 0; i < count; i++) {
    const auto text_ptr = g_textRows[i];

    const size_t text_length = strlen(text_ptr);
    font.measureText(text_ptr, text_length, SkTextEncoding::kUTF8,
                     &text_bounds);

    // Why are we using avg_text_height instead of text_bounds.width()?
    // Well, we need to calculate the average text height since for instance
    // the total x height will be affected by the presence of a p or q since
    // they go below the start pos so the total height will
    // appear larger than it should be.
    const SkScalar pos_x = x_offset;
    const SkScalar pos_y = bounds.centerY() + (avg_text_height / 2.f);

    // move offset
    x_offset += text_bounds.width() + 15.f;
    c->drawSimpleText(text_ptr, text_length, SkTextEncoding::kUTF8, pos_x,
                      pos_y, font, lcol);
  }
}

void DrawPlainTextBox(SkCanvas* c, SkFont& font) {
  static constexpr char g_FunnyText[] = R"(
push rsi push rdi sub rsp,
60h movaps[rsp + 78h + var_28],
xmm6 mov rax,
cs : __security_cookie xor rax,
rsp mov[rsp + 78h + var_30],
rax mov rax,
[rcx] mov rbx,
rcx mov rsi,
r9 movaps xmm6,
xmm2 mov rdi,
rdx mov ecx,
[rax + 24h] mov rax,
[rdx] cmp ecx,
[rax + 24h] jz short loc_14006B66A xor al,
al jmp short loc_14006B6D0)";
  auto bounds = SkRect::MakeXYWH(200.f, 30.f, 600.f, 600.f);

  // https://github.com/aseprite/laf/blob/80ec051ecf4b702d769d4b2483e1a34b52368bde/os/skia/skia_surface.cpp#L42

  // https://github.com/NXPmicro/gtec-demo-framework/blob/master/Doc/FslSimpleUI.md

  SkPaint p;
  p.setColor(SK_ColorDKGRAY);
  // c->drawRoundRect(bounds, 1, 1, p);

  // http://microsoft.github.io/Win2D/WinUI2/html/DPI.htm
  // https://mapsui.com/documentation/skia-scale.html
  c->drawRect(bounds, p);

  SkPaint q;
  q.setColor(SK_ColorWHITE);

  SkRect text_bounds{};
  SkScalar avg_text_offset = 1.f;

  // easy and cheap workaround of doing this
  font.measureText("Aq", 1, SkTextEncoding::kUTF8, &text_bounds);
  avg_text_offset = text_bounds.height() + 5.f;

  size_t last_i = 0;
  size_t depth = 0;
  for (size_t i = 0; i < strlen(g_FunnyText); i++) {
    if (g_FunnyText[i] == '\n') {
      const SkScalar fx = (bounds.fTop + (depth * avg_text_offset));

      c->drawSimpleText(&g_FunnyText[last_i], i - last_i, SkTextEncoding::kUTF8,
                        bounds.fLeft, fx, font, q);
      last_i = i;
      depth++;
    }
  }
}

void DrawListModel(SkCanvas* c) {
  const char* g_textRows[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I",
                              "J", "K", "L", "M", "N", "O", "P", "Q", "R",
                              "S", "T", "U", "V", "W", "X", "Y", "Z"};

  auto bounds = SkRect::MakeXYWH(g_THE_POS_X, g_THE_POS_Y, 200.f, 300.f);

  // https://github.com/aseprite/laf/blob/80ec051ecf4b702d769d4b2483e1a34b52368bde/os/skia/skia_surface.cpp#L42

  // https://github.com/NXPmicro/gtec-demo-framework/blob/master/Doc/FslSimpleUI.md

  SkPaint p;
  p.setColor(SK_ColorDKGRAY);
  // c->drawRoundRect(bounds, 1, 1, p);

  // http://microsoft.github.io/Win2D/WinUI2/html/DPI.htm
  // https://mapsui.com/documentation/skia-scale.html
  c->drawRect(bounds, p);

  SkPaint lcol;
  lcol.setColor(SK_ColorWHITE);
  const size_t count = sizeof(g_textRows) / sizeof(const char*);
  SkScalar row_height = bounds.height() / count;
  for (size_t i = 1; i < count; i++) {
    c->drawLine({bounds.x(), bounds.y() + row_height * i},
                {bounds.fRight, (bounds.y() + row_height * i)}, lcol);
  }
}

// NOTE ON dropShadow
// https://docs.microsoft.com/en-us/dotnet/api/skiasharp.skimagefilter.createdropshadow?view=skiasharp-2.80.2
// https://material.io/components/chips#usage