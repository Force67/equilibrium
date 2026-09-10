// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/optional.h>
#include <base/filesystem/path.h>

namespace base {

enum class KnownPath {
  Start = 0,
  // The following refer to the current application.
  CurrentExecutable,  // Path and filename of the current executable.

  PathWinStart = 100,

  DirWindows,  // Windows directory, usually "c:\windows"
  DirSystem,   // Usually c:\windows\system32"
  //                         32-bit     32-bit on 64-bit   64-bit on 64-bit
  // DirProgramFiles             1               2                  1
  // DirProgramFilesX86          1               2                  2
  // DirProgramFiles6432         1               1                  1
  // 1 - C:\Program Files   2 - C:\Program Files (x86)
  DirProgramFiles,      // See table above.
  DirProgramFilesX86,   // See table above.
  DirProgramFiles6432,  // See table above.

  DirIeInternetCache,       // Temporary Internet Files directory.
  DirCommonStartMenu,       // C:\ProgramData\Microsoft\Windows\
                            // Start Menu\Programs
  DirStartMenu,             // C:\Users\<user>\AppData\Roaming\
                            // Microsoft\Windows\Start Menu\Programs
  DirCommonStartup,         // C:\ProgramData\Microsoft\Windows\
                            // Start Menu\Programs\Startup
  DirUserStartup,           // C:\Users\<user>\AppData\Roaming\
                            // Microsoft\Windows\Start Menu\Programs\Startup
  DirRoamingAppData,        // C:\Users\<user>\AppData\Roaming.
  DirLocalAppData,          // C:\Users\<user>\AppData\Local.
  DirCommonAppData,         // C:\ProgramData.
  DirAppShortcuts,          // Windows 8 start-screen tiles; maps to
                            // "Local\AppData\Microsoft\Windows\
                            // Application Shortcuts\".
  DirCommonDesktop,         // Common desktop, visible on all users' Desktop.
  DirUserQuickLaunch,       // Quick launch shortcuts.
  DirTaskbarPins,           // Shortcuts pinned to the taskbar.
  DirImplicitAppShortcuts,  // Implicit user pinned shortcuts.
  DirWindowsFonts,          // C:\Windows\Fonts.

  PathWinEnd
};

base::Optional<base::Path> FetchKnownPath(const KnownPath);
}  // namespace base