// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <qcolor.h>

// ui icon color:
// http://www.flatuicolorpicker.com/purple-rgb-color-model/
// Common rgb: 142:68:173
// Blue highlight rgb: 34:69:246
// Cloud: 47a5db

// Green checkmark color:

// backport of QColorConstants for QT 5.6
// colors taken from official color spec at
// https://doc.qt.io/qt-5/qcolorconstants.html
namespace noda::colorconstant {
  constexpr QRgb orange{ 0xffa500 };
  constexpr QRgb green{ 0x00ff00 };
  constexpr QRgb darkGreen{ 0x008000 };
  constexpr QRgb red{ 0xff0000 };
} // namespace noda::colorconstant
