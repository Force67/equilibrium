// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Example window presentation based on GLFW
#pragma once

#include <assert.h>
#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>
#include <base/compiler.h>
#include <core/SkSurface.h>
#include <ui/skia/skia_context.h>

// Generates the forwarders. Note that you have to declare these with a bit
// of an awkward syntax, like: IMPL_GLFW_FORWARDER(Move, int(x), int(y))
#define IMPL_GLFW_FORWARDER(name, ...)                                   \
  static void On##name(GLFWwindow* window, __VA_ARGS__) {                \
    WindowGlfw* self =                                                   \
        reinterpret_cast<WindowGlfw*>(glfwGetWindowUserPointer(window)); \
    assert(self);                                                        \
    self->Handle##name(__VA_ARGS__);                                     \
  }                                                                      \
                                                                         \
  STRONG_INLINE void Handle##name(__VA_ARGS__);

struct GlfwContextHolder {
  // TODO: consider reimplementing the allocator
  inline GlfwContextHolder() { assert(glfwInit()); }
  inline ~GlfwContextHolder() { glfwTerminate(); }
};

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

  SkCanvas* canvas() { return skia_->canvas(); }

  void QuerySize(int& width, int& height) {
    glfwGetFramebufferSize(window_, &width, &height);
  }

  GLFWwindow* HACK_GETGlfwWindow() { return window_; }

  ui::SkiaContext* context() { return skia_.get(); }

 private:
  static void BindContext();
  void InternalCreateWindow(int w, int h);

  IMPL_GLFW_FORWARDER(Move, int(x), int(y))
  IMPL_GLFW_FORWARDER(Resize, int(w), int(h))
  IMPL_GLFW_FORWARDER(Scale, float(xscale), float(yscale))
  IMPL_GLFW_FORWARDER(KeyInput, int(key), int(scancode), int(action), int(mods))

 private:
  GlfwContextHolder context_owner_;
  GLFWwindow* window_ = nullptr;
  std::unique_ptr<ui::SkiaContext> skia_;
};