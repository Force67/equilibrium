// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Settings.h"
#include "utility/SysInfo.h"

namespace noda::ui
{
  Settings::Settings(SyncClient &client, QWidget *pParent) :
      QDialog(pParent), _client(client)
  {
	setupUi(this);

	connect(editIP, &QLineEdit::textChanged, [&](const QString &data) {
	  _settings.setValue("Nd_SyncIp", data);
	});
	connect(editPort, &QLineEdit::textChanged, [&](const QString &data) {
	  _settings.setValue("Nd_SyncPort", data.toUInt());
	});
	connect(editPass, &QLineEdit::textChanged, [&](const QString &data) {
	  _settings.setValue("Nd_SyncPass", data);
	});
	connect(editUser, &QLineEdit::textChanged, [&](const QString &data) {
	  _settings.setValue("Nd_SyncUser", data);
	});
	connect(editTimeout, &QLineEdit::textChanged, [&](const QString &data) {
	  _settings.setValue("Nd_NetTimeout", data.toUInt());
	});
	connect(cbShowWelcome, &QCheckBox::clicked, [&](bool chcked) {
	  _settings.setValue("Nd_UiSkipWelcome", chcked);
	});
	connect(cbShowAutoconnect, &QCheckBox::clicked, [&](bool chcked) {
	  _settings.setValue("Nd_UiSkipConnect", chcked);
	});

	editIP->setText(_settings.value("Nd_SyncIp", kServerIp).toString());
	editPort->setText(_settings.value("Nd_SyncPort", kServerPort).toString());
	editPass->setText(_settings.value("Nd_SyncPass", "").toString());
	editUser->setText(_settings.value("Nd_SyncUser", utility::GetSysUsername())
	                          .toString());
	editTimeout->setText(_settings.value("Nd_NetTimeout", kTimeout).toString());
	cbShowWelcome->setEnabled(_settings.value("Nd_UiSkipWelcome").toBool());
	cbShowAutoconnect->setEnabled(_settings.value("Nd_UiSkipConnect").toBool());

	// gray out these fields when the network is active
	bool enabled = !_client.IsConnected();
	gbSyncConfig->setDisabled(enabled);
	gbNetConfig->setDisabled(enabled);
  }
} // namespace noda