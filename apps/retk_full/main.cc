// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <GL/glew.h>
#include <ShellScalingApi.h>

#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>

#include <core/SkCanvas.h>
#include <core/SkFont.h>
#include <core/SkGraphics.h>
#include <core/SkSurface.h>
#include <core/SkColorSpace.h>

#include <gpu/gl/GrGLInterface.h>
#include <gpu/GrContextOptions.h>

#include <gpu/GrDirectContext.h>
#include <gpu/GrBackendSurface.h>

#include <effects/SkGradientShader.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

//#include <ui/layout/grid_layout.h>


// uncomment the two lines below to enable correct color spaces
//#define GL_FRAMEBUFFER_SRGB 0x8DB9
//#define GL_SRGB8_ALPHA8 0x8C43

GrDirectContext* sContext = nullptr;
SkSurface* sSurface = nullptr;

ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

void error_callback(int error, const char* description) {
  fputs(description, stderr);
}

void key_callback(GLFWwindow* window,
                  int key,
                  int scancode,
                  int action,
                  int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}

void init_skia(int w, int h) {
  GrContextOptions options;
  sContext = GrDirectContext::MakeGL(nullptr, options).release();

  GrGLFramebufferInfo framebufferInfo;
  framebufferInfo.fFBOID = 0;  // assume default framebuffer
  // We are always using OpenGL and we use RGBA8 internal format for both RGBA
  // and BGRA configs in OpenGL.
  //(replace line below with this one to enable correct color spaces)
  //framebufferInfo.fFormat = GL_SRGB8_ALPHA8;
  framebufferInfo.fFormat = GL_RGBA8;
  #if 0
  SkColorType colorType;
  if (kRGBA_8888_GrPixelConfig == kSkia8888_GrPixelConfig) {
    colorType = kRGBA_8888_SkColorType;
  } else {
    colorType = kBGRA_8888_SkColorType;
  }
  #endif
  SkColorType colorType = kBGRA_8888_SkColorType;

  GrBackendRenderTarget backendRenderTarget(w, h,
                                            0,  // sample count
                                            0,  // stencil bits
                                            framebufferInfo);

  //(replace line below with this one to enable correct color spaces) sSurface =
  //SkSurface::MakeFromBackendRenderTarget(sContext, backendRenderTarget,
  //kBottomLeft_GrSurfaceOrigin, colorType, SkColorSpace::MakeSRGB(),
  //nullptr).release();
  sSurface = SkSurface::MakeFromBackendRenderTarget(
                 sContext, backendRenderTarget, kBottomLeft_GrSurfaceOrigin,
                 colorType, nullptr, nullptr)
                 .release();
  if (sSurface == nullptr)
    abort();
}

void cleanup_skia() {
  delete sSurface;
  delete sContext;
}

// https://gist.github.com/ad8e/dd150b775ae6aa4d5cf1a092e4713add

const int kWidth = 1920;
const int kHeight = 1080;
const char* glsl_version = "#version 130";

static GLFWwindow* g_window = nullptr;

