// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "application.h"
#include "widget_prototyping.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <ui/skia/layer/imgui_layer.h>
#include <ui/platform/win/native_window_win32.h>

#include <Windows.h>
#include <ui/platform/win/message_pump_win.h>

#if 0
  {

    float SCALE = dpi.fX / 96.f;
    ImFontConfig cfg;
    cfg.SizePixels = 13 * SCALE;
    ImGui::GetIO().Fonts->AddFontDefault(&cfg)->Scale = SCALE;
  }
#endif

constexpr int kDefaultSuggestedWindowWidth = 1280;
constexpr int kDefaultSuggestedWindowHeight = 720;

Application::Application()
    :  // main_window_(kDefaultSuggestedWindowWidth, kDefaultSuggestedWindowHeight),
      layer_(im_ctx_) {
  window_ = std::make_unique<ui::NativeWindowWin32>(u8"RETK");
}

Application::~Application() {}

int Application::Exec() {
  window_->Init(nullptr,
                {0, 0, kDefaultSuggestedWindowWidth, kDefaultSuggestedWindowHeight});

  // further init stuff...
  // bind ui...

  window_->Show();

  ui::MessagePumpWin loop;
  while (loop.Update()) {
    loop.Pump();
  }

#if 0
  SkCanvas* canvas = main_window_.canvas();

  SkScalar dpi = main_window_.context()->dpi_scale().x();

  layer_.SetScaleFactor(dpi);

  // while the window should not close
  while (main_window_.IsOpen()) {
    // Yield this thread when we don't have to redraw
    main_window_.WaitForEventsThisFrame();

    // TODO: follow the DIP spec
    // https:  // en.wikipedia.org/wiki/Device-independent_pixel

    // Start the Dear ImGui frame
    ImGui_ImplGlfw_NewFrame();
    // we should have some pre-paint event
    ImGui::NewFrame();

#if 1
    // draw background canvas..
    SkPaint paint;
    paint.setColor(SK_ColorGREEN);
    canvas->drawPaint(paint);

    SkFont font;
    font.setSubpixel(true);
    font.setSize(15);
    paint.setColor(SK_ColorWHITE);

    // skia_->RestoreScaling();
    // https://mapsui.com/documentation/skia-scale.html
    // https://github.com/Mapsui/Mapsui/blob/d44f9cf0cdb30b118f3cb0d2342ac53717c50827/Mapsui.Rendering.Skia/SymbolStyleRenderer.cs
    // https://github.com/Mapsui/Mapsui/blob/1e2565651eb043a92e41cb575a6928fb345ad64d/Mapsui.UI.Shared/MapControl.cs#L464
    // SKIA uses pixel based rendering, so we need to scale.

    DrawListModel(canvas);
    DrawButton(canvas, "test", font);
    DrawToggleButton(canvas, true);

    // DrawGroupBox(canvas, paint, font);
    DrawMenubar(canvas, font);
    DrawStatusBar(canvas, font);
    DrawPlainTextBox(canvas, font);
#endif

    // finalize context
    RenderImGuiThisFrame(canvas);

    // present the frame
    int display_w, display_h;
    main_window_.QuerySize(display_w, display_h);
    glViewport(0, 0, display_w, display_h);
    layer_.Draw(canvas);
    main_window_.context()->Flush();

    // ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(main_window_.HACK_GETGlfwWindow());
  }
#endif

  return 0;
}