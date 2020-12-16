#pragma once
// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "session_store.h"
#include "sync_service.h"

#include <QObject>
#include <QScopedPointer>

#include <network/tcp_client.h>

class Plugin;

namespace QT {
class QTimer;
}

class SyncSession final : public QObject,
                          public network::NetworkedClientComponent {
  Q_OBJECT;

 public:
  explicit SyncSession(Plugin&);
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
    USERS_JOIN,
    USER_QUIT,
    USER_HINT,
  };

  void LoginUser();
  void LogOff();

  static const char* TransportStateToString(TransportState);
  const char* CurrentTransportStateToString() const;

  int UserCount() const;

  SessionStore& Store();
 signals:

  void TransportStateChange(TransportState);
  void SessionNotification(NotificationCode);

 private:
  void SetTransportState(TransportState state);

  // NetworkedComponent
  void ConsumeMessage(const uint8_t* ptr, size_t len) override;
  void OnDisconnected(int reason) override{};

  void HandleAuthAck(const protocol::MessageRoot*);
  void HandleUserEvent(const protocol::MessageRoot*);

  TransportState _state;
  SyncService _service;
  SessionStore _storage;

  Plugin& _plugin;
  int _userCount = 1;

  QScopedPointer<QTimer> _timeout;

  Q_ENUM(SyncSession::TransportState)
  Q_ENUM(SyncSession::NotificationCode)
};