void InitRendererBullshit() {
  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //(uncomment to enable correct color spaces) glfwWindowHint(GLFW_SRGB_CAPABLE,
  // GL_TRUE);
  glfwWindowHint(GLFW_STENCIL_BITS, 0);
  // glfwWindowHint(GLFW_ALPHA_BITS, 0);
  glfwWindowHint(GLFW_DEPTH_BITS, 0);

  g_window = glfwCreateWindow(kWidth, kHeight, "ReTK", NULL, NULL);
  if (!g_window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(g_window);
  //(uncomment to enable correct color spaces) glEnable(GL_FRAMEBUFFER_SRGB);
  bool err = glewInit() != GLEW_OK;

  init_skia(kWidth, kHeight);
  glfwSwapInterval(1);
  glfwSetKeyCallback(g_window, key_callback);
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();
  // Setup Platform/Renderer back ends
  ImGui_ImplGlfw_InitForOpenGL(g_window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);
}

SkPoint GetMonitorDpi() {
  // https://docs.microsoft.com/en-us/windows/win32/api/shellscalingapi/nf-shellscalingapi-getscalefactorformonitor
  // https://building.enlyze.com/posts/writing-win32-apps-like-its-2020-part-3/
  // usage with skia:  https://gitter.im/AvaloniaUI/Avalonia?at=5802746c891a53016311d46f
  // usage with imgui: https://ourmachinery.com/post/dpi-aware-imgui/
  // Don't forget to add the manifest to your application that specifies dpi awareness
  // note however that this method is unsupported on systems running windows 8 or older
  // but we don't provide fall back for these cases since we dropped windows 8 support.
  // DPI https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-how-should-i-handle-dpi-in-my-application

  HMONITOR monitor_handle =
      MonitorFromWindow(glfwGetWin32Window(g_window), MONITOR_DEFAULTTONEAREST);

  UINT x, y = 96;
  GetDpiForMonitor(monitor_handle, MONITOR_DPI_TYPE::MDT_EFFECTIVE_DPI, &x, &y);

  return SkPoint::Make(static_cast<float>(x), static_cast<float>(y));
}

void DebugPrintStats(SkCanvas *c) {
  printf("Canvas local bounds %f:%f\n", c->getLocalClipBounds().width(),
         c->getLocalClipBounds().height());

  printf("Canvas device bounds %f:%f\n", c->getDeviceClipBounds().width(),
         c->getDeviceClipBounds().height());
}

void RenderImGuiThisFrame() {
  static float f = 0.0f;
  static int counter = 0;

  ImGui::Begin("Ui Debug Stats");

  if (ImGui::Button("Get Device Bounds")) 
  {
    DebugPrintStats(sSurface->getCanvas());
  }

  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
  ImGui::End();

  ImGui::Render();
}

void DrawButton(SkCanvas* c, const char* text, const SkPaint& p, SkFont &font) {
  // todo: test if button has 'round' attribute
  SkRect r = SkRect::MakeXYWH(300, 100, 50, 20);
  c->drawRoundRect(r, 10, 10, p);

  SkPaint textCol;
  textCol.setColor(SK_ColorMAGENTA);
  c->drawSimpleText(text, strlen(text), SkTextEncoding::kUTF8, 
      r.centerX() - (r.width() / 4.f), 
      r.centerY() + (r.height() / 4.f), 
      font, textCol);
}

void DrawToggleButton(SkCanvas* c, const SkPaint& p, bool checked) {
  SkRect r = SkRect::MakeXYWH(400, 100, 30, 20);
  c->drawRoundRect(r, 10, 10, p);

  float margin = r.height() / 4.f;

  SkPaint lcol;
  lcol.setColor(SK_ColorWHITE);

  SkPoint circle_pos =
      checked
      ? SkPoint::Make(
            (r.centerX() + (r.width() / 4.f) - (margin / 2.f)), r.centerY())
              : SkPoint::Make((r.centerX() - (r.width() / 4.f)) + (margin / 2.f), r.centerY());

  c->drawCircle(circle_pos, ((r.height() - margin) / 2), lcol);
}

// widgets/uikit

static SkImage* g_img;

void DrawGroupBox(SkCanvas* c, const SkPaint& p, SkFont& font) {
  const char* g_textRows[] = {"Allgemein", "Mitteilungen", "Töne", "Fokus",
                              "Bildschirmzeit"};

  const SkRect bounds = SkRect::MakeXYWH(100, 200, 300, 300);
  const size_t count = sizeof(g_textRows) / sizeof(const char*);
  SkScalar row_height = bounds.height() / count;

  c->drawRoundRect(bounds, 10, 10, p);

  SkPaint lcol;
  lcol.setColor(SK_ColorWHITE);
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

    SkImageInfo imageInfo = SkImageInfo::Make(
        5, 5, kGray_8_SkColorType, kOpaque_SkAlphaType);
    SkPixmap pixmap(imageInfo, storage[0], sizeof(storage) / 5);

    SkBitmap bitmap;
    bitmap.installPixels(pixmap);

    SkRect img_bounds = SkRect::MakeXYWH(
        bounds.x(), bounds.y() + i * row_height, img_width, img_width);
    c->drawImageRect(bitmap.asImage(), img_bounds, SkSamplingOptions());

    c->drawSimpleText(g_textRows[i], strlen(g_textRows[i]),
                      SkTextEncoding::kUTF8, bounds.x(), 
        (bounds.y() + i * row_height) + 
        (row_height / 4.f), font,
                      lcol);
  }

}

int main(void) {
  InitRendererBullshit();
 
  // Draw to the surface via its SkCanvas.
  SkCanvas* canvas =
      sSurface->getCanvas();  // We don't manage this pointer's lifetime.

  DebugPrintStats(canvas);

  // enable dpi scaling.
  // TODO: handle monitor switching
  // see: https://gitter.im/AvaloniaUI/Avalonia?at=5802746c891a53016311d46f
  SkPoint dpi = GetMonitorDpi();
  canvas->restoreToCount(0);
  canvas->save();
  // USER_DEFAULT_SCREEN_DPI 
  canvas->scale(dpi.fX / 96.f, dpi.fY / 96.f);
  canvas->resetMatrix();

  {

    float SCALE = dpi.fX / 96.f;
    ImFontConfig cfg;
    cfg.SizePixels = 13 * SCALE;
    ImGui::GetIO().Fonts->AddFontDefault(&cfg)->Scale = SCALE;
  }

  while (!glfwWindowShouldClose(g_window)) {
    glfwWaitEvents();
    
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // draw background canvas..
    SkPaint paint;
    paint.setColor(SK_ColorLTGRAY);
    canvas->drawPaint(paint);

    SkFont font;
    font.setSubpixel(true);
    font.setSize(15);
    paint.setColor(SK_ColorBLACK);

    DrawButton(canvas, "test", paint, font);
    DrawToggleButton(canvas, paint, true);

    DrawGroupBox(canvas, paint, font);

    // finalize context
    sContext->flush();
    RenderImGuiThisFrame();

    // present the frame
    int display_w, display_h;
    glfwGetFramebufferSize(g_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(g_window);
  }

  cleanup_skia();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(g_window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}