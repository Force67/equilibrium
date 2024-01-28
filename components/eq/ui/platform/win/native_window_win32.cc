// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Windows platform window implementation.

#include <base/check.h>
#include <base/atomic.h>
#include <base/logging.h>
#include <base/containers/bitsets/bitset.h>

#include <base/text/code_convert.h>

#include <ui/display/dpi.h>
#include <ui/display/win/dpi_win.h>
#include "native_window_win32.h"

#include <Windows.h>
#include <VersionHelpers.h>

#include <Uxtheme.h>
// only for vsync, not necessary in copies:
#include <dwmapi.h>

#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "UxTheme.lib")

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

namespace eq::ui {

namespace {
base::Atomic<u32> window_count = 0;

constexpr DWORD kWindowDefaultChildStyle =
    WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
constexpr DWORD kWindowDefaultStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;

void* SetWindowUserData(HWND hwnd, void* user_data) {
  return reinterpret_cast<void*>(SetWindowLongPtrW(
      hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(user_data)));
}

void* GetWindowUserData(HWND hwnd) {
  DWORD process_id = 0;
  GetWindowThreadProcessId(hwnd, &process_id);
  // A window outside the current process needs to be ignored.
  if (process_id != ::GetCurrentProcessId())
    return nullptr;
  return reinterpret_cast<void*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
}

HWND GetWindowToParentTo(bool get_real_hwnd) {
  return get_real_hwnd ? ::GetDesktopWindow() : HWND_DESKTOP;
}

inline HWND TranslateHandle(NativeWindow::handle native_window_handle) {
  return static_cast<HWND>(native_window_handle);
}

i32 HitTest(i32 x, i32 y, RECT rect) {
  return ((rect.left <= x && x < rect.right) &&
          (rect.top <= y && y < rect.bottom));
}

BOOL IsWindows10BuildOrGreaterWin32(WORD build) {
  OSVERSIONINFOEXW osvi = {sizeof(osvi), 10, 0, build};
  DWORD mask = VER_MAJORVERSION | VER_MINORVERSION | VER_BUILDNUMBER;
  ULONGLONG cond = VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL);
  cond = VerSetConditionMask(cond, VER_MINORVERSION, VER_GREATER_EQUAL);
  cond = VerSetConditionMask(cond, VER_BUILDNUMBER, VER_GREATER_EQUAL);
  // HACK: Use RtlVerifyVersionInfo instead of VerifyVersionInfoW as the
  //       latter lies unless the user knew to embed a non-default manifest
  //       announcing support for Windows 10 via supportedOS GUID
  return VerifyVersionInfoW(&osvi, mask, cond) == 0;
}

bool IsWindows10AnniversaryOrGreater() {
  // Build 1607O
  return IsWindows10BuildOrGreaterWin32(14393);
}

bool IsWindowsCreatorsOrGreater() {
  // Build 15063
  return IsWindows10BuildOrGreaterWin32(15063);
}

// This function scales the window up from its desired viewport_size
void ScaleWindowSize(DWORD style,
                     DWORD exStyle,
                     i32 contentWidth,
                     i32 contentHeight,
                     i32& fullWidth,
                     i32& fullHeight,
                     UINT dpi) {
  RECT rect{0, 0, contentWidth, contentHeight};

  if (IsWindows10AnniversaryOrGreater())
    AdjustWindowRectExForDpi(&rect, style, FALSE, exStyle, dpi);
  else
    AdjustWindowRectEx(&rect, style, FALSE, exStyle);

  fullWidth = rect.right - rect.left;
  fullHeight = rect.bottom - rect.top;
}

constexpr wchar_t kWindowClassName[] = L"vhtNativeWindow";

int caption_width = 30;
int border_width = 10;

#define MAX_EMBEDDED_WIDGETS 128
int embedded_widget_count = 0;
RECT embedded_widget_rect[MAX_EMBEDDED_WIDGETS];
void EmbeddedWidgetRect(RECT rect) {
  if (embedded_widget_count < MAX_EMBEDDED_WIDGETS) {
    embedded_widget_rect[embedded_widget_count] = rect;
    embedded_widget_count += 1;
  }
}
}  // namespace

