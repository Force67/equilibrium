// Copyright (C) 2022 Vincent Hengel.
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

  virtual void OnDisplayChanged(NativeWindow::handle native_handle) = 0;
};

class NativeWindowWin32 final : public ui::NativeWindow {
 public:
  explicit NativeWindowWin32(base::StringRefU8 name,
                     WindowDelegateWin* delegate = nullptr);
  ~NativeWindowWin32();

  bool Init(handle parent, const SkIRect bounds) override;
  bool SetTitle(const base::StringRefU8) override;

  void SendCommand(Command) override;

  const SkIRect bounds() const override;

 private:
  // static item.
  static LRESULT CALLBACK WndProc(HWND window,
                                  UINT message,
                                  WPARAM w_param,
                                  LPARAM l_param);
  LRESULT ProcessMessage(HWND, UINT, WPARAM, LPARAM);

  void ClearUserData();

  void HandleDestroy();
  void HandleWindowMove();
  void HandleWindowResize(const SkIPoint new_size);

  bool ResizeBounds(const SkIPoint window_pos, const SkIPoint in_dimension);
 private:
  DWORD window_style_;
  DWORD window_ex_style_ = 0;
  UINT class_style_;
  HWND hwnd_ = nullptr;
  HMONITOR tracked_monitor_ = nullptr;
  SkIPoint user_size_{};
  bool request_resize_{false};
  WindowDelegateWin* delegate_;
};
}  // namespace ui