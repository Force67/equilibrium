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

	connect(editIp, &QLineEdit::textChanged, this, &Settings::OnIpChange);
	connect(editPort, &QLineEdit::textChanged, this, &Settings::OnPortChange);
	connect(editPassword, &QLineEdit::textChanged, this, &Settings::OnPasswordChange);
	connect(editUsername, &QLineEdit::textChanged, this, &Settings::OnNameChange);

	// trying to follow the hexrays naming convention lol
	editIp->setText(_settings.value("Nd_SyncIp", kServerIp).toString());
	editPort->setText(_settings.value("Nd_SyncPort", kServerPort).toString());
	editPassword->setText(_settings.value("Nd_SyncPass", "").toString());
	editUsername->setText(_settings.value("Nd_ASyncUser",
	                                      utility::GetSysUsername())
	                          .toString());

	if(client.IsConnected()) {
	  editIp->setDisabled(true);
	  editPort->setDisabled(true);
	  editPassword->setDisabled(true);
	  editUsername->setDisabled(true);
	}
  }

  void Settings::OnIpChange(const QString &data)
  {
	_settings.setValue("Nd_SyncIp", data);
  }

  void Settings::OnPortChange(const QString &data)
  {
	_settings.setValue("Nd_SyncPort", data.toUInt());
  }

  void Settings::OnPasswordChange(const QString &data)
  {
	_settings.setValue("Nd_SyncPass", data);
  }

  void Settings::OnNameChange(const QString &data)
  {
	_settings.setValue("Nd_SyncUser", data);
  }
} // namespace noda