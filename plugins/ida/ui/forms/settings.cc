// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "settings.h"

#include <sync/utils/user_info.h>
#include <sync/engine/sync_base.h>

namespace forms {
Settings::Settings(bool connected, QWidget* pParent) : QDialog(pParent) {
  setupUi(this);

  connect(editIP, &QLineEdit::textChanged,
          [&](const QString& data) { _settings.setValue("Nd_SyncIp", data); });
  connect(editPort, &QLineEdit::textChanged, [&](const QString& data) {
    _settings.setValue("Nd_SyncPort", data.toUInt());
  });
  connect(editPass, &QLineEdit::textChanged, [&](const QString& data) {
    _settings.setValue("Nd_SyncPass", data);
  });
  connect(editUser, &QLineEdit::textChanged, [&](const QString& data) {
    _settings.setValue("Nd_SyncUser", data);
  });
  connect(editTimeout, &QLineEdit::textChanged, [&](const QString& data) {
    _settings.setValue("Nd_NetTimeout", data.toUInt());
  });
  connect(cbShowWelcome, &QCheckBox::clicked,
          [&](bool down) { _settings.setValue("Nd_UiSkipWelcome", !down); });
  connect(cbShowAutoconnect, &QCheckBox::clicked,
          [&](bool down) { _settings.setValue("Nd_UiSkipConnect", !down); });

  editIP->setText(
      _settings.value("Nd_SyncIp", sync::kDefaultSyncIp).toString());
  editPort->setText(
      _settings.value("Nd_SyncPort", sync::kDefaultSyncPort).toString());
  editPass->setText(_settings.value("Nd_SyncPass", "").toString());

  std::string username;
  {
    QSettings settings;
    username = settings.value("RETK_SyncUser", "").toString().toUtf8();
  }

  if (username.empty())
    username = sync::utils::GetSysUserName();

  editUser->setText(username.c_str());

  // network::kTimeout
  editTimeout->setText(
      _settings.value("Nd_NetTimeout", 1337).toString());
  cbShowWelcome->setChecked(!_settings.value("Nd_UiSkipWelcome").toBool());
  cbShowAutoconnect->setChecked(!_settings.value("Nd_UiSkipConnect").toBool());

  // gray out these fields when the network is active
  gbSyncConfig->setDisabled(connected);
  gbNetConfig->setDisabled(connected);
}
}  // namespace forms