// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Windows platform window implementation.

#include <base/check.h>
#include <base/text/code_convert.h>

#include <ui/display/dpi.h>
#include <ui/display/win/dpi_win.h>
#include <ui/platform/win/native_window_win32.h>

#include <Windows.h>
#include <VersionHelpers.h>

namespace ui {

namespace {
constexpr DWORD kWindowDefaultChildStyle =
    WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
constexpr DWORD kWindowDefaultStyle = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;

void* SetWindowUserData(HWND hwnd, void* user_data) {
  return reinterpret_cast<void*>(
      SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(user_data)));
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
    case WM_NCDESTROY:
      hwnd_ = nullptr;
      break;
    case WM_QUIT:
    case WM_DESTROY: {
      // TODO(Force): evaluate if this makes that much sense, since this in its
      // current form
      // would mean, that the whole process dies for every closed window...
      PostQuitMessage(0);
      break;
    }
    case WM_MOVE:
      HandleWindowMove();
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
  if (!delegate_ ||
      !delegate_->ProcessWindowMessage(a_hwnd, message, w_param, l_param, result))
    result = DefWindowProcW(a_hwnd, message, w_param, l_param);

  return result;
}

void NativeWindowWin32::HandleWindowMove() {
  // we changed displays.
  HMONITOR current_mon = ui::GetCurrentMonitorHandle(hwnd_);
  if (current_mon != tracked_monitor_) {
    dpi_ = ui::GetMonitorDpi(current_mon).x();
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

void NativeWindowWin32::HandleWindowResize(const SkIPoint new_size) {
  // TODO: does this even make much sense, no!
  if (new_size != user_size_) {
    user_size_ = new_size;
  }
}

bool NativeWindowWin32::Init(handle parent_handle, const SkIRect suggested_bounds) {
  HWND parent = TranslateHandle(parent_handle);

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

  /*
     int32_t fLeft;   //!< smaller x-axis bounds
    int32_t fTop;    //!< smaller y-axis bounds
    int32_t fRight;  //!< larger x-axis bounds
    int32_t fBottom; //!< larger y-axis bounds
  */

  SkIRect bounds{};
  if (suggested_bounds.isEmpty())
    bounds = {CW_USEDEFAULT};
  else
    bounds = suggested_bounds;

  const WNDCLASSEXW wc{
      .cbSize = sizeof(wc),
      .style = CS_VREDRAW | CS_HREDRAW,
      .lpfnWndProc = WndProc,
      .hInstance = nullptr,
      .lpszClassName = kWindowClassName,
  };
  BUGCHECK(RegisterClassExW(&wc));

  // ATOM atom = GetWindowClassAtom();
  auto wide_name = base::UTF8ToWide(title_);
  hwnd_ = ::CreateWindowExW(window_ex_style_, kWindowClassName, wide_name.c_str(),
                            window_style_, bounds.x(), bounds.y(), bounds.width(),
                            bounds.height(), parent, nullptr, nullptr, this);
  if (!hwnd_)
    return false;

  const DWORD create_window_error = ::GetLastError();
  if (hwnd_ && (window_style_ & WS_CAPTION)) {
    ::SetWindowPos(hwnd_, nullptr, 0, 0, 0, 0,
                   SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                       SWP_NOACTIVATE | SWP_NOREDRAW);
  }

  // store the monitor we were spawned from
  BUGCHECK(tracked_monitor_ = ui::GetCurrentMonitorHandle(hwnd_));
  user_size_ = {bounds.width(), bounds.height()};
  dpi_ = ui::GetMonitorDpi(tracked_monitor_).x();
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

  ::ShowWindow(hwnd_, cmd_show);
}

const SkIRect NativeWindowWin32::bounds() const {
  RECT r{};
  ::GetWindowRect(hwnd_, &r);
  return {r.left, r.top, r.right, r.bottom};
}

bool NativeWindowWin32::ResizeBounds(const SkIPoint window_pos,
                                     const SkIPoint in_dimension) {
  DCHECK(dpi_ > 95.f,
         "A dpi value must be provided to ensure the same size across all "
         "monitors");
  // Manually do what EnableNonClientDpiScaling() would do, so we are compatible
  // with all versions of windows.
  SkScalar dpi_factor = GetDPIFactor(dpi_);
  const SkIPoint scaled_bounds = {static_cast<i32>(in_dimension.x() * dpi_factor),
                                  static_cast<i32>(in_dimension.y() * dpi_factor)};

  // what we get out is the new desired size, but what if we update that only after
  // the
  i32 new_width = 0, new_height = 0;
  ScaleWindowSize(window_style_, window_ex_style_, scaled_bounds.x(),
                  scaled_bounds.y(), new_width, new_height, static_cast<UINT>(dpi_));

  const auto bounds =
      SkIRect::MakeXYWH(window_pos.x(), window_pos.y(), new_width, new_height);

  request_resize_ = true;
  return ::MoveWindow(hwnd_, bounds.x(), bounds.y(), bounds.width(), bounds.height(),
                      TRUE);
}
}  // namespace ui