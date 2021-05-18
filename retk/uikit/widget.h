// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

class SkCanvas;

#include <uikit/keycode.inl>

namespace uikit {

class Widget {
 public:
  Widget();
  ~Widget();

  Widget(const Widget&) = delete;
  Widget& operator=(const Widget&) = delete;

  // must be implemented
  virtual void Draw(SkCanvas* canvas) = 0;
  // key message.
  virtual void OnInput(Key keycode){};

 private:
  float width_ = 0.f;
  float height_ = 0.f;
  bool visible_{false};
};
}