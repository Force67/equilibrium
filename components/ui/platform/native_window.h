// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Platform window.
#pragma once

#if defined(CreateWindow)
#undef CreateWindow
#endif

#include <core/SkRect.h>

#include <base/strings/string_ref.h>
#include <base/memory/unique_pointer.h>

namespace ui {

class NativeWindow {
 public:
  class Delegate {};  // delegate prototype

  // On windows this aliases directly to the native handle type, however
  // on other platforms we store a metadata struct block in the handle.
  using handle = void*;

  inline NativeWindow(const base::StringRefU8 title) : title_(title.data()) {}

  // create the window, if a parent handle is passed the style and other
  // properties get inherited.
  virtual bool Init(handle native_parent_handle, const SkIRect bounds) = 0;

  virtual handle os_handle() const = 0;

  virtual bool SetTitle(const base::StringRefU8 title) = 0;
  const base::StringRefU8 title() const { return title_; }

  // may return an empty rect if the getter function fails.
  virtual const SkIRect bounds() const = 0;
  SkScalar dpi() const { return dpi_; }

  // visibility control
  inline void Show() { SendCommand(Command::kShow); }
  inline void Hide() { SendCommand(Command::kHide); }
  inline void ToggleVisibility(bool b) {
    SendCommand(b ? Command::kShow : Command::kHide);
  }

 protected:
  // implementation specific commands
  enum class Command {
    kHide,
    kNormal,
    kMinimized,
    kMaximized,
    kMaximize,
    kShowNoActivate,
    kShow,
    kMinimize,
    kShowMinnoActive,
    kRestore,
    kShowDefault,
    kForceMinimize,
    kMax
  };
  virtual void SendCommand(Command) = 0;

 protected:
  base::StringU8 title_;
  SkScalar dpi_{};
};

base::UniquePointer<NativeWindow> MakeWindow(const base::StringRefU8,
                                             NativeWindow::Delegate&);
}  // namespace ui