// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Example window presentation based on GLFW
#pragma once

#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>

#include <core/SkSurface.h>
#include <ui/skia/skia_context.h>

// This class is named 'WindowGlfw' so we can avoid collisions with
// glfw's window struct name
class WindowGlfw {
 public:
  explicit WindowGlfw(int width, int height);
  ~WindowGlfw();

  inline bool IsOpen() const { return !glfwWindowShouldClose(window_); }

  static void WaitForEventsThisFrame() {
    // This is responsible for slowing down our fps when unused.
    glfwWaitEvents();
  }

  SkCanvas* canvas() { return skia_->canvas();
  }

  void QuerySize(int& width, int& height) {
    glfwGetFramebufferSize(window_, &width, &height);
  }

  GLFWwindow* HACK_GETGlfwWindow() { return window_;
  }

  ui::SkiaContext* context() { return skia_.get();
  }

 private:
  static void BindContext();
  void CreateWindowWindowsIsStupid();

  void HandleResize(int x, int y);

  static void OnWindowMove(GLFWwindow* window, int, int);
  static void OnWindowSize(GLFWwindow*, int, int);
  static void OnWindowScale(GLFWwindow*, float, float);

 private:
  GLFWwindow* window_ = nullptr;
  std::unique_ptr<ui::SkiaContext> skia_;
};
