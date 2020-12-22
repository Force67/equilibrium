// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "Settings.h"
#include "Pch.h"
#include "utils/UserInfo.h"

#include "network/Netbase.h"

namespace noda {
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
      _settings.value("Nd_SyncIp", network::constants::kServerIp).toString());
  editPort->setText(
      _settings.value("Nd_SyncPort", network::constants::kServerPort)
          .toString());
  editPass->setText(_settings.value("Nd_SyncPass", "").toString());
  editUser->setText(
      _settings.value("Nd_SyncUser", GetDefaultUserName()).toString());

  editTimeout->setText(
      _settings.value("Nd_NetTimeout", network::constants::kTimeout)
          .toString());
  cbShowWelcome->setChecked(!_settings.value("Nd_UiSkipWelcome").toBool());
  cbShowAutoconnect->setChecked(!_settings.value("Nd_UiSkipConnect").toBool());

  // gray out these fields when the network is active
  gbSyncConfig->setDisabled(connected);
  gbNetConfig->setDisabled(connected);
}
}  // namespace noda