NativeWindowWin32::NativeWindowWin32(base::StringRefU8 name,
                                     WindowDelegateWin* delegate)
    : class_style_(CS_DBLCLKS),
      window_style_(WS_OVERLAPPEDWINDOW),
      delegate_(delegate),
      ui::NativeWindow(name) {}

NativeWindowWin32::~NativeWindowWin32() {
  ClearUserData();
}

void NativeWindowWin32::ClearUserData() {
  if (::IsWindow(hwnd_))
    SetWindowUserData(hwnd_, nullptr);
}

// static
LRESULT CALLBACK NativeWindowWin32::WndProc(HWND hwnd,
                                            UINT message,
                                            WPARAM w_param,
                                            LPARAM l_param) {
  NativeWindowWin32* window = nullptr;
  if (message == WM_NCCREATE) {
    const auto* cs = reinterpret_cast<CREATESTRUCTW*>(l_param);
    window = reinterpret_cast<NativeWindowWin32*>(cs->lpCreateParams);
    DCHECK(window);
    SetWindowUserData(hwnd, window);
    // for release
    if (window)
      window->hwnd_ = hwnd;
  } else {
    window = reinterpret_cast<NativeWindowWin32*>(GetWindowUserData(hwnd));
  }

  if (!window)
    return 0;

  return window->ProcessMessage(hwnd, message, w_param, l_param);
}

LRESULT NativeWindowWin32::ProcessMessage(HWND a_hwnd,
                                          UINT message,
                                          WPARAM w_param,
                                          LPARAM l_param) {
  LRESULT result = 0;

  switch (message) {
    case WM_DWMCOMPOSITIONCHANGED: {
      BUGCHECK(false,
               "Invalid DWM message. Windows 7 and below are unsupported.");
      break;
    }
    case WM_NCCALCSIZE: {
      if (!is_custom_styled_)
        break;

      RECT& r = *reinterpret_cast<RECT*>(l_param);
      ExtendClientFrame(r);
      break;
    }
    case WM_NCACTIVATE: {
      // TODO(Vince): this is a nice way of testing if we were minimized
      break;
    }
    case WM_NCHITTEST: {
      if (!is_custom_styled_)
        break;

      result =
          HandleWindowHittest({GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param)});
      break;
    }
    case WM_PAINT: {
      break;
    }
    case WM_NCDESTROY:
      hwnd_ = nullptr;
      break;
    case WM_QUIT:
    case WM_DESTROY: {
      HandleDestroy();
      break;
    }
    case WM_MOVE:
      HandleWindowMove();
      break;
    case WM_SIZING:
      ::InvalidateRect(hwnd_, nullptr, FALSE);
      break;
    case WM_SIZE: {
#if 0
      if (!request_resize_) {
        HandleWindowResize({LOWORD(l_param), HIWORD(l_param)});
      } else
        request_resize_ = false;
#endif
      break;
    }
    default:
      break;
  }

  // Handle the message if it's in our message map; otherwise, let the system
  // handle it.
  if (!delegate_ || !delegate_->ProcessWindowMessage(a_hwnd, message, w_param,
                                                     l_param, result))
    result = DefWindowProcW(a_hwnd, message, w_param, l_param);

  return result;
}

void NativeWindowWin32::HandleWindowMove() {
  // we changed displays.
  HMONITOR current_mon = ui::GetCurrentMonitorHandle(hwnd_);
  if (current_mon != tracked_monitor_) {
    dpi_ = ui::GetMonitorDpi(current_mon).x;
    tracked_monitor_ = current_mon;
    // re-scale based on the actual desired bounds instead of the
    // current(scaled) bounds in order to avoid infinite scaling.
    const auto b = bounds();
    ResizeBounds({b.x(), b.y()}, user_size_);

    // fire dpi change event so UI can react.
    if (delegate_)
      delegate_->OnDisplayChanged(hwnd_);
  }
}

