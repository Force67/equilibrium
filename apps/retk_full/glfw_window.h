// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Example window presentation based on GLFW
#pragma once

#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>

// This class is named 'WindowGlfw' so we can avoid collisions with
// glfw's window struct name
class WindowGlfw {
 public:
  explicit WindowGlfw(int width, int height);
  ~WindowGlfw();

  private:
  void Configure();

 private:
  GLFWwindow* window_ = nullptr;
};
