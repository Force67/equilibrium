// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "Pch.h"
#include <qobject.h>
#include <qscopedpointer.h>

#include "utils/NetNode.h"

namespace QT {
  class QAction;
  class QLabel;
  class QTimer;
  class QMainWindow;
} // namespace QT

namespace noda {
  class SyncController;
  class StatusWidget;

  class UiController final : public QObject {
	Q_OBJECT;

  public:
	UiController(SyncController &);
	~UiController();

	static QMainWindow *GetMainWindow();

	void OpenRunDialog();

  private:
	void DestroyUi();

	void ImportNodaDB();
	void ExportNodaDB();

	void OnIdbLoad();
	void OnIdbSave();

	void UpdateCounter();

	static ssize_t idaapi OnUiEvent(void *, int, va_list);

	QAction *_connectAct = nullptr;
	QAction *_projectAct = nullptr;
	QAction *_cloudDlAct = nullptr;
	QAction *_cloudUpAct = nullptr;

	QScopedPointer<QLabel> _labelCounter;
	QScopedPointer<StatusWidget> _netStatus;

	QScopedPointer<QTimer> _timer;
	uint32_t _timeCount = 0;

	SyncController &_sync;

	enum NodeIndex : nodeidx_t {
	  Flags,
	  Timer,
	};

	enum UiFlags : uint32_t {
	  None,
	  SkipConnect = 1 << 0,
	};

	NetNode _node;

  public slots:
	void ToggleConnect();

  private slots:
	void OpenSyncMenu();
	void OpenAboutDialog();
	void OpenConfiguration();
  };
} // namespace noda