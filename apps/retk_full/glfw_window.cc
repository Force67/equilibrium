// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Example window presentation based on GLFW

#include <cstdio>
#include <GL/glew.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <ui/skia/skia_context.h>
#include <ui/skia/skia_context_factory.h>
#include <ui/display/win/dpi_win.h>

#include "glfw_window.h"

namespace {
HMONITOR tracked_monitor_handle = nullptr;

void error_callback(int error, const char* description) {
  fputs(description, stderr);
}
}  // namespace

WindowGlfw::WindowGlfw(int width, int height) {
  glfwSetErrorCallback(error_callback);
  BindContext();
  InternalCreateWindow(width, height);
}

WindowGlfw::~WindowGlfw() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  if (window_)
    glfwDestroyWindow(window_);
}

void WindowGlfw::InternalCreateWindow(int width, int height) {
  ui::ContextCreateInfo create_info;
  create_info.width = width;
  create_info.height = height;

  window_ = glfwCreateWindow(create_info.width, create_info.height, "ReTK",
                             NULL, NULL);
  assert(window_);

  glfwMakeContextCurrent(window_);
  glfwSetWindowUserPointer(window_, this);
  //(uncomment to enable correct color spaces)
  // glEnable(GL_FRAMEBUFFER_SRGB);
  assert(glewInit() == GLEW_OK);

  glfwSwapInterval(1);
  glfwSetKeyCallback(window_, OnKeyInput);
  glfwSetWindowPosCallback(window_, OnMove);
  glfwSetWindowSizeCallback(window_, OnResize);
  glfwSetWindowContentScaleCallback(window_, OnScale);

  // detect if we were up-scaled
  // This is done to counter GLFW's initial upscaling when
  // GLFW_SCALE_TO_MONITOR is specified.
  {
    int real_width = 0, real_height = 0;
    glfwGetWindowSize(window_, &real_width, &real_height);

    if (create_info.width != real_width || create_info.height != real_height) {
      create_info = {.width = real_width, .height = real_height};
    }
  }

  skia_ = ui::CreateSkiaContext(create_info);
  skia_->SetDpiAware(glfwGetWin32Window(window_));
  tracked_monitor_handle =
      ui::GetCurrentMonitorHandle(glfwGetWin32Window(window_));

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

void WindowGlfw::BindContext() {
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //(uncomment to enable correct color spaces)
  // glfwWindowHint(GLFW_SRGB_CAPABLE,
  // GL_TRUE);
  glfwWindowHint(GLFW_STENCIL_BITS, 0);
  // glfwWindowHint(GLFW_ALPHA_BITS, 0);
  glfwWindowHint(GLFW_DEPTH_BITS, 0);

  // ==================
  // When switching dpi contexts, make the window automatically aware of the
  // new desired size. Note that when this is enabled, we have to first
  // bounce another resize call Make sure to pop this hint before actually
  // creating the window
  // https://github.com/glfw/glfw/blob/56a4cb0a3a2c7a44a2fd8ab3335adf915e19d30c/src/win32_window.c#L1303
  glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
}

void WindowGlfw::HandleMove(int x, int y) {
  HWND hwnd = glfwGetWin32Window(window_);
  HMONITOR current_mon = ui::GetCurrentMonitorHandle(hwnd);
  if (current_mon != tracked_monitor_handle) {
    // re-apply DPI awareness for new monitor
    skia_->SetDpiAware(hwnd);
    tracked_monitor_handle = current_mon;
  }
}

void WindowGlfw::HandleResize(int x, int y) {
  skia_->Resize({static_cast<float>(x), static_cast<float>(y)});
  // ensure monitor is re applied
  // TODO: this is improper, but fixes the resize bug
  // i have no idea why
  HWND hwnd = glfwGetWin32Window(window_);
  skia_->SetDpiAware(hwnd, true);
}

void WindowGlfw::HandleScale(float, float) {
  HWND hwnd = glfwGetWin32Window(window_);
}

void WindowGlfw::HandleKeyInput(int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window_, GL_TRUE);
}