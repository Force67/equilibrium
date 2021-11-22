// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// 
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file.
#pragma once

namespace ui {

enum class Key {
  kNONE,  // corresponds to android's UNKNOWN

  kLeftSoftKey,
  kRightSoftKey,

  kHome,  //!< the home key - added to match android
  kBack,  //!< (CLR)
  kSend,  //!< the green (talk) key
  kEnd,   //!< the red key

  k0,
  k1,
  k2,
  k3,
  k4,
  k5,
  k6,
  k7,
  k8,
  k9,
  kStar,  //!< the * key
  kHash,  //!< the # key

  kUp,
  kDown,
  kLeft,
  kRight,

  // Keys needed by ImGui
  kTab,
  kPageUp,
  kPageDown,
  kDelete,
  kEscape,
  kShift,
  kCtrl,
  kOption,  // AKA Alt
  kSuper,   // AKA Command
  kA,
  kC,
  kV,
  kX,
  kY,
  kZ,

  kOK,  //!< the center key

  kVolUp,    //!< volume up    - match android
  kVolDown,  //!< volume down  - same
  kPower,    //!< power button - same
  kCamera,   //!< camera       - same
};
}