// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#include "UiSettings.h"
#include "utility/SysInfo.h"

UiSettings::UiSettings(SyncClient &client, QWidget *pParent) :
	QDialog(pParent), _client(client) {
  setupUi(this);

  connect(editIp, &QLineEdit::textChanged, this, &UiSettings::OnIpChange);
  connect(editPort, &QLineEdit::textChanged, this, &UiSettings::OnPortChange);
  connect(editPassword, &QLineEdit::textChanged, this, &UiSettings::OnPasswordChange);
  connect(editUsername, &QLineEdit::textChanged, this, &UiSettings::OnNameChange);

  // trying to follow the hexrays naming convention lol
  editIp->setText(_settings.value("NODASyncIp", kServerIp).toString());
  editPort->setText(_settings.value("NODASyncPort", kServerPort).toString());
  editPassword->setText(_settings.value("NODASyncPass", "").toString());
  editUsername->setText(_settings.value("NODASyncUser",
										utility::GetSysUsername())
							.toString());

  if (client.IsConnected()) {
	editIp->setDisabled(true);
	editPort->setDisabled(true);
	editPassword->setDisabled(true);
	editUsername->setDisabled(true);
  }
}

void UiSettings::OnIpChange(const QString &data) {
  _settings.setValue("NODASyncIp", data);
}

void UiSettings::OnPortChange(const QString &data) {
  _settings.setValue("NODASyncPort", data.toUInt());
}

void UiSettings::OnPasswordChange(const QString &data) {
  _settings.setValue("NODASyncPass", data);
}

void UiSettings::OnNameChange(const QString &data) {
  _settings.setValue("NODASyncUser", data);
}