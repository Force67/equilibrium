// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Windows platform window implementation.
#pragma once

// window includes windows- he he
#include <Windows.h>
#include <core/SkRect.h>

#include "platform/window.h"

namespace ui {

class MessageMapInterface {
 public:
  // Processes one message from the window's message queue.
  virtual BOOL ProcessWindowMessage(HWND window,
                                    UINT message,
                                    WPARAM w_param,
                                    LPARAM l_param,
                                    LRESULT& result,
                                    DWORD msg_map_id = 0) = 0;
};

class WindowWin final : public Window, MessageMapInterface {
 public:
  // TODO: optional delegate.
  explicit WindowWin();

  WindowWin(const WindowWin&) = delete;
  WindowWin& operator=(const WindowWin&) = delete;

  virtual ~WindowWin();

  void Init(HWND parent, const SkRect& bounds);

  HWND hwnd() const { return hwnd_; }

  // call these before init:
  void set_window_style(DWORD style) { window_style_ = style; }
  DWORD window_style() const { return window_style_; }

  void set_window_ex_style(DWORD style) { window_ex_style_ = style; }
  DWORD window_ex_style() const { return window_ex_style_; }

  // Sets the class style to use. The default is CS_DBLCLKS.
  void set_initial_class_style(UINT class_style) {
    // We dynamically generate the class name, so don't register it globally!
    DCHECK((class_style & CS_GLOBALCLASS), 0u);
    class_style_ = class_style;
  }

  UINT initial_class_style() const { return class_style_; }

 protected:
  virtual LRESULT OnWndProc(UINT message, WPARAM w_param, LPARAM l_param);

  // Call this when subclassed, else you'd might leak the window object.
  void ClearUserData();

 private:
  // The window procedure used by all Windows.
  static LRESULT CALLBACK WndProc(HWND window,
                                  UINT message,
                                  WPARAM w_param,
                                  LPARAM l_param);

  // All classes registered by WindowImpl start with this name.
  static const wchar_t* const kBaseClassName;

  DWORD window_style_ = 0;
  DWORD window_ex_style_ = 0;
  UINT class_style_;
  HWND hwnd_ = nullptr;
};
}  // namespace ui