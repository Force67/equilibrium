// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <IdaInc.h>
#include <qobject.h>
#include <qscopedpointer.h>

namespace QT {
  class QAction;
  class QLabel;
  class QMainWindow;
} // namespace QT

namespace noda {
  class SyncController;
  class StatusWidget;

  class UiController final : public QObject {
	Q_OBJECT;

  public:
	UiController(SyncController&);
	~UiController();

	static QMainWindow *GetMainWindow();

	void OpenRunDialog();

  private:
	void BuildUi();
	void DestroyUi();

	void ImportNodaDB();
	void ExportNodaDB();

	void OnIdbLoad();

	static ssize_t idaapi OnUiEvent(void *, int, va_list);

	QAction *_connectAct = nullptr;
	QAction *_cloudDlAct = nullptr;
	QAction *_cloudUpAct = nullptr;

	QScopedPointer<QLabel> _labelBuild;
	QScopedPointer<StatusWidget> _netStatus;

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