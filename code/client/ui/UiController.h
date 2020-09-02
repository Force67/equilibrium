// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#pragma once

#include <IdaInc.h>
#include <memory>

#include <qscopedpointer.h>

namespace QT {
class UiStatusBar;
}

class UiController {
public:
  UiController();
  ~UiController();

private:
  static ssize_t idaapi UiHandler(void *, int, va_list);

  QT::QScopedPointer<QT::UiStatusBar> _statusBar;
};