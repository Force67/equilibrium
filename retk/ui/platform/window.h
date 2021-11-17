// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Platform window.
#pragma once

namespace ui {
class Window {
 public:
  inline Window(const std::string_view title) : title_(title) {}

  virtual void SetTitle(const std::string_view title);

 protected:
  std::string title_;
};
}