// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Example window presentation based on GLFW

#include <cstdio>
#include "glfw_window.h"

static void error_callback(int error, const char* description) {
  fputs(description, stderr);
}

WindowGlfw::WindowGlfw(int width, int height) {
  if (!glfwInit()) {
    exit(EXIT_FAILURE);
  }

  window_ = glfwCreateWindow(width, height, "ReTK", nullptr, nullptr);
}

WindowGlfw::~WindowGlfw() {
  if (window_)
    glfwDestroyWindow(window_);
}

void WindowGlfw::Configure() {
  glfwSetErrorCallback(error_callback);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //(uncomment to enable correct color spaces) glfwWindowHint(GLFW_SRGB_CAPABLE,
  // GL_TRUE);
  glfwWindowHint(GLFW_STENCIL_BITS, 0);
  // glfwWindowHint(GLFW_ALPHA_BITS, 0);
  glfwWindowHint(GLFW_DEPTH_BITS, 0);

  // ==================
  // When switching dpi contexts, make the window automatically aware of the new
  // desired size.
  glfwWindowHint(GLFW_SCALE_TO_MONITOR, 1);
}