// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#pragma once

#include <IdaInc.h>
#include <memory>

#include <qobject.h>
#include <qscopedpointer.h>

namespace QT {
class QAction;
}

class UiStatusBar;

class UiController final : public QObject {
  Q_OBJECT;

public:
  UiController();
  ~UiController();

private:
  void BuildUi();

  static ssize_t idaapi OnUiEvent(void *, int, va_list);

  QT::QScopedPointer<UiStatusBar> _statusBar;

  QAction *_pConnectionAct = nullptr;
  QAction *_pMenuAct = nullptr;

private slots:
  void OnConnectionAct();
  void OnConfigureAct();
};