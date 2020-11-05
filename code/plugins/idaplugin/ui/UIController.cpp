// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "UiController.h"
#include "test.h"

#include "forms/Settings.h"
#include "forms/ConnectDialog.h"
#include "forms/AboutDialog.h"
#include "forms/WelcomeDialog.h"
#include "forms/NSyncDialog.h"
#include "forms/RunDialog.h"
#include "forms/StatusWidget.h"

#include <qmainwindow.h>
#include <QToolButton>
#include <qstatusbar.h>
#include <QTime>
#include <QTimer>
#include <qmenubar.h>
#include <qerrormessage.h>

#include "ColorConstants.h"

namespace noda {
  static const char kUiNodeName[] = "$ nd_ui_data";

  static QMainWindow *GetTopWindow()
  {
	return qobject_cast<QMainWindow *>(
	    QApplication::activeWindow()->topLevelWidget());
  }

  QMainWindow *UiController::GetMainWindow()
  {
	static QMainWindow *s_mainWindow{ nullptr };
	if(!s_mainWindow) {
	  for(auto *it : QApplication::topLevelWidgets()) {
		if(auto *window = qobject_cast<QMainWindow *>(it)) {
		  s_mainWindow = window;
		  break;
		}
	  }
	}

	return s_mainWindow;
  }

  UiController::UiController(SyncController &s) :
      _sync(s)
  {
	hook_to_notification_point(hook_type_t::HT_UI, OnUiEvent, this);

	auto *mainWindow = GetMainWindow();
	assert(mainWindow != nullptr);

	auto *menuBar = mainWindow->menuBar();

	// look away!!!! this is a bit hacked together, but nevertheless it works
	QMenu *fileMenu = reinterpret_cast<QMenu *>(menuBar->actions()[0]->parent());
	for(auto *it : fileMenu->actions()) {
	  // glue on the new open act
	  if(it->text() == "&Load file") {
		_cloudDlAct = new QAction(QIcon(":/cloud_download"), "Import NodaDB", fileMenu);
		connect(_cloudDlAct, &QAction::triggered, this, &UiController::ImportNodaDB);

		fileMenu->insertAction(it, _cloudDlAct);
	  }
	  if(it->text() == "Sa&ve as...") {
		_cloudUpAct = new QAction(QIcon(":/cloud_upload"), "Export NodaDB", fileMenu);
		connect(_cloudUpAct, &QAction::triggered, this, &UiController::ExportNodaDB);

		_cloudUpAct->setEnabled(false);
		fileMenu->insertAction(it, _cloudUpAct);
	  }
	}

	if(QMenu *nodaMenu = menuBar->addMenu(QIcon(":/logo"), "NODA")) {
	  _connectAct = nodaMenu->addAction("Connect", this, &UiController::ToggleConnect);
	  nodaMenu->addAction(QIcon(":/sync"), "NSync", this, &UiController::OpenSyncMenu);
	  nodaMenu->addSeparator();
	  nodaMenu->addAction(QIcon(":/cog"), "Configure", this, &UiController::OpenConfiguration);
	  nodaMenu->addSeparator();
	  nodaMenu->addAction(QIcon(":/info"), "About NODA", this, &UiController::OpenAboutDialog);
	}

	auto *statusBar = mainWindow->statusBar();

	_labelBuild.reset(new QLabel("NODA - " GIT_BRANCH "@" GIT_COMMIT));
	_netStatus.reset(new StatusWidget(statusBar));

	_labelCounter.reset(new QLabel());
	_timer.reset(new QTimer(this));

	statusBar->addPermanentWidget(_labelCounter.data());
	statusBar->addPermanentWidget(_labelBuild.data());
	statusBar->addPermanentWidget(_netStatus.data());

	_labelCounter->hide();

	connect(_timer.data(), &QTimer::timeout, this, &UiController::UpdateCounter);
	connect(&_sync, &SyncController::Connected, _netStatus.data(), &StatusWidget::OnConnected);
	connect(&_sync, &SyncController::Disconnected, _netStatus.data(), &StatusWidget::OnDisconnect);
	connect(&_sync, &SyncController::Broadcasted, _netStatus.data(), &StatusWidget::OnBroadcast);

	// this needs to be done here, for some reason :D
	connect(&_sync, &SyncController::Connected, [&]() {
	  _connectAct->setText("Disconnect");
	});
	connect(&_sync, &SyncController::Disconnected, [&]() {
	  _connectAct->setText("Connect");
	});
  }

