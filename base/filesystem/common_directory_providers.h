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
  DirCommonStartMenu,       // Usually "C:\ProgramData\Microsoft\Windows\
                                // Start Menu\Programs"
  DirStartMenu,             // Usually "C:\Users\<user>\AppData\Roaming\
                                // Microsoft\Windows\Start Menu\Programs"
  DirCommonStartup,         // Usually "C:\ProgramData\Microsoft\Windows\
                                // Start Menu\Programs\Startup"
  DirUserStartup,           // Usually "C:\Users\<user>\AppData\Roaming\
                                // Microsoft\Windows\Start Menu\Programs\Startup"
  DirRoamingAppData,        // Roaming Application Data directory under the
                            // user profile.
                            // Usually "C:\Users\<user>\AppData\Roaming".
                            // Data for specific applications is stored in
                            // subdirectories.
  DirLocalAppData,          // Local Application Data directory under the
                            // user profile.
                            // Usually "C:\Users\<user>\AppData\Local".
                            // Data for specific applications is stored in
                            // subdirectories.
  DirCommonAppData,         // Usually "C:\ProgramData". Data for specific
                            // applications is stored in subdirectories.
  DirAppShortcuts,          // Where tiles on the start screen are stored,
                            // only for Windows 8. Maps to "Local\AppData\
                                // Microsoft\Windows\Application Shortcuts\".
  DirCommonDesktop,         // Directory for the common desktop (visible
                            // on all user's Desktop).
  DirUserQuickLaunch,       // Directory for the quick launch shortcuts.
  DirTaskbarPins,           // Directory for the shortcuts pinned to taskbar.
  DirImplicitAppShortcuts,  // The implicit user pinned shortcut directory.
  DirWindowsFonts,          // Usually C:\Windows\Fonts.

  PathWinEnd
};

base::Optional<base::Path> FetchKnownPath(const KnownPath);
}  // namespace base