void NativeWindowWin32::HandleWindowResize(const ui::IPoint new_size) {
  // TODO: does this even make much sense, no!
  if (new_size != user_size_) {
    user_size_ = new_size;
  }
}

LRESULT NativeWindowWin32::HandleWindowHittest(const ui::IPoint pos) {
  // Make sure the point is inside of the window
  RECT frame_rect{};
  ::GetWindowRect(hwnd_, &frame_rect);
  if (!HitTest(pos.x, pos.y, frame_rect))
    return HTNOWHERE;

  RECT rect{};
  ::GetClientRect(hwnd_, &rect);

  POINT p{.x = pos.x, .y = pos.y};
  ::ScreenToClient(hwnd_, &p);

  // Check each border
  bool l = false;
  bool r = false;
  bool b = false;
  bool t = false;
  if (!::IsZoomed(hwnd_)) {
    if (rect.left <= pos.x && pos.x < rect.left + border_width)
      l = true;  // left
    if (rect.right - border_width <= pos.x && pos.x < rect.right)
      r = true;  // right
    if (rect.bottom - border_width <= pos.y && pos.y < rect.bottom)
      b = true;  // bottom
    if (rect.top <= pos.y && pos.y < rect.top + border_width)
      t = true;  // top
  }

  // If the point is in two borders, use the corresponding corner resize.
  // If the point is in just one border, use the corresponding side
  // resize.

  LRESULT result = 0;
  if (l) {
    if (t) {
      result = HTTOPLEFT;
    } else if (b) {
      result = HTBOTTOMLEFT;
    } else {
      result = HTLEFT;
    }
  } else if (r) {
    if (t) {
      result = HTTOPRIGHT;
    } else if (b) {
      result = HTBOTTOMRIGHT;
    } else {
      result = HTRIGHT;
    }
  } else if (t) {
    result = HTTOP;
  } else if (b) {
    result = HTBOTTOM;
  }

  // Here the point must be further inside the window than the resize
  // borders, so the final options are the window moving area (caption)
  // and the client area.
  else {
    if (rect.top <= pos.y && pos.y < rect.top + caption_width) {
      result = HTCAPTION;
      // Check the application defined widget areas
      for (int i = 0; i < embedded_widget_count; i += 1) {
        if (HitTest(pos.x, pos.y, embedded_widget_rect[i])) {
          result = HTCLIENT;
          break;
        }
      }
    } else {
      result = HTCLIENT;
    }
  }

  return result;
}

void NativeWindowWin32::HandleDestroy() {
  BUGCHECK(window_count > 0, "Invalid construction to destruction ratio");

  if (window_count == 1) {
    // last man standing, exit the message loop, we need to verify the window
    // count in order to make sure we don't exit while other windows still might
    // be active.
    ::PostQuitMessage(0);
    //::SendMessageW(hwnd_, WM_QUIT, 0, 0);
    //::PostMessageW(hwnd_, WM_QUIT, 0, 0);
  }
  window_count--;
}

