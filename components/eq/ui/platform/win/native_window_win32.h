// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Windows platform window implementation.
#pragma once

#include <Windows.h>
#include <eq/ui/primitives/rect.h>
#include <eq/ui/primitives/point.h>
#include <eq/ui/platform/native_window.h>

namespace ui {

class WindowDelegateWin : public NativeWindow::Delegate {
 public:
  virtual ~WindowDelegateWin() = default;

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

  bool Init(handle parent,
            const ui::IRect bounds,
            const CreateFlags,
            u8 icon_id = 102) override;
  bool SetTitle(const base::StringRefU8) override;
  void SetDelegate(ui::NativeWindow::Delegate*);

  void SendCommand(Command) override;

  handle os_handle() const override;
  const ui::IRect bounds() const override;

  void SetDelegate(WindowDelegateWin* new_delegate) { delegate_ = new_delegate; }

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
  void HandleWindowResize(const ui::IPoint new_size);
  LRESULT HandleWindowHittest(const ui::IPoint);

  bool ResizeBounds(const ui::IPoint window_pos, const ui::IPoint in_dimension);
  void ExtendClientFrame(RECT&);

  bool IsCustomWindowBorderEnabled() const;

 private:
  DWORD window_style_;
  DWORD window_ex_style_ = 0;
  UINT class_style_;
  HWND hwnd_ = nullptr;
  HMONITOR tracked_monitor_ = nullptr;
  ui::IPoint user_size_{};
  bool request_resize_{false};
  bool is_custom_styled_{false};
  WindowDelegateWin* delegate_;
};
}  // namespace ui