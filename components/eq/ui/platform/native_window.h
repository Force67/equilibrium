// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Platform window.
#pragma once

// workaround for windows bs
#if defined(CreateWindow)
#undef CreateWindow
#endif

#include <base/enum_traits.h>
#include <base/strings/string_ref.h>
#include <base/memory/unique_pointer.h>

#include <ui/primitives/rect.h>

namespace ui {

class NativeWindow {
 public:
  class Delegate {
   public:
    virtual ~Delegate() = default;
  };  // delegate prototype

  // On windows this aliases directly to the native handle type, however
  // on other platforms we store a metadata struct block in the handle.
  using handle = void*;

  inline NativeWindow(const base::StringRefU8 title) : title_(title.data()) {}
  virtual ~NativeWindow() = default;

  // create the window, if a parent handle is passed the style and other
  // properties get inherited.
  enum class CreateFlags : u32 {
    kNone = 0,
    kCustomBorder = 1 << 0,
  };

  virtual bool Init(handle native_parent_handle,
                    const ui::IRect bounds,
                    const CreateFlags flags, uint8_t icon_id = 102) = 0;

  virtual void SetDelegate(Delegate*) = 0;

  virtual handle os_handle() const = 0;

  virtual bool SetTitle(const base::StringRefU8 title) = 0;
  const base::StringRefU8 title() const { return title_; }

  // may return an empty rect if the getter function fails.
  virtual const ui::IRect bounds() const = 0;
  float dpi() const { return dpi_; }

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

BASE_IMPL_ENUM_BIT_TRAITS(NativeWindow::CreateFlags, u32)

base::UniquePointer<NativeWindow> MakeWindow(
    const base::StringRefU8,
    NativeWindow::Delegate* delegate = nullptr);
}  // namespace ui