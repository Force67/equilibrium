// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "UiController.h"

#include "forms/Settings.h"
#include "forms/ConnectDialog.h"
#include "forms/AboutDialog.h"
#include "forms/ProgressDialog.h"
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

extern plugin_t PLUGIN;

namespace noda {
  static const char kUiNodeName[] = "$ nd_ui_data";

  QWidget *UiController::GetTopWidget()
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

  void UiController::ShowError(const char* text)
  {
	QErrorMessage error(GetTopWidget());
	error.showMessage(text);
	error.exec();
  }

  UiController::UiController(SyncController &s) :
      _sync(s)
  {
	hook_to_notification_point(hook_type_t::HT_UI, UiEvent, this);
	hook_to_notification_point(hook_type_t::HT_IDB, IdbEvent, this);

	auto *mainWindow = GetMainWindow();
	assert(mainWindow != nullptr);

	auto *menuBar = mainWindow->menuBar();

	QMenu* fileMenu = reinterpret_cast<QMenu*>(menuBar->actions()[0]->parent());
	QAction* boltOnPoint = fileMenu->actions()[10];

	_openFromServerAct = new QAction(QIcon(":/cloud_download"), "Open from Server", fileMenu);
	_saveToServerAct = new QAction(QIcon(":/cloud_upload"), "Save to server", fileMenu);
	connect(_openFromServerAct, &QAction::triggered, this, &UiController::OpenFromServer);
	connect(_saveToServerAct, &QAction::triggered, this, &UiController::SaveToServer);

	QAction *before = fileMenu->insertSeparator(boltOnPoint);
	fileMenu->insertAction(before, _openFromServerAct);
	fileMenu->insertAction(before, _saveToServerAct);

	if(QMenu *nodaMenu = menuBar->addMenu("Noda")) {

	  _connectAct = nodaMenu->addAction("Connect", this, &UiController::ToggleConnect);
	  //_projectAct = nodaMenu->addAction(QIcon(":/sync"), "Projects", this, &UiController::OpenSyncMenu);
	  nodaMenu->addSeparator();
	  nodaMenu->addAction(QIcon(":/cog"), "Settings", this, &UiController::OpenSettings);
	  nodaMenu->addSeparator();
	  nodaMenu->addAction(QIcon(":/info"), "About NODA", this, &UiController::OpenAboutDialog);
	}

	// those only become available when opening an idb
	_connectAct->setEnabled(false);
	//_projectAct->setEnabled(false);

	auto *statusBar = mainWindow->statusBar();
	_netStatus.reset(new StatusWidget(statusBar));

	_labelCounter.reset(new QLabel());
	_timer.reset(new QTimer(this));

	statusBar->addPermanentWidget(_labelCounter.data());
	statusBar->addPermanentWidget(_netStatus.data());

	_labelCounter->hide();

	connect(_timer.data(), &QTimer::timeout, this, &UiController::UpdateCounter);
	connect(&_sync, &SyncController::Connected, _netStatus.data(), &StatusWidget::OnConnected);
	connect(&_sync, &SyncController::Disconnected, _netStatus.data(), &StatusWidget::OnDisconnect);
	connect(&_sync, &SyncController::Announce, _netStatus.data(), &StatusWidget::OnAnnounce);

	connect(&_sync, &SyncController::Connected, [&]() {
	  _connectAct->setText("Disconnect");
	});
	connect(&_sync, &SyncController::Disconnected, [&]() {
	  _connectAct->setText("Connect");
	});
  }

  UiController::~UiController()
  {
	unhook_from_notification_point(hook_type_t::HT_IDB, IdbEvent, this);
	unhook_from_notification_point(hook_type_t::HT_UI, UiEvent, this);
  }

  void UiController::OpenFromServer()
  {
	  // list remote projects..
	  LOG_INFO("TODO: OpenFromServer()");
  }

  void UiController::SaveToServer()
  {
	  LOG_INFO("TODO: SaveToServer()");
  }

