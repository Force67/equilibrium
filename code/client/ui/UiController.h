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
  namespace sync {
	class SyncController;
  }

  class UiController final : public QObject {
	Q_OBJECT;

  public:
	UiController(sync::SyncController &);
	~UiController();

	void OpenRunDialog();
  private:
	void BuildUi();

	static ssize_t idaapi OnUiEvent(void *, int, va_list);

	QAction *_connectAct = nullptr;

	sync::SyncController &_sync;

	bool _init = false;
  public slots:
	void ToggleConnect();

  private slots:
	void OpenSyncMenu();
	void OpenAboutDialog();
	void OpenConfiguration();
  };
} // namespace noda