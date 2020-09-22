// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <IdaInc.h>
#include <qobject.h>
#include <qscopedpointer.h>

namespace QT
{
  class QAction;
}

namespace noda
{
  namespace ui
  {
	class StatusBar;
  }

  namespace sync
  {
	class SyncController;
  }

  class UiController final : public QObject
  {
	Q_OBJECT;

  public:
	UiController(sync::SyncController &);
	~UiController();

  private:
	void BuildUi();

	static ssize_t idaapi OnUiEvent(void *, int, va_list);

	QT::QScopedPointer<ui::StatusBar> _statusBar;

	QAction *_connectAct = nullptr;
	QAction *_pMenuAct = nullptr;

	sync::SyncController &_sync;

  public slots:
	void ToggleConnect();

	void OnConnected();
	void OnDisconnect(uint32_t);

  private slots:
	void OpenSyncMenu();
	void OpenAboutDialog();
	void OpenConfiguration();
  };
} // namespace noda