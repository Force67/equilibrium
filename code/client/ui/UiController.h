// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
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
  UiController(SyncClient &);
  ~UiController();

  private:
  void BuildUi();

  static ssize_t idaapi OnUiEvent(void *, int, va_list);

  QT::QScopedPointer<UiStatusBar> _statusBar;

  QAction *_connectAct = nullptr;
  QAction *_pMenuAct = nullptr;

  SyncClient &_client;
  public slots:
  void ToggleConnect();

  private slots:
  void OpenSyncMenu();
  void OpenAboutDialog();
  void OpenConfiguration();
};