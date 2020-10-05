// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <IdaInc.h>
#include <qobject.h>
#include <qscopedpointer.h>

namespace QT {
  class QAction;
  class QLabel;
} // namespace QT

namespace noda {
  class SyncController;

  class UiController final : public QObject {
	Q_OBJECT;

  public:
	UiController(SyncController &);
	~UiController();

	void OpenRunDialog();
  private:
	void BuildUi();
	void DestroyUi();

	static ssize_t idaapi OnUiEvent(void *, int, va_list);

	QAction *_connectAct = nullptr;
	QLabel *_labelBuild = nullptr;

	SyncController &_sync;

	static bool _s_init;
  public slots:
	void ToggleConnect();

  private slots:
	void OpenSyncMenu();
	void OpenAboutDialog();
	void OpenConfiguration();
  };
} // namespace noda