bool NativeWindowWin32::Init(handle parent_handle,
                             const eq::ui::IRect suggested_bounds,
                             const CreateFlags flags,
                             u8 icon_id) {
  // Note  As of Windows 8, DWM composition is always enabled. If an app
  // declares Windows 8 compatibility in their manifest, this function will
  // receive a value of TRUE through pfEnabled. If no such manifest entry is
  // found, Windows 8 compatibility is not assumed and this function receives a
  // value of FALSE through pfEnabled. This is done so that older programs that
  // interpret a value of TRUE to imply that high contrast mode is off can
  // continue to make the correct decisions about rendering their images. (Note
  // that this is a bad practice�you should use the SystemParametersInfo
  // function with the SPI_GETHIGHCONTRAST flag to determine the state of high
  // contrast mode.)
  BOOL compositon_enabled = FALSE;
  ::DwmIsCompositionEnabled(&compositon_enabled);
  BUGCHECK(
      compositon_enabled,
      "Composition isn't enabled. Win <= 8 isn't supported anymore. Enable "
      "Windows8 compatability in manifest!");

  LOG_INFO(
      "Warning: VSYNC might be enabled though the DWMCompositionEnabled, make "
      "sure "
      "it isnt killing perf?");

  HWND parent = TranslateHandle(parent_handle);

  if (suggested_bounds.width() == 0 || suggested_bounds.height() == 0) {
    LOG_DEBUG(
        "Win32Window: Invalid rect was specified: could lead to negative "
        "bounds "
        "(invisible window). Terminating init!");
    return false;
  }

  if (window_style_ == 0)
    window_style_ = parent ? kWindowDefaultChildStyle : kWindowDefaultStyle;

  if ((flags & NativeWindow::CreateFlags::kCustomBorder) &&
      (window_style_ & kWindowDefaultStyle)) {
    is_custom_styled_ = true;
    window_style_ = 0x80000000;  // WS_POPUP
    LOG_DEBUG(
        "Win32Window: fancy border is enabled. OS Window border handling is "
        "disabled.");
  }

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

  eq::ui::IRect bounds;
  if (suggested_bounds.empty())
    bounds = {CW_USEDEFAULT};
  else
    bounds = suggested_bounds;

  // guard the class registration against being called twice
  // TODO: fix this hack.

  if (window_count == 0) {
    // this icon stuff is questionable.
    HICON icon_handle = icon_id && !is_custom_styled_
                            ? ::LoadIconW(::GetModuleHandleW(nullptr),
                                          MAKEINTRESOURCEW(icon_id))
                            : nullptr;
    const WNDCLASSEXW wc{
        .cbSize = sizeof(wc),
        .style = /*CS_VREDRAW | CS_HREDRAW*/ 0,
        .lpfnWndProc = WndProc,
        .cbClsExtra = 0,
        .cbWndExtra = 0,
        .hInstance = nullptr,
        .hIcon = icon_handle,
        .lpszClassName = kWindowClassName,
    };

    BUGCHECK(RegisterClassExW(&wc));
  }
  window_count++;

  // ATOM atom = GetWindowClassAtom();
  auto wide_name = base::UTF8ToWide(title_);
  hwnd_ =
      ::CreateWindowExW(window_ex_style_, kWindowClassName, wide_name.c_str(),
                        window_style_, bounds.x(), bounds.y(), bounds.width(),
                        bounds.height(), parent, nullptr, nullptr, this);
  if (!hwnd_)
    return false;

  // The other key aspect is SetWindowTheme. Using SetWindowTheme with single
  // spaces effectively disables an theme on the window. Doing this is critical
  // for removing special shapes, shading, etc. from the default window on
  // any given version of Windows.
  // This will make the window look like:
  // https://media.discordapp.net/attachments/598919611118911488/1022557987509903470/unknown.png?width=1463&height=845
  if (is_custom_styled_)
    ::SetWindowTheme(hwnd_, L" ", L" ");

  const DWORD create_window_error = ::GetLastError();
  if (hwnd_ && (window_style_ & WS_CAPTION)) {
    ::SetWindowPos(hwnd_, nullptr, 0, 0, 0, 0,
                   SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                       SWP_NOACTIVATE | SWP_NOREDRAW);
  }

  // store the monitor we were spawned from
  BUGCHECK(tracked_monitor_ = ui::GetCurrentMonitorHandle(hwnd_));
  user_size_ = {bounds.width(), bounds.height()};
  dpi_ = ui::GetMonitorDpi(tracked_monitor_).x;
  // this is only done now since we have to wait for the window to be placed in
  // order to fetch DPI
  ResizeBounds({bounds.x(), bounds.y()}, user_size_);

  return true;
}

bool NativeWindowWin32::SetTitle(const base::StringRefU8 title_name) {
  auto wide_name = base::UTF8ToWide(title_name);
  if (::SetWindowTextW(hwnd_, wide_name.c_str())) {
    title_ = title_name.data();
    return true;
  }
  return false;
}

void NativeWindowWin32::SetDelegate(ui::NativeWindow::Delegate* delegate) {
  delegate_ = reinterpret_cast<WindowDelegateWin*>(delegate);
}