  void UiController::OnIdbSave()
  {
	assert(_node.good());

	bool res;
	res = _node.StoreScalar(NodeIndex::Timer, _timeCount);
	res = _node.StoreScalar(NodeIndex::Flags, _flags);

	if(!res) {
	  QErrorMessage error(QApplication::activeWindow());
	  error.showMessage("Unable to flush ui storage node");
	  error.exec();
	}
  }

  void UiController::OnIdbClose()
  {
	// note that closing the idb doesnt mean the closure of IDA
	_connectAct->setEnabled(false);
	//_projectAct->setEnabled(false);
	_timer->stop();
  }

  void UiController::OnIdbFinishAu()
  {
	_connectAct->setEnabled(true);
	//_cloudUpAct->setEnabled(true);
  }

  void UiController::OnIdbLoad()
  {
	if(auto_is_ok())
	  OnIdbFinishAu();
	else
	  LOG_INFO("Waiting for autoanalysis to finish...");

	_node.clear();
	_node = NetNode(kUiNodeName);

	if(!_node.good()) {
		ShowError("Unable to connect to ui storage node.");
	  return;
	}

	uint32_t version = _node.LoadScalar(NodeIndex::Version, UINT_MAX);
	if(version == UINT_MAX) {
	  _node.StoreScalar(NodeIndex::Version, kUiVersion);
	}

	// every sec we update the clock
	_timer->start(1000);
	_labelCounter->show();

	_timeCount = _node.LoadScalar(NodeIndex::Timer, 0u);
	_flags = _node.LoadScalar(NodeIndex::Flags, UiFlags::None);

	if(ConnectDialog::ShouldShow()) {
	  ConnectDialog promt(*this);
	  promt.exec();
	}
  }

  void UiController::UpdateCounter()
  {
	int hours = (_timeCount / 3600) % 24;
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
	// release the ownership of statusbar resources here early so ida cant release them
	// after our plugin is already unloaded

	_netStatus.reset();
  }

  void UiController::OpenRunDialog()
  {
	RunDialog dialog(GetTopWidget());
	dialog.exec();
  }

  void UiController::OpenAboutDialog()
  {
	AboutDialog dialog(GetTopWidget());
	dialog.exec();
  }

  void UiController::OpenSyncMenu()
  {
	NSyncDialog dialog(GetTopWidget());
	dialog.exec();
  }

  void UiController::ToggleConnect()
  {
	bool connected = _sync.IsConnected();
	if (connected) {
	  _sync.Disconnect();

	  _projectAct->setEnabled(false);
	  return;
	}

	bool result = _sync.Connect();
	if (!result) {
	  ShowError(
	      "Unable to connect to the NODA sync host.\n"
	      "It is likely that the selected port is not available.");
	  return;
	}

	_projectAct->setEnabled(result);
	LOG_INFO("ToggleConnect(): {}", connected);
  }

  void UiController::OpenSettings()
  {
	Settings settings(_sync.IsConnected(), GetTopWidget());
	settings.exec();
  }

  ssize_t UiController::UiEvent(void *userp, int status, va_list va)
  {
	auto *self = reinterpret_cast<UiController *>(userp);
	switch(status) {
	case ui_notification_t::ui_term:
	  self->OnIdbClose();
	  break;
	case ui_notification_t::ui_saving:
	  self->OnIdbSave();
	  break;
	case ui_notification_t::ui_database_inited:
	  self->OnIdbLoad();
	  break;
	case ui_notification_t::ui_plugin_unloading: {
	  const plugin_info_t *pinfo = va_arg(va, plugin_info_t *);
	  if(pinfo && pinfo->entry) {
		if(pinfo->entry == &PLUGIN) {
		  self->DestroyUi();
		}
	  }

	  break;
	}
	}

	return 0;
  }

  ssize_t UiController::IdbEvent(void *userp, int status, va_list)
  {
	auto *self = reinterpret_cast<UiController *>(userp);
	if(status == idb_event::auto_empty_finally) {
	  self->OnIdbFinishAu();
	}

	return 0;
  }
} // namespace noda