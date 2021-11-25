// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Windows platform window implementation.

#include <base/check.h>
#include "window_win.h"

namespace ui {
namespace {
constexpr DWORD kWindowDefaultChildStyle =
    WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
constexpr DWORD kWindowDefaultStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;

void* SetWindowUserData(HWND hwnd, void* user_data) {
  return reinterpret_cast<void*>(SetWindowLongPtr(
      hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(user_data)));
}

void* GetWindowUserData(HWND hwnd) {
  DWORD process_id = 0;
  GetWindowThreadProcessId(hwnd, &process_id);
  // A window outside the current process needs to be ignored.
  if (process_id != ::GetCurrentProcessId())
    return NULL;
  return reinterpret_cast<void*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}

HWND GetWindowToParentTo(bool get_real_hwnd) {
  return get_real_hwnd ? ::GetDesktopWindow() : HWND_DESKTOP;
}
}  // namespace

WindowWin::WindowWin() : Window(""), class_style_(CS_DBLCLKS) {}

WindowWin::~WindowWin() {
  ClearUserData();
}

void WindowWin::Init(HWND parent, const SkRect& bounds) {
  if (window_style_ == 0)
    window_style_ = parent ? kWindowDefaultChildStyle : kWindowDefaultStyle;

  if (parent == HWND_DESKTOP) {
    // Only non-child windows can have HWND_DESKTOP (0) as their parent.
    TK_BUGCHECK((window_style_ & WS_CHILD) == 0);
    parent = GetWindowToParentTo(false);
  } else if (parent == ::GetDesktopWindow()) {
    // Any type of window can have the "Desktop Window" as their parent.
    parent = GetWindowToParentTo(true);
  } else if (parent != HWND_MESSAGE) {
    TK_DCHECK(::IsWindow(parent));
  }

  int x, y, width, height;
  if (bounds.isEmpty()) {
    x = y = width = height = CW_USEDEFAULT;
  } else {
    x = bounds.x();
    y = bounds.y();
    width = bounds.width();
    height = bounds.height();
  }

  //ATOM atom = GetWindowClassAtom();
  ATOM atom = 0;
  HWND hwnd = CreateWindowEx(window_ex_style_, reinterpret_cast<wchar_t*>(atom),
                             NULL, window_style_, x, y, width, height, parent,
                             NULL, NULL, this);
  const DWORD create_window_error = ::GetLastError();

  if (hwnd && (window_style_ & WS_CAPTION)) {
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOACTIVATE | SWP_NOREDRAW);
  }

  if (!hwnd_ && create_window_error == 0) {
    WNDCLASSEX class_info;
    memset(&class_info, 0, sizeof(WNDCLASSEX));
    class_info.cbSize = sizeof(WNDCLASSEX);
    BOOL got_class = GetClassInfoEx(
        GetModuleHandle(NULL), reinterpret_cast<wchar_t*>(atom), &class_info);
  }
}

LRESULT WindowWin::OnWndProc(UINT message, WPARAM w_param, LPARAM l_param) {
  LRESULT result = 0;

  HWND hwnd = hwnd_;
  if (message == WM_NCDESTROY)
    hwnd_ = nullptr;

  // Handle the message if it's in our message map; otherwise, let the system
  // handle it.
  if (!ProcessWindowMessage(hwnd, message, w_param, l_param, result))
    result = DefWindowProc(hwnd, message, w_param, l_param);

  return result;
}

void WindowWin::ClearUserData() {
  if (::IsWindow(hwnd_))
    SetWindowUserData(hwnd_, nullptr);
}

// static
LRESULT CALLBACK WindowWin::WndProc(HWND hwnd,
                                    UINT message,
                                    WPARAM w_param,
                                    LPARAM l_param) {
  WindowWin* window = nullptr;
  if (message == WM_NCCREATE) {
    CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(l_param);
    window = reinterpret_cast<WindowWin*>(cs->lpCreateParams);
    TK_DCHECK(window);
    SetWindowUserData(hwnd, window);
    window->hwnd_ = hwnd;
  } else {
    window = reinterpret_cast<WindowWin*>(GetWindowUserData(hwnd));
  }

  if (!window)
    return 0;

  return window->OnWndProc(message, w_param, l_param);
}
}  // namespace ui