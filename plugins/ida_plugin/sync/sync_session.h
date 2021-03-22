#pragma once
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "session_store.h"
#include "sync_service.h"

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
 signals:

  void TransportStateChange(TransportState);
  void SessionNotification(NotificationCode);

 private:
  void SetTransportState(TransportState state);

  // NetworkedComponent
  void ConsumeMessage(const uint8_t* ptr, size_t len) override;
  void OnDisconnected(int reason) override{};

  TransportState _state;

  Q_ENUM(SyncSession::TransportState)
  Q_ENUM(SyncSession::NotificationCode)
};