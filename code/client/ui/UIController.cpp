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
#include <qstatusbar.h>
#include <qmenubar.h>
#include <qerrormessage.h>

#include "ColorConstants.h"

namespace noda {
  static QMainWindow *GetTopWindow()
  {
	return qobject_cast<QMainWindow *>( 
	    QApplication::activeWindow()->topLevelWidget());
  }

  bool UiController::_s_init = false;

  UiController::UiController(SyncController &s) :
      _sync(s)
  {
	hook_to_notification_point(hook_type_t::HT_UI, OnUiEvent, this);
  }

  UiController::~UiController()
  {
	unhook_from_notification_point(hook_type_t::HT_UI, OnUiEvent, this);
  }

  void UiController::BuildUi()
  {
	auto *mainWindow = GetTopWindow();

	// this needs to be done here, for some reason :D
	connect(&_sync, &SyncController::Connected, [&]() {
	  _connectAct->setText("Disconnect");
	});
	connect(&_sync, &SyncController::Disconnected, [&]() {
	  _connectAct->setText("Connect");
	});

	// pin additional NODA status bar information
	QStatusBar *statusBar = mainWindow->statusBar();

	_labelBuild.reset(new QLabel("NODA - " GIT_BRANCH "@" GIT_COMMIT));
	statusBar->addPermanentWidget(_labelBuild.data());

	_netStatus.reset(new StatusWidget(statusBar));
	connect(&_sync, &SyncController::Connected, _netStatus.data(), &StatusWidget::OnConnected);
	connect(&_sync, &SyncController::Disconnected, _netStatus.data(), &StatusWidget::OnDisconnect);
	connect(&_sync, &SyncController::Broadcasted, _netStatus.data(), &StatusWidget::OnBroadcast);
	connect(&_sync, &SyncController::StatsUpdated, _netStatus.data(), &StatusWidget::OnStatsUpdate);

	statusBar->addPermanentWidget(_netStatus.data());

	// create the top level menu entry*/
	auto *mainBar = mainWindow->menuBar();

	if(auto *topMenu = mainBar->addMenu(QIcon(":/logo"), "NODA")) {
	  _connectAct = topMenu->addAction("Connect", this, &UiController::ToggleConnect);
	  topMenu->addAction(QIcon(":/sync"), "NSync", this, &UiController::OpenSyncMenu);
	  topMenu->addSeparator();
	  topMenu->addAction(QIcon(":/cog"), "Configure", this, &UiController::OpenConfiguration);
	  topMenu->addSeparator();
	  topMenu->addAction(QIcon(":/info"), "About NODA", this, &UiController::OpenAboutDialog);
	}
  }

  void UiController::DestroyUi()
  {
	  // takes the ownership of these widgets *away* from ida on purpose
	  // so it cant release em
	  _netStatus.reset();
	  _labelBuild.reset();
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
	auto *self = reinterpret_cast<UiController*>(userp);
	switch (status) {
	case ui_notification_t::ui_ready_to_run: {
	  if(!_s_init) {
		_s_init = true;
		self->BuildUi();

		if(WelcomeDialog::ShouldShow()) {
		  WelcomeDialog dialog;
		  dialog.exec();
		}
	  }

	  if(ConnectDialog::ShouldShow()) {
		ConnectDialog promt(*self);
		promt.exec();
	  }
		break;
	}
	case ui_notification_t::ui_term: {
	  self->DestroyUi();
		break;
	}
	}

	return 0;
  }
} // namespace noda