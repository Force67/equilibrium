#pragma once
// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "SyncService.h"
#include "SessionStore.h"

#include <QObject>
#include <QScopedPointer>

#include <network/TCPClient.h>

class Plugin;

namespace QT {
  class QTimer;
}

class SyncSession final : public QObject,
                          public network::NetworkedClientComponent {
  Q_OBJECT;

public:
  explicit SyncSession(Plugin &);
  ~SyncSession();

  enum class TransportState {
	DISABLED,
	INITIALIZED,
	PENDING,
	CONFIGURING,
	ACTIVE,
  };

  enum class NotificationCode {
	USER_JOIN,
	USER_QUIT,
	USER_HINT,
  };

  void LoginUser();

  static const char *TransportStateToString(TransportState);
  const char *CurrentTransportStateToString() const;

  int UserCount() const;
signals:

  void TransportStateChange(TransportState);
  void SessionNotification(NotificationCode);

  /*
  Qt VS Tools: QML debug: DISABLED: Non-Qt/MSBuild project
QObject::connect: Cannot queue arguments of type 'NotificationCode'
(Make sure 'NotificationCode' is registered using qRegisterMetaType().)
Der Thread 0x4448 hat mit Code 0 (0x0) geendet.
  */

private:
  void SetTransportState(TransportState state);

  // NetworkedComponent
  void ConsumeMessage(const uint8_t *ptr, size_t len) override;
  void OnDisconnected(int reason) override{};

  void HandleAuthAck(const protocol::MessageRoot *);
  void HandleUserEvent(const protocol::MessageRoot *);

  TransportState _state;
  SyncService _service;
  SessionStore _storage;

  Plugin &_plugin;
  int _userCount = 1;

  QScopedPointer<QTimer> _timeout;
};