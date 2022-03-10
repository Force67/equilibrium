// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Windows platform window implementation.

#include <base/check.h>
#include <base/text/code_convert.h>

#include "native_window_win.h"
#include <Windows.h>

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

constexpr wchar_t kWindowClassName[] = L"vhtNativeWindow";
}  // namespace

WindowWin::WindowWin(base::StringRef name, WindowDelegateWin* delegate)
    : class_style_(CS_DBLCLKS),
      window_style_(WS_OVERLAPPEDWINDOW),
      delegate_(delegate),
      ui::NativeWindow(name) {}

WindowWin::~WindowWin() {
  ClearUserData();
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
    const auto* cs = reinterpret_cast<CREATESTRUCTW*>(l_param);
    window = reinterpret_cast<WindowWin*>(cs->lpCreateParams);
    DCHECK(window);
    SetWindowUserData(hwnd, window);
    window->hwnd_ = hwnd;
  } else {
    window = reinterpret_cast<WindowWin*>(GetWindowUserData(hwnd));
  }

  if (!window)
    return 0;

  return window->ProcessMessage(hwnd, message, w_param, l_param);
}

LRESULT WindowWin::ProcessMessage(HWND a_hwnd,
                                  UINT message,
                                  WPARAM w_param,
                                  LPARAM l_param) {
  LRESULT result = 0;

  HWND hwnd = hwnd_;
  if (message == WM_NCDESTROY)
    hwnd_ = nullptr;

  // Handle the message if it's in our message map; otherwise, let the system
  // handle it.
  if (!delegate_ ||
      !delegate_->ProcessWindowMessage(hwnd, message, w_param, l_param, result))
    result = DefWindowProcW(hwnd, message, w_param, l_param);

  return result;
}

void WindowWin::Init(HWND parent, const SkRect& bounds) {
  if (window_style_ == 0)
    window_style_ = parent ? kWindowDefaultChildStyle : kWindowDefaultStyle;

  if (parent == HWND_DESKTOP) {
    // Only non-child windows can have HWND_DESKTOP (0) as their parent.
    BUGCHECK((window_style_ & WS_CHILD) == 0);
    parent = GetWindowToParentTo(false);
  } else if (parent == ::GetDesktopWindow()) {
    // Any type of window can have the "Desktop Window" as their parent.
    parent = GetWindowToParentTo(true);
  } else if (parent != HWND_MESSAGE) {
    DCHECK(::IsWindow(parent));
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

  WNDCLASSEXW wc{};
  wc.cbSize = sizeof(wc);
  wc.style = 3;
  wc.lpfnWndProc = WndProc;
  wc.hInstance = nullptr;
  wc.lpszClassName = kWindowClassName;
  RegisterClassExW(&wc);
  // TODO: some dchecks.

  auto wide_name = base::ASCIIToWide(title_);
  // ATOM atom = GetWindowClassAtom();
  hwnd_ = CreateWindowExW(window_ex_style_, kWindowClassName,
                          wide_name.c_str(), window_style_, x, y, width, height,
                          parent, nullptr, nullptr, this);
  const DWORD create_window_error = ::GetLastError();

  if (hwnd_ && (window_style_ & WS_CAPTION)) {
    SetWindowPos(hwnd_, nullptr, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOACTIVATE | SWP_NOREDRAW);
  }

  if (!hwnd_ && create_window_error == 0) {
    WNDCLASSEX class_info;
    memset(&class_info, 0, sizeof(WNDCLASSEX));
    class_info.cbSize = sizeof(WNDCLASSEX);
    BOOL got_class = GetClassInfoExW(GetModuleHandleW(nullptr),
                                     kWindowClassName, &class_info);
  }
}

void WindowWin::Show() {
  ShowWindow(hwnd_, SW_SHOW);
}
}  // namespace ui