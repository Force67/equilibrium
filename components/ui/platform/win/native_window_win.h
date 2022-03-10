// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Windows platform window implementation.
#pragma once

#include <core/SkRect.h>
#include <base/win/minwin.h>
#include <ui/platform/native_window.h>

namespace ui {

class WindowDelegateWin {
 public:
  // Processes one message from the window's message queue.
  virtual BOOL ProcessWindowMessage(HWND window,
                                    UINT message,
                                    WPARAM w_param,
                                    LPARAM l_param,
                                    LRESULT& result,
                                    DWORD msg_map_id = 0) = 0;
};

class WindowWin final : public ui::NativeWindow {
 public:
  explicit WindowWin(base::StringRef name,
                     WindowDelegateWin* delegate = nullptr);
  ~WindowWin();

  void Init(HWND parent, const SkRect& bounds);

  void Show() override;

 private:
  // static item.
  static LRESULT CALLBACK WndProc(HWND window,
                                  UINT message,
                                  WPARAM w_param,
                                  LPARAM l_param);
  LRESULT ProcessMessage(HWND, UINT, WPARAM, LPARAM);

  void ClearUserData();

 private:
  DWORD window_style_;
  DWORD window_ex_style_ = 0;
  UINT class_style_;
  HWND hwnd_ = nullptr;
  WindowDelegateWin* delegate_;
};
}  // namespace ui