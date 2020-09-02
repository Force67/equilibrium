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
class SyncClient;

class UiController final : public QObject {
  Q_OBJECT;

public:
  UiController(SyncClient&);
  ~UiController();

private:
  void BuildUi();

  static ssize_t idaapi OnUiEvent(void *, int, va_list);

  QT::QScopedPointer<UiStatusBar> _statusBar;

  QAction *_pConnectionAct = nullptr;
  QAction *_pMenuAct = nullptr;

  SyncClient &_client;
public slots:
  void DoConnect();

private slots:
  void OnConfigureAct();
};