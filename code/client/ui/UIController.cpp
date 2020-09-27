// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "UiController.h"

#include "forms/Settings.h"
#include "forms/ConnectDialog.h"
#include "forms/AboutDialog.h"
#include "forms/WelcomeDialog.h"
#include "forms/NetStatusWidget.h"

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

	UiController::UiController(sync::SyncController &s) :
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
		connect(&_sync, &sync::SyncController::Connected, [&]() {
			_connectAct->setText("Disconnect");
		});
		connect(&_sync, &sync::SyncController::Disconnected, [&]() {
			_connectAct->setText("Connect");
		});

		// pin additonal NODA statusbar information
		QStatusBar *statusBar = mainWindow->statusBar();
		statusBar->addPermanentWidget(new QLabel("NODA - " GIT_BRANCH "@" GIT_COMMIT));

		auto *netStats = new ui::NetStatusWidget(statusBar);

		connect(&_sync, &sync::SyncController::Connected, netStats, &ui::NetStatusWidget::OnConnected);
		connect(&_sync, &sync::SyncController::Disconnected, netStats, &ui::NetStatusWidget::OnDisconnect);
		connect(&_sync, &sync::SyncController::Broadcasted, netStats, &ui::NetStatusWidget::OnBroadcast);
		connect(&_sync, &sync::SyncController::StatsUpdated, netStats, &ui::NetStatusWidget::OnStatsUpdate);

		statusBar->addPermanentWidget(netStats);

		// create the top level menu entry
		auto *mainBar = mainWindow->menuBar();

		if(auto *topMenu = mainBar->addMenu(QIcon(":/logo"), "NODA")) {
			_connectAct = topMenu->addAction("Connect", this, &UiController::ToggleConnect);
			topMenu->addAction(QIcon(":/sync"), "INSync", this, &UiController::OpenSyncMenu);
			topMenu->addSeparator();
			topMenu->addAction(QIcon(":/cog"), "Configure", this, &UiController::OpenConfiguration);
			topMenu->addSeparator();
			topMenu->addAction(QIcon(":/info"), "About NODA", this, &UiController::OpenAboutDialog);
		}
	}

	void UiController::OpenAboutDialog()
	{
		ui::AboutDialog dialog(GetTopWindow());
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

	void UiController::OpenSyncMenu()
	{
		msg("OpenSyncMenu");
	}

	void UiController::OpenConfiguration()
	{
		ui::Settings settings(_sync.IsConnected(), GetTopWindow());
		settings.exec();
	}

	ssize_t UiController::OnUiEvent(void *userp, int notificationCode,
	                                va_list va)
	{
		auto *self = reinterpret_cast<UiController *>(userp);

		if(notificationCode == ui_notification_t::ui_ready_to_run) {
			if(!self->_init) {
				self->_init = true;
				self->BuildUi();

				if(ui::WelcomeDialog::ShouldShow()) {
					ui::WelcomeDialog dialog;
					dialog.exec();
				}

				if(ui::ConnectDialog::ShouldShow()) {
					ui::ConnectDialog promt(*self);
					promt.exec();
				}
			}
		}

		return 0;
	}
} // namespace noda