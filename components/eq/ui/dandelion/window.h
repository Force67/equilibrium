// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "key.h"

// https://github.com/google/skia/blob/main/tools/sk_app/VulkanWindowContext.h
// https://github.com/google/skia/blob/main/example/HelloWorld.cpp
// https://github.com/skiftOS/skift/blob/82ac3e77cb52c46b3982e60bda7a3679e0d0c211/userspace/libs/libwidget/Window.cpp

namespace ui {

// https://github.com/google/skia/blob/main/tools/sk_app/VulkanWindowContext.cpp

class Window {
 public:
  virtual ~Window() = default;

  #if 0
  virtual void show() = 0;

  // update
  virtual void Update() = 0;

  void Paint(SkSurface&);

  // events
  virtual bool OnChar(SkUnichar c, ModifierKey) { return false; }
  virtual bool OnKey(Key key, InputState, ModifierKey) { return false; }
  virtual bool OnMouseMove(base::Vec2i coords, InputState, ModifierKey) {
    return false;
  }
  virtual bool OnMouseScroll(float delta, ModifierKey) { return false; }

  // core root
  Item* root() { return root_item_; }

 protected:
  base::Vec2i bound_{250, 250};
  bool in_focus_ = true;
  Item* root_item_;
  #endif
};
}  // namespace ui