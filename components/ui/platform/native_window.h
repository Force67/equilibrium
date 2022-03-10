// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Platform window.
#pragma once

#include <base/xstring.h>

namespace ui {
class NativeWindow {
 public:
  inline NativeWindow(const base::StringRef title) : title_(title) {}
  inline NativeWindow() : title_("<unknown>") {}

  // optional
  virtual void SetTitle(const base::StringRef title){};

  virtual void Show() = 0;

 protected:
  base::String title_;
};
}  // namespace ui