  UiController::~UiController()
  {
	unhook_from_notification_point(hook_type_t::HT_UI, OnUiEvent, this);
  }

  void UiController::ImportNodaDB()
  {
	LOG_INFO("DlFromServer");

	// progress dialog.. and thread...

	// select IDB
  }

  void UiController::ExportNodaDB()
  {
	LOG_INFO("SaveToServer");

	// this file...
  }

  void UiController::OnIdbLoad()
  {
	_cloudUpAct->setEnabled(true);

	// kill netnode if open before...
	_node.clear();

	// load the netnode
	_node = NetNode(kUiNodeName);

	if(!_node.good()) {
	  QErrorMessage error(QApplication::activeWindow());
	  error.showMessage("Unable to connect to ui storage node");
	  error.exec();
	  return;
	}

	// every sec we update the clock
	_timer->start(1000);
	_labelCounter->show();
	_timeCount = _node.LoadScalar(NodeIndex::Timer, 0u);
  }

  void UiController::OnIdbSave()
  {
	assert(_node.good());

	bool res;
	res = _node.StoreScalar(NodeIndex::Timer, _timeCount);

	if(!res) {
	  QErrorMessage error(QApplication::activeWindow());
	  error.showMessage("Unable to flush ui storage node");
	  error.exec();
	}
  }

  void UiController::UpdateCounter()
  {
	int hours = (_timeCount / (60 * 60)) % 24;
	int minutes = (_timeCount / 60) % 60;
	int seconds = _timeCount % 60;

	// format time with leading zeros
	QString text = tr("Time wasted: %1:%2:%3")
	                   .arg(hours, 2, 10, QLatin1Char('0'))
	                   .arg(minutes, 2, 10, QLatin1Char('0'))
	                   .arg(seconds, 2, 10, QLatin1Char('0'));

	_labelCounter->setText(text);
	_timeCount++;
  }

  void UiController::DestroyUi()
  {
	// takes the ownership of these widgets *away* from ida on purpose
	// so it cant release em
	_netStatus.reset();
	_labelBuild.reset();

	_timer->stop();
  }

  void UiController::OpenRunDialog()
  {
	RunDialog dialog(GetTopWindow());
	dialog.exec();
  }

  void UiController::OpenAboutDialog()
  {
	AboutDialog dialog(GetTopWindow());
	dialog.exec();
  }

  void UiController::OpenSyncMenu()
  {
	NSyncDialog dialog(GetTopWindow());
	dialog.exec();
  }

  void UiController::ToggleConnect()
  {
	if(_sync.IsConnected())
	  _sync.DisconnectServer();
	else {
	  bool result = _sync.ConnectServer();
	  if(!result) {
		QErrorMessage error(QApplication::activeWindow());
		error.showMessage(
		    "Unable to connect to the NODA sync host.\n"
		    "It is likely that the selected port is not available.");
		error.exec();
	  }
	}
  }

  void UiController::OpenConfiguration()
  {
	Settings settings(_sync.IsConnected(), GetTopWindow());
	settings.exec();
  }

  ssize_t UiController::OnUiEvent(void *userp, int status, va_list va)
  {
	auto *self = reinterpret_cast<UiController *>(userp);
	switch(status) {
	case ui_notification_t::ui_ready_to_run:
	  self->OnIdbLoad();
	  break;
	case ui_notification_t::ui_term:
	  self->DestroyUi();
	  break;
	case ui_notification_t::ui_saving:
	  self->OnIdbSave();
	  break;
	case ui_notification_t::ui_updating_actions: {
	  auto *ctx = va_arg(va, action_update_ctx_t *);
	  /*  char buf[512]{};
	  QT::qsnprintf(buf, 512, "%s:%s:%d", ctx->action, ctx->widget_title.c_str(), ctx->widget_type);
	  MessageBoxA(nullptr, buf, 0, 0);*/
	  break;
	}
	}

	return 0;
  }
} // namespace noda