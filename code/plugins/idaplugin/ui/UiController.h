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

	static QWidget *GetTopWidget();
	static QMainWindow *GetMainWindow();

	void OpenRunDialog();

	static void ShowError(const char *text);

  private:
	void DestroyUi();

	void OnIdbLoad();
	void OnIdbSave();
	void OnIdbClose();
	void OnIdbFinishAu();

	void UpdateCounter();

	static ssize_t idaapi UiEvent(void *, int, va_list);
	static ssize_t idaapi IdbEvent(void *, int, va_list);

	QAction *_connectAct = nullptr;
	QAction *_projectAct = nullptr;
	QAction *_saveToServerAct = nullptr;
	QAction *_openFromServerAct = nullptr;

	QScopedPointer<QLabel> _labelCounter;
	QScopedPointer<StatusWidget> _netStatus;

	QScopedPointer<QTimer> _timer;
	uint32_t _timeCount = 0;

	SyncController &_sync;

	// ui storage:
	enum NodeIndex : nodeidx_t {
	  Reserved,
	  Flags,
	  Timer,
	};

	enum UiFlags : uint32_t {
	  None,
	  EverJoined = 1 << 1,
	} _flags;

	NetNode _node;

  public slots:
	void ToggleConnect();

  private slots:
	void OpenSyncMenu();
	void OpenAboutDialog();
	void OpenSettings();

	void OpenFromServer();
	void SaveToServer();
  };
} // namespace noda