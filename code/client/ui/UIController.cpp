// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "UiController.h"
#include "sync/SyncClient.h"

#include "forms/StatusBar.h"
#include "forms/Settings.h"
#include "forms/ConnectDialog.h"
#include "forms/AboutDialog.h"
#include "forms/WelcomeDialog.h"

#include <qmainwindow.h>
#include <qstatusbar.h>
#include <qmenubar.h>
#include <qerrormessage.h>

#include "ColorConstants.h"

namespace noda
{
  static QMainWindow *GetTopWindow()
  {
	return qobject_cast<QMainWindow *>(
	    QApplication::activeWindow()->topLevelWidget());
  }

  UiController::UiController(SyncClient &client) :
      _client(client)
  {
	hook_to_notification_point(hook_type_t::HT_UI, OnUiEvent, this);
  }

  UiController::~UiController()
  {
	unhook_from_notification_point(hook_type_t::HT_UI, OnUiEvent, this);
  }

  // http://www.flatuicolorpicker.com/purple-rgb-color-model/  rgb(142, 68, 173)
  // https://icons8.com/articles/how-to-recolor-a-raster-icon-in-photoshop/

  void UiController::BuildUi()
  {
	auto *pIdaWindow = GetTopWindow();

	// pin bottom status bar (online/offline indicator)
	_statusBar.reset(new ui::StatusBar());
	_statusBar->SetColor(colorconstant::red);
	_statusBar->show();
	pIdaWindow->statusBar()->addPermanentWidget(_statusBar.data());

	// create the top level menu entry
	if(auto *pMenu = pIdaWindow->menuBar()->addMenu(QIcon(":/logo"), "NODA")) {
	  _connectAct = pMenu->addAction("Connect", this, &UiController::ToggleConnect);
	  pMenu->addAction(QIcon(":/sync"), "Synchronus", this, &UiController::OpenSyncMenu);
	  pMenu->addSeparator();
	  pMenu->addAction(QIcon(":/cog"), "Configure", this, &UiController::OpenConfiguration);
	  pMenu->addSeparator();
	  pMenu->addAction(QIcon(":/info"), "About NODA", this, &UiController::OpenAboutDialog);
	}
  }

  void UiController::OpenAboutDialog()
  {
	ui::AboutDialog dialog(GetTopWindow());
	dialog.exec();
  }

  void UiController::ToggleConnect()
  {
	if(!_client.IsConnected()) {
	  _statusBar->SetColor(colorconstant::orange);

	  bool result = _client.Connect();
	  if(!result) {
		_statusBar->SetColor(colorconstant::red);

		QErrorMessage error(QApplication::activeWindow());
		error.showMessage(
		    "Unable to connect to the NODA sync host.\n"
		    "It is likely that the selected port is not available.");
		error.exec();
		return;
	  }

	  _connectAct->setText("Disconnect");
	  _statusBar->SetColor(colorconstant::green);
	} else {
	  _client.Disconnect();
	  _connectAct->setText("Connect");
	  _statusBar->SetColor(colorconstant::red);
	}
  }

  void UiController::OpenSyncMenu()
  {
	msg("OpenSyncMenu");
  }

  void UiController::OpenConfiguration()
  {
	ui::Settings settings(_client, GetTopWindow());
	settings.exec();
  }

  ssize_t UiController::OnUiEvent(void *userp, int notificationCode,
                                  va_list va)
  {
	auto *self = reinterpret_cast<UiController *>(userp);

	if(notificationCode == ui_notification_t::ui_ready_to_run) {
	  if(!self->_statusBar) {
		self->BuildUi();

		// crashes the UI: todo investigate why
		if (ui::WelcomeDialog::ShouldShow()) {
		  ui::WelcomeDialog dialog;
		  dialog.exec();
		}

		if (ui::ConnectDialog::ShouldShow()) {
		  ui::ConnectDialog promt(*self);
		  promt.exec();
		}
	  }
	}

	return 0;
  }
}