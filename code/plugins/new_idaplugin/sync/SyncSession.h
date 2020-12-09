#pragma once
// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "SyncService.h"
#include "SessionStore.h"

#include <QObject>
#include <network/TCPClient.h>

class Plugin;

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

  void LoginUser();

  static const char *TransportStateToString(TransportState);
  const char *CurrentTransportStateToString() const;

  void SetTransportState(TransportState state);

signals:
  void TransportStateChange(TransportState newState);

private:
  // NetworkedComponent
  void ConsumeMessage(const uint8_t *ptr, size_t len) override;
  void OnDisconnected(int reason) override{};

  void HandleAuthAck(const protocol::MessageRoot *);

  TransportState _state;
  SyncService _service;
  SessionStore _storage;

  Plugin &_plugin;
};