void NativeWindowWin32::SendCommand(Command command) {
  int cmd_show;
  switch (command) {
    case Command::kHide:
      cmd_show = SW_SHOW;
      break;
    case Command::kNormal:
      cmd_show = SW_NORMAL;
      break;
    case Command::kMinimized:
      cmd_show = SW_SHOWMINIMIZED;
      break;
    case Command::kMaximized:
      cmd_show = SW_SHOWMAXIMIZED;
      break;
    case Command::kMaximize:
      cmd_show = SW_MAXIMIZE;
      break;
    case Command::kShowNoActivate:
      cmd_show = SW_SHOWNOACTIVATE;
      break;
    case Command::kShow:
      cmd_show = SW_SHOW;
      break;
    case Command::kMinimize:
      cmd_show = SW_MINIMIZE;
      break;
    case Command::kShowMinnoActive:
      cmd_show = SW_SHOWMINNOACTIVE;
      break;
    case Command::kRestore:
      cmd_show = SW_RESTORE;
      break;
    case Command::kShowDefault:
      cmd_show = SW_SHOWDEFAULT;
      break;
    case Command::kForceMinimize:
      cmd_show = SW_FORCEMINIMIZE;
      break;
    case Command::kMax:
      cmd_show = SW_MAX;
      break;
    default:
      return;
  }

  // If the window was previously visible, the return value is nonzero.
  // If the window was previously hidden, the return value is zero.
  ::ShowWindow(hwnd_, cmd_show);
}

NativeWindowWin32::handle NativeWindowWin32::os_handle() const {
  return hwnd_;
}

const eq::ui::IRect NativeWindowWin32::bounds() const {
  RECT r{};
  ::GetWindowRect(hwnd_, &r);
  return {r.left, r.top, r.right, r.bottom};
}

bool NativeWindowWin32::ResizeBounds(const ui::IPoint window_pos,
                                     const ui::IPoint in_dimension) {
  DCHECK(dpi_ > 95.f,
         "A dpi value must be provided to ensure the same size across all "
         "monitors");
  // Manually do what EnableNonClientDpiScaling() would do, so we are compatible
  // with all versions of windows.
  f32 dpi_factor = GetDPIFactor(dpi_).x;
  const ui::IPoint scaled_bounds = {
      static_cast<i32>(in_dimension.x * dpi_factor),
      static_cast<i32>(in_dimension.y * dpi_factor)};

  // what we get out is the new desired size, but what if we update that only
  // after the
  i32 new_width = 0, new_height = 0;
  ScaleWindowSize(window_style_, window_ex_style_, scaled_bounds.x,
                  scaled_bounds.y, new_width, new_height,
                  static_cast<UINT>(dpi_));

  const auto bounds = eq::ui::IRect::MakeXYWH(window_pos.x, window_pos.y,
                                              new_width, new_height);

  request_resize_ = true;
  return ::MoveWindow(hwnd_, bounds.x(), bounds.y(), bounds.width(),
                      bounds.height(), TRUE);
}

void NativeWindowWin32::ExtendClientFrame(RECT& r) {
  MARGINS margins = {};
  // A convenient function for checking if a window is maximized.
  if (::IsZoomed(hwnd_)) {
    int x_push_in =
        ::GetSystemMetrics(SM_CXFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER);
    int y_push_in =
        ::GetSystemMetrics(SM_CYFRAME) + ::GetSystemMetrics(SM_CXPADDEDBORDER);

    r.left += x_push_in;
    r.top += y_push_in;
    r.bottom -= x_push_in;
    r.right -= y_push_in;

    margins.cxLeftWidth = margins.cxRightWidth = x_push_in;
    margins.cyTopHeight = margins.cyBottomHeight = y_push_in;
  }

  ::DwmExtendFrameIntoClientArea(hwnd_, &margins);
}

bool NativeWindowWin32::IsCustomWindowBorderEnabled() const {
  return is_custom_styled_;
}
}  // namespace eq::ui