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

#include <ui/skia/skia_context.h>
#include <ui/skia/skia_context_factory.h>
#include <ui/display/win/dpi_win.h>

//#include <ui/layout/grid_layout.h>

// uncomment the two lines below to enable correct color spaces
//#define GL_FRAMEBUFFER_SRGB 0x8DB9
//#define GL_SRGB8_ALPHA8 0x8C43

static HMONITOR tracked_monitor_handle = nullptr;

int g_THE_POS_X = 0;
int g_THE_POS_Y = 0;

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

  ImGui::Render();
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

void DrawListModel(SkCanvas* c, ui::SkiaContext* skCtx) {
  const char* g_textRows[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I",
                              "J", "K", "L", "M", "N", "O", "P", "Q", "R",
                              "S", "T", "U", "V", "W", "X", "Y", "Z"};

  auto bounds = SkRect::MakeXYWH(g_THE_POS_X, g_THE_POS_Y, 300.f, 300.f);
  
  // https://github.com/aseprite/laf/blob/80ec051ecf4b702d769d4b2483e1a34b52368bde/os/skia/skia_surface.cpp#L42

  // https://github.com/NXPmicro/gtec-demo-framework/blob/master/Doc/FslSimpleUI.md

  SkPaint p;
  p.setColor(SK_ColorDKGRAY);
  //c->drawRoundRect(bounds, 1, 1, p);

  // http://microsoft.github.io/Win2D/WinUI2/html/DPI.htm
  // https://mapsui.com/documentation/skia-scale.html

  //c->drawCircle({bounds.x(), bounds.y()}, 100.f, p);
  //skCtx->RestoreScaling();
  c->drawRect(bounds, p);
}

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

class App {
 public:
  App();
  ~App();

  void Run();

 private:
  void BindGLContext();
  void DoCreateWindow();
  void DoResize(int x, int y);

  static void OnWindowMove(GLFWwindow* window, int, int);
  static void OnWindowSize(GLFWwindow*, int, int);

 private:
  GLFWwindow* window_ = nullptr;
  std::unique_ptr<ui::SkiaContext> skia_;
};

App::App() {
  BindGLContext();
  DoCreateWindow();
}

App::~App() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window_);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}

void App::BindGLContext() {
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
}

void App::DoCreateWindow() {
  ui::ContextCreateInfo create_info;
  create_info.width = 1920;
  create_info.height = 1080;

  window_ = glfwCreateWindow(create_info.width, create_info.height, "ReTK",
                             NULL, NULL);
  if (!window_) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window_);
  glfwSetWindowUserPointer(window_, this);
  //(uncomment to enable correct color spaces) glEnable(GL_FRAMEBUFFER_SRGB);
  bool err = glewInit() != GLEW_OK;

  skia_ = ui::CreateSkiaContext(create_info);
  skia_->SetDpiAware(glfwGetWin32Window(window_));
  tracked_monitor_handle =
      ui::GetCurrentMonitorHandle(glfwGetWin32Window(window_));

  glfwSwapInterval(1);
  glfwSetKeyCallback(window_, key_callback);
  glfwSetWindowPosCallback(window_, OnWindowMove);
  glfwSetWindowSizeCallback(window_, OnWindowSize);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();

  const char* glsl_version = "#version 130";

  // Setup Platform/Renderer back ends
  ImGui_ImplGlfw_InitForOpenGL(window_, true);
  ImGui_ImplOpenGL3_Init(glsl_version);
}

void App::DoResize(int x, int y) {
  skia_->Resize({static_cast<float>(x), static_cast<float>(y)});
}

void App::OnWindowMove(GLFWwindow* window, int x, int y) {
  HWND hwnd = glfwGetWin32Window(window);
  HMONITOR current_mon = ui::GetCurrentMonitorHandle(hwnd);
  if (current_mon != tracked_monitor_handle) {
    App* self = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
    assert(self);

    // re-apply dpi awareness for new monitor
    self->skia_->SetDpiAware(hwnd);
    tracked_monitor_handle = current_mon;
  }
}

void App::OnWindowSize(GLFWwindow* window, int x, int y) {
  HWND hwnd = glfwGetWin32Window(window);
  HMONITOR current_mon = ui::GetCurrentMonitorHandle(hwnd);
  App* self = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
  assert(self);

  // tell skia to apply a new window size
  self->DoResize(x, y);

  /* THIS IS ALSO REQUIRED! Since we need to resize the window to what the DPI value has to say
  * 
  // DPI Change handler. on WM_DPICHANGE resize the window and
// then call a function to redo layout for the child controls
UINT HandleDpiChange(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HWND hWndStatic = FindWindowEx(hWnd, nullptr, L"STATIC", nullptr);

    if (hWndStatic != nullptr)
    {
        UINT uDpi = HIWORD(wParam);

        // Resize the window
        auto lprcNewScale = reinterpret_cast<RECT*>(lParam);

        SetWindowPos(hWnd, nullptr, lprcNewScale->left, lprcNewScale->top,
            lprcNewScale->right - lprcNewScale->left, lprcNewScale->bottom -
lprcNewScale->top, SWP_NOZORDER | SWP_NOACTIVATE);

        // Redo layout of the child controls
        UpdateAndDpiScaleChildWindows(hWnd, uDpi);
    }

    return 0;
}
  */


  // ensure monitor is re applied
  self->skia_->SetDpiAware(hwnd);
}

void App::Run() {
#if 0
  {

    float SCALE = dpi.fX / 96.f;
    ImFontConfig cfg;
    cfg.SizePixels = 13 * SCALE;
    ImGui::GetIO().Fonts->AddFontDefault(&cfg)->Scale = SCALE;
  }
#endif

  SkCanvas* canvas = skia_->canvas();

  while (!glfwWindowShouldClose(window_)) {
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
    paint.setColor(SK_ColorWHITE);

    //skia_->RestoreScaling();
    // https://mapsui.com/documentation/skia-scale.html
    // https://github.com/Mapsui/Mapsui/blob/d44f9cf0cdb30b118f3cb0d2342ac53717c50827/Mapsui.Rendering.Skia/SymbolStyleRenderer.cs
    // https://github.com/Mapsui/Mapsui/blob/1e2565651eb043a92e41cb575a6928fb345ad64d/Mapsui.UI.Shared/MapControl.cs#L464
    // Skia uses pixel based rendering, so we need to scale.

        DrawListModel(canvas, skia_.get());
    DrawButton(canvas, "test", font);
    DrawToggleButton(canvas, true);

    DrawGroupBox(canvas, paint, font);

    // finalize context
    skia_->Flush();
    RenderImGuiThisFrame(canvas);

    // present the frame
    int display_w, display_h;
    glfwGetFramebufferSize(window_, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window_);
  }
}

// https://gist.github.com/ad8e/dd150b775ae6aa4d5cf1a092e4713add

int main(void) {
  App app;
  app.Run();
  